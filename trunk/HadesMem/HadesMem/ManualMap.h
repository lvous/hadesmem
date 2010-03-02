#pragma once

// Windows API
#include <Windows.h>
#include <atlbase.h>
#include <atlfile.h>

// C++ Standard Library
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <iterator>
#include <iostream>

// AsmJit
#pragma warning(push, 1)
#include "AsmJit/AsmJit.h"
#pragma warning(pop)

// HadesMem
#include "I18n.h"
#include "Memory.h"
#include "Module.h"
#include "Injector.h"

namespace Hades
{
  namespace Memory
  {
    // ManualMap exception type
    class ManualMapError : public virtual HadesMemError 
    { };

    // Manual mapping class
    // Credits to Darawk for the original implementation this is based off
    class ManualMap
    {
    public:
      // Constructor
      inline ManualMap(MemoryMgr const& MyMemory);

      // Manually map MyJitFunc DLL
      inline PVOID Map(std::wstring const& Path, std::string const& Export = 
        "");

    private:
      // Convert RVA to file offset
      inline DWORD_PTR RvaToFileOffset(PIMAGE_NT_HEADERS pNtHeaders, 
        DWORD_PTR Rva);

      // Map sections
      inline void MapSections(PIMAGE_NT_HEADERS pNtHeaders, PVOID RemoteAddr, 
        std::vector<BYTE> const& ModBuffer);

      // Fix imports
      inline void FixImports(std::vector<BYTE>& ModBuffer, 
        PIMAGE_NT_HEADERS pNtHeaders, PIMAGE_IMPORT_DESCRIPTOR pImpDesc);

      // Fix relocations
      inline void FixRelocations(std::vector<BYTE>& ModBuffer, 
        PIMAGE_NT_HEADERS pNtHeaders, PIMAGE_BASE_RELOCATION pRelocDesc, 
        DWORD RelocDirSize, PVOID RemoteBase);

      // Get address of export in remote process
      inline FARPROC GetRemoteProcAddress(HMODULE RemoteMod, 
        std::wstring const& Module, LPCSTR Function, bool ByOrdinal = false);

      // Disable assignment
      ManualMap& operator= (ManualMap const&);

      // MemoryMgr instance
      MemoryMgr const& m_Memory;
    };

    // RAII class for AsmJit
    class EnsureAsmJitFree : private boost::noncopyable
    {
    public:
      // Constructor
      EnsureAsmJitFree(PVOID Address) 
        : m_Address(Address)
      { }

      // Destructor
      ~EnsureAsmJitFree()
      {
        // Free memory if necessary
        if (m_Address)
        {
          AsmJit::MemoryManager::global()->free(m_Address);
        }
      }

      // Get address
      PVOID Get() const 
      {
        return m_Address;
      }

    private:
      // Address
      PVOID m_Address;
    };

    // Constructor
    ManualMap::ManualMap(MemoryMgr const& MyMemory) 
      : m_Memory(MyMemory)
    { }

    // Manually map MyJitFunc DLL
    PVOID ManualMap::Map(std::wstring const& Path, std::string const& Export)
    {
      // Open file for reading
      std::wcout << "Opening module file for reading." << std::endl;
      std::ifstream ModuleFile(Path.c_str(), std::ios::binary);
      if (!ModuleFile)
      {
        BOOST_THROW_EXCEPTION(ManualMapError() << 
          ErrorFunction("ManualMap::Map") << 
          ErrorString("Could not open module file."));
      }

      // Read file into buffer
      std::wcout << "Reading module file into buffer." << std::endl;
      std::vector<BYTE> ModuleFileBuf((std::istreambuf_iterator<char>(
        ModuleFile)), std::istreambuf_iterator<char>());

      // Ensure file is MyJitFunc valid PE file
      std::wcout << "Performing PE file format validation." << std::endl;
      auto pBase = &ModuleFileBuf[0];
      auto pDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(pBase);
      if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
      {
        BOOST_THROW_EXCEPTION(ManualMapError() << 
          ErrorFunction("ManualMap::Map") << 
          ErrorString("Target file is not MyJitFunc valid PE file (DOS)."));
      }
      auto pNtHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(pBase + 
        pDosHeader->e_lfanew);
      if (pNtHeaders->Signature != IMAGE_NT_SIGNATURE)
      {
        BOOST_THROW_EXCEPTION(ManualMapError() << 
          ErrorFunction("ManualMap::Map") << 
          ErrorString("Target file is not MyJitFunc valid PE file (NT)."));
      }

      // Allocate memory for image
      std::wcout << "Allocating memory for module." << std::endl;
      PVOID RemoteBase = m_Memory.Alloc(pNtHeaders->OptionalHeader.
        SizeOfImage);
      std::wcout << "Module base address: " << RemoteBase << "." << std::endl;
      std::wcout << "Module size: " << pNtHeaders->OptionalHeader.SizeOfImage 
        << "." << std::endl;

      // Get all TLS callbacks
      std::vector<PIMAGE_TLS_CALLBACK> TlsCallbacks;
      auto TlsDirSize = pNtHeaders->OptionalHeader.DataDirectory
        [IMAGE_DIRECTORY_ENTRY_TLS].Size;
      if (TlsDirSize)
      {
        auto pTlsDir = reinterpret_cast<PIMAGE_TLS_DIRECTORY>(pBase + 
          RvaToFileOffset(pNtHeaders, pNtHeaders->OptionalHeader.
          DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress));

        std::wcout << "Enumerating TLS callbacks." << std::endl;
        std::wcout << "Image Base: " << reinterpret_cast<PVOID>(pNtHeaders->
          OptionalHeader.ImageBase) << "." << std::endl;
        std::wcout << "Address Of Callbacks: " << reinterpret_cast<PVOID>(
          pTlsDir->AddressOfCallBacks) << "." << std::endl;

        for (auto pCallbacks = reinterpret_cast<PIMAGE_TLS_CALLBACK*>(pBase + 
          RvaToFileOffset(pNtHeaders, pTlsDir->AddressOfCallBacks - 
          pNtHeaders->OptionalHeader.ImageBase)); *pCallbacks; ++pCallbacks)
        {
          std::wcout << "TLS Callback: " << *pCallbacks << "." << std::endl;
          TlsCallbacks.push_back(*pCallbacks);
        }
      }

      // Fix import table if applicable
      auto ImpDirSize = pNtHeaders->OptionalHeader.DataDirectory[
        IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
      if (ImpDirSize)
      {
        auto pImpDir = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(pBase + 
          RvaToFileOffset(pNtHeaders, pNtHeaders->OptionalHeader.
          DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress));

        FixImports(ModuleFileBuf, pNtHeaders, pImpDir);
      }

      // Fix relocations if applicable
      auto RelocDirSize = pNtHeaders->OptionalHeader.DataDirectory[
        IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
      if (RelocDirSize)
      {
        auto pRelocDir = reinterpret_cast<PIMAGE_BASE_RELOCATION>(pBase + 
          RvaToFileOffset(pNtHeaders, pNtHeaders->OptionalHeader.
          DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress));

        FixRelocations(ModuleFileBuf, pNtHeaders, pRelocDir, RelocDirSize, 
          RemoteBase);
      }

      // Write DOS header to process
      PBYTE DosHeaderStart = pBase;
      PBYTE DosHeaderEnd = DosHeaderStart + sizeof(IMAGE_DOS_HEADER);
      std::vector<BYTE> DosHeaderBuf(DosHeaderStart, DosHeaderEnd);
      std::wcout << "Writing DOS header." << std::endl;
      std::wcout << "DOS Header: " << DosHeaderStart << std::endl;
      m_Memory.Write(RemoteBase, DosHeaderBuf);

      // Write PE header to process
      PBYTE PeHeaderStart = reinterpret_cast<PBYTE>(pNtHeaders);
      PBYTE PeHeaderEnd = PeHeaderStart + sizeof(pNtHeaders->Signature) + 
        sizeof(pNtHeaders->FileHeader) + pNtHeaders->FileHeader.
        SizeOfOptionalHeader;
      std::vector<BYTE> PeHeaderBuf(PeHeaderStart, PeHeaderEnd);
      PBYTE TargetAddr = reinterpret_cast<PBYTE>(RemoteBase) + 
        pDosHeader->e_lfanew;
      std::wcout << "Writing NT header." << std::endl;
      std::wcout << "NT Header: " << TargetAddr << std::endl;
      m_Memory.Write(TargetAddr, PeHeaderBuf);

      // Write sections to process
      MapSections(pNtHeaders, RemoteBase, ModuleFileBuf);

      // Calculate module entry point
      PVOID EntryPoint = static_cast<PBYTE>(RemoteBase) + pNtHeaders->
        OptionalHeader.AddressOfEntryPoint;
      std::wcout << "Entry Point: " << EntryPoint << "." << std::endl;
      
      // Get address of export in remote process
      PVOID ExportAddr = GetRemoteProcAddress(reinterpret_cast<HMODULE>(
        RemoteBase), Path, Export.c_str());
      std::wcout << "Export Address: " << ExportAddr << "." << std::endl;

      // Create Assembler.
      AsmJit::Assembler MyJitFunc;
      
      #if defined(_M_AMD64) 
      // Prologue
      MyJitFunc.push(AsmJit::rbp); // 55
      MyJitFunc.mov(AsmJit::rbp, AsmJit::rsp); // 488bec

      // Entry-point calling code
      AsmJit::Immediate MyImmediate0(0);
      MyJitFunc.push(MyImmediate0); // 6a00
      MyJitFunc.push(MyImmediate0); // 6a00
      MyJitFunc.push(MyImmediate0); // 6a00
      MyJitFunc.push(MyImmediate0); // 6a00
      MyJitFunc.mov(AsmJit::rax, reinterpret_cast<DWORD_PTR>(EntryPoint)); // 48c7c000c70102
      MyJitFunc.call(AsmJit::rax); // ffd0

      // Export calling code (if necessary)
      if (ExportAddr)
      {
        MyJitFunc.mov(AsmJit::rax, reinterpret_cast<DWORD_PTR>(ExportAddr)); // 48c7c081110102
        MyJitFunc.call(AsmJit::rax); // ffd0
      }
      
      // Cleanup ghost code
      MyJitFunc.pop(AsmJit::rax); // ffd0
      MyJitFunc.pop(AsmJit::rax); // ffd0
      MyJitFunc.pop(AsmJit::rax); // ffd0
      MyJitFunc.pop(AsmJit::rax); // ffd0

      // Epilogue
      MyJitFunc.mov(AsmJit::rsp, AsmJit::rbp); // 488be5
      MyJitFunc.pop(AsmJit::rbp); // 5d

      // Return
      MyJitFunc.ret(); // c3
      #elif defined(_M_IX86) 
      // Prologue
      MyJitFunc.push(AsmJit::ebp);
      MyJitFunc.mov(AsmJit::ebp, AsmJit::esp);

      // TLS calling code
      std::for_each(TlsCallbacks.begin(), TlsCallbacks.end(), 
        [&MyJitFunc, RemoteBase] (PIMAGE_TLS_CALLBACK pCallback) 
      {
        // Entry-point calling code
        AsmJit::Immediate MyImmediate0(0);
        MyJitFunc.push(MyImmediate0);
        AsmJit::Immediate MyImmediate1(DLL_PROCESS_ATTACH);
        MyJitFunc.push(MyImmediate1);
        AsmJit::Immediate MyImmediateMod(reinterpret_cast<DWORD_PTR>(RemoteBase));
        MyJitFunc.push(MyImmediateMod);
        MyJitFunc.mov(AsmJit::eax, reinterpret_cast<DWORD_PTR>(pCallback));
        MyJitFunc.call(AsmJit::eax);
      });

      // Entry-point calling code
      AsmJit::Immediate MyImmediate0(0);
      MyJitFunc.push(MyImmediate0);
      AsmJit::Immediate MyImmediate1(DLL_PROCESS_ATTACH);
      MyJitFunc.push(MyImmediate1);
      AsmJit::Immediate MyImmediateMod(reinterpret_cast<DWORD_PTR>(RemoteBase));
      MyJitFunc.push(MyImmediateMod);
      MyJitFunc.mov(AsmJit::eax, reinterpret_cast<DWORD_PTR>(EntryPoint));
      MyJitFunc.call(AsmJit::eax);

      // Export calling code (if necessary)
      if (ExportAddr)
      {
        MyJitFunc.push(MyImmediateMod);
        MyJitFunc.mov(AsmJit::eax, reinterpret_cast<DWORD_PTR>(ExportAddr));
        MyJitFunc.call(AsmJit::eax);
      }

      // Epilogue
      MyJitFunc.mov(AsmJit::esp, AsmJit::ebp);
      MyJitFunc.pop(AsmJit::ebp);

      // Return
      MyJitFunc.ret();
      #else 
        #error "Unsupported architecture."
      #endif
      
      // Make JIT function.
      typedef void (*JitFuncT)(HMODULE);
      EnsureAsmJitFree LoaderStub(AsmJit::function_cast<JitFuncT>(
        MyJitFunc.make()));

      // Ensure function creation succeeded
      if (!LoaderStub.Get())
      {
        BOOST_THROW_EXCEPTION(ManualMapError() << 
          ErrorFunction("ManualMap::Map") << 
          ErrorString("Error JIT'ing loader stub."));
      }

      // Get stub size
      DWORD_PTR StubSize = MyJitFunc.codeSize();

      // Output
      std::wcout << "Loader Stub Size: " << StubSize << "." << std::endl;
      std::wcout << "Loader Stub (Local): " << LoaderStub.Get() << "." << 
        std::endl;

      // Allocate memory for stub buffer
      AllocAndFree EpCallerMem(m_Memory, StubSize);
      // Copy loader stub to stub buffer
      std::vector<BYTE> EpCallerBuf(reinterpret_cast<PBYTE>(LoaderStub.Get()), 
        reinterpret_cast<PBYTE>(LoaderStub.Get()) + StubSize);
      // Write stub buffer to process
      m_Memory.Write(EpCallerMem.GetAddress(), EpCallerBuf);

      // Output
      std::wcout << "Loader Stub (Remote): " << EpCallerMem.GetAddress() << 
        "." << std::endl;

      // Execute EP calling stub
      m_Memory.Call<BOOL (PVOID)>(EpCallerMem.GetAddress(), RemoteBase);

      // Return pointer to module in remote process
      return RemoteBase;
    }

    // Convert RVA to file offset
    DWORD_PTR ManualMap::RvaToFileOffset(PIMAGE_NT_HEADERS pNtHeaders, 
      DWORD_PTR Rva)
    {
      // Get number of sections
      WORD NumSections = pNtHeaders->FileHeader.NumberOfSections;
      // Get pointer to first section
      PIMAGE_SECTION_HEADER pCurrentSection = IMAGE_FIRST_SECTION(pNtHeaders);

      // Loop over all sections
      for (WORD i = 0; i < NumSections; ++i)
      {
        // Check if the RVA may be inside the current section
        if (pCurrentSection->VirtualAddress <= Rva)
        {
          // Check if the RVA is inside the current section
          if ((pCurrentSection->VirtualAddress + pCurrentSection->
            Misc.VirtualSize) > Rva)
          {
            // Convert RVA to file (raw) offset
            Rva -= pCurrentSection->VirtualAddress;
            Rva += pCurrentSection->PointerToRawData;
            return Rva;
          }
        }

        // Advance to next section
        ++pCurrentSection;
      }

      // Could not perform conversion. Return number signifying an error.
      return static_cast<DWORD>(-1);
    }

    // Map sections
    void ManualMap::MapSections(PIMAGE_NT_HEADERS pNtHeaders, PVOID RemoteAddr, 
      std::vector<BYTE> const& ModBuffer)
    {
      // Debug output
      std::wcout << "Mapping sections." << std::endl;

      // Loop over all sections 
      PIMAGE_SECTION_HEADER pCurrent = IMAGE_FIRST_SECTION(pNtHeaders);
      for(WORD i = 0; i != pNtHeaders->FileHeader.NumberOfSections; ++i, 
        ++pCurrent) 
      {
        // Debug output
        std::string Name(pCurrent->Name, pCurrent->Name + 8);
        std::wcout << "Section Name: " << Name.c_str() << std::endl;

        // Calculate target address for section in remote process
        PVOID TargetAddr = reinterpret_cast<PBYTE>(RemoteAddr) + 
          pCurrent->VirtualAddress;
        std::wcout << "Target Address: " << TargetAddr << std::endl;

        // Calculate virtual size of section
        DWORD VirtualSize = pCurrent->Misc.VirtualSize; 
        std::wcout << "Virtual Size: " << TargetAddr << std::endl;

        // Calculate start and end of section data in buffer
        PBYTE DataStart = const_cast<PBYTE>(&ModBuffer[0]) + 
          pCurrent->PointerToRawData;
        PBYTE DataEnd = DataStart + pCurrent->SizeOfRawData;

        // Get section data
        std::vector<BYTE> SectionData(DataStart, DataEnd);
        
        // Write section data to process
        m_Memory.Write(TargetAddr, SectionData);

        // Set the proper page protections for this section
        DWORD OldProtect;
        if (!VirtualProtectEx(m_Memory.GetProcessHandle(), TargetAddr, 
          VirtualSize, pCurrent->Characteristics & 0x00FFFFFF, &OldProtect))
        {
          DWORD LastError = GetLastError();
          BOOST_THROW_EXCEPTION(ManualMapError() << 
            ErrorFunction("ManualMap::MapSections") << 
            ErrorString("Could not change page protections for section.") << 
            ErrorCodeWin(LastError));
        }
      }
    }

    // Fix imports
    void ManualMap::FixImports(std::vector<BYTE>& ModBuffer, 
      PIMAGE_NT_HEADERS pNtHeaders, PIMAGE_IMPORT_DESCRIPTOR pImpDesc)
    {
      // Debug output
      std::wcout << "Fixing imports." << std::endl;

      // Loop through all the required modules
      char* ModuleName = nullptr;
      PBYTE ModBase = &ModBuffer[0];
      for (; pImpDesc && pImpDesc->Name; ++pImpDesc) 
      {
        // Get module name
        ModuleName = reinterpret_cast<char*>(ModBase + RvaToFileOffset(
          pNtHeaders, pImpDesc->Name));
        std::string ModuleNameA(ModuleName);
        auto ModuleNameW(boost::lexical_cast<std::wstring>(ModuleName));
        std::wcout << "Module Name: " << ModuleNameW << "." << std::endl;

        // Check whether dependent module is already loaded
        auto ModuleList(GetModuleList(m_Memory));
        auto ModIter = std::find_if(ModuleList.begin(), ModuleList.end(), 
          [&ModuleNameW] (std::shared_ptr<Module> Current)
        {
          return I18n::ToLower<wchar_t>(Current->GetName()) == 
            I18n::ToLower<wchar_t>(ModuleNameW) || 
            I18n::ToLower<wchar_t>(Current->GetPath()) == 
            I18n::ToLower<wchar_t>(ModuleNameW);
        });

        // Module base address and name
        HMODULE CurModBase = 0;
        std::wstring CurModName;

        // If dependent module is not yet loaded then inject it
        if (ModIter == ModuleList.end())
        {
          // Inject dependent DLL
          std::wcout << "Injecting dependent DLL." << std::endl;
          Injector MyInjector(m_Memory);
          DWORD ReturnValue = 0;
          CurModBase = MyInjector.InjectDll(ModuleNameW, "", ReturnValue, 
            false);
          CurModName = ModuleNameW;
        }
        else
        {
          CurModBase = (*ModIter)->GetBase();
          CurModName = (*ModIter)->GetName();
        }

        // Lookup the first import thunk for this module
        // Todo: Forwarded import support
        auto pThunkData = reinterpret_cast<PIMAGE_THUNK_DATA>(ModBase + 
          RvaToFileOffset(pNtHeaders, pImpDesc->FirstThunk));
        while(pThunkData->u1.AddressOfData) 
        {
          // Get import data
          auto pNameImport = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(ModBase + 
            RvaToFileOffset(pNtHeaders, static_cast<DWORD>(pThunkData->u1.
            AddressOfData)));

          // Get name of function
          std::string const ImpName(reinterpret_cast<char*>(pNameImport->Name));
          std::cout << "Function Name: " << ImpName << "." << std::endl;

          // Get function address in remote process
          bool ByOrdinal = IMAGE_SNAP_BY_ORDINAL(pNameImport->Hint);
          LPCSTR Function = ByOrdinal ? reinterpret_cast<LPCSTR>(IMAGE_ORDINAL(
            pNameImport->Hint)) : reinterpret_cast<LPCSTR>(pNameImport->Name);
          FARPROC FuncAddr = GetRemoteProcAddress(CurModBase, CurModName, 
            Function, ByOrdinal);

          // Set function address
          pThunkData->u1.Function = reinterpret_cast<DWORD_PTR>(FuncAddr);

          // Advance to next function
          pThunkData++;
        }
      } 
    }

    // Get address of export in remote process
    FARPROC ManualMap::GetRemoteProcAddress(HMODULE RemoteMod, 
      std::wstring const& ModulePath, LPCSTR Function, bool ByOrdinal)
    {
      // Load module as data so we can read the EAT locally
      EnsureFreeLibrary LocalMod(LoadLibraryExW(ModulePath.c_str(), NULL, 
        DONT_RESOLVE_DLL_REFERENCES));
      if (!LocalMod)
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(ManualMapError() << 
          ErrorFunction("ManualMap::GetRemoteProcAddress") << 
          ErrorString("Could not load module locally.") << 
          ErrorCodeWin(LastError));
      }

      // Find target function in module
      FARPROC LocalFunc = GetProcAddress(LocalMod, ByOrdinal ? 
        reinterpret_cast<LPCSTR>(IMAGE_ORDINAL(reinterpret_cast<DWORD_PTR>(
        Function))) : Function);
      if (!LocalFunc)
      {
        return nullptr;
      }
      
      // Calculate function delta
      DWORD_PTR FuncDelta = reinterpret_cast<DWORD_PTR>(LocalFunc) - 
        reinterpret_cast<DWORD_PTR>(static_cast<HMODULE>(LocalMod));

      // Calculate function location in remote process
      auto RemoteFunc = reinterpret_cast<FARPROC>(reinterpret_cast<DWORD_PTR>(
        RemoteMod) + FuncDelta);

      // Return remote function location
      return RemoteFunc;
    }

    // Fix relocations
    void ManualMap::FixRelocations(std::vector<BYTE>& ModBuffer, 
      PIMAGE_NT_HEADERS pNtHeaders, PIMAGE_BASE_RELOCATION pRelocDesc, 
      DWORD RelocDirSize, PVOID RemoteBase)
    {
      // Debug output
      std::wcout << "Fixing relocations." << std::endl;

      DWORD_PTR ImageBase = pNtHeaders->OptionalHeader.ImageBase;

      DWORD_PTR Delta = reinterpret_cast<DWORD_PTR>(RemoteBase) - ImageBase;

      PBYTE ModBase = &ModBuffer[0];

      unsigned int BytesProcessed = 0; 
      for (;;) 
      { 
        PVOID RelocBase = ModBase + RvaToFileOffset(pNtHeaders, 
          pRelocDesc->VirtualAddress);

        DWORD NumRelocs = (pRelocDesc->SizeOfBlock - 
          sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD); 

        if(BytesProcessed >= RelocDirSize) 
          break; 

        WORD* pRelocData = reinterpret_cast<WORD*>(reinterpret_cast<PBYTE>(
          pRelocDesc) + sizeof(IMAGE_BASE_RELOCATION)); 
        for(DWORD i = 0; i < NumRelocs; ++i, ++pRelocData) 
        {        
          if(((*pRelocData >> 12) & IMAGE_REL_BASED_HIGHLOW)) 
          {
            *reinterpret_cast<DWORD_PTR*>(static_cast<PBYTE>(RelocBase) + 
              (*pRelocData & 0x0FFF)) += Delta; 
          }
        } 

        BytesProcessed += pRelocDesc->SizeOfBlock; 
        pRelocDesc = reinterpret_cast<PIMAGE_BASE_RELOCATION>(pRelocData); 
      } 
    }
  }
}
