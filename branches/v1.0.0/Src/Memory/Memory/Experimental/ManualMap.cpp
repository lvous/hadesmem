// Copyright Joshua Boyce 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// This file is part of HadesMem.
// <http://www.raptorfactor.com/> <raptorfactor@raptorfactor.com>

#include <HadesMemory/Experimental/ManualMap.hpp>
#include <HadesMemory/Module.hpp>
#include <HadesMemory/Detail/I18n.hpp>
#include <HadesMemory/PeLib/TlsDir.hpp>
#include <HadesMemory/Detail/Config.hpp>
#include <HadesMemory/Detail/WinAux.hpp>
#include <HadesMemory/PeLib/Section.hpp>
#include <HadesMemory/PeLib/ImportDir.hpp>
#include <HadesMemory/PeLib/DosHeader.hpp>
#include <HadesMemory/PeLib/NtHeaders.hpp>
#include <HadesMemory/Detail/EnsureCleanup.hpp>

#include <vector>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iterator>

#include <boost/range.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/fstream.hpp>

#ifdef HADES_MSVC
#pragma warning(push, 1)
#endif
#ifdef HADES_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#pragma GCC diagnostic ignored "-Wattributes"
#pragma GCC diagnostic ignored "-Wparentheses"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <AsmJit/AsmJit.h>
#ifdef HADES_MSVC
#pragma warning(pop)
#endif
#ifdef HADES_GCC
#pragma GCC diagnostic pop
#endif

#include <Shlobj.h>

// MinGW compatibility workaround
#if defined(HADES_GCC)
#if defined(_M_AMD64) 
extern "C" unsigned __int64 __readgsqword(unsigned long offset);
#elif defined(_M_IX86) 
extern "C" unsigned long __readfsdword(unsigned long offset);
#else 
#error "[HadesMem] Unsupported architecture."
#endif
#endif

// WARNING: Here be dragons...
// This component is highly experimental, and relies on information gleaned 
// from reverse engineering and other 'unofficial' sources. As such, the code 
// is quite compilcated and 'volatile' (future OS versions may break 
// invariants unexpectedly - such as the API Schema Redirection added in 
// Windows 7 as part of the 'MinWin' rewrite). If you don't know what you're 
// doing, it's probably best not to touch this file.

namespace HadesMem
{
  // Constructor
  ManualMap::ManualMap(MemoryMgr const& MyMemory) 
    : m_Memory(MyMemory), 
    m_MappedMods(), 
    m_ApiSchemaDefaults(), 
    m_ApiSchemaExceptions()
  {
    InitializeApiSchema();
  }
  
  // Copy constructor
  ManualMap::ManualMap(ManualMap const& Other)
    : m_Memory(Other.m_Memory), 
    m_MappedMods(Other.m_MappedMods), 
    m_ApiSchemaDefaults(Other.m_ApiSchemaDefaults), 
    m_ApiSchemaExceptions(Other.m_ApiSchemaExceptions)
  { }
  
  // Copy assignment operator
  ManualMap& ManualMap::operator=(ManualMap const& Other)
  {

    this->m_Memory = Other.m_Memory;
    this->m_MappedMods = Other.m_MappedMods;
    this->m_ApiSchemaDefaults = Other.m_ApiSchemaDefaults;
    this->m_ApiSchemaExceptions = Other.m_ApiSchemaExceptions;
    
    return *this;
  }
  
  // Move constructor
  ManualMap::ManualMap(ManualMap&& Other)
    : m_Memory(std::move(Other.m_Memory)), 
    m_MappedMods(std::move(Other.m_MappedMods)), 
    m_ApiSchemaDefaults(std::move(Other.m_ApiSchemaDefaults)), 
    m_ApiSchemaExceptions(std::move(Other.m_ApiSchemaExceptions))
  { }
  
  // Move assignment operator
  ManualMap& ManualMap::operator=(ManualMap&& Other)
  {
    this->m_Memory = std::move(Other.m_Memory);
    this->m_MappedMods = std::move(Other.m_MappedMods);
    this->m_ApiSchemaDefaults = std::move(Other.m_ApiSchemaDefaults);
    this->m_ApiSchemaExceptions = std::move(Other.m_ApiSchemaExceptions);
    
    return *this;
  }
  
  // Destructor
  ManualMap::~ManualMap()
  { }
  
  // Equality operator
  bool ManualMap::operator==(ManualMap const& Rhs) const
  {
    return m_Memory == Rhs.m_Memory;
  }
  
  // Inequality operator
  bool ManualMap::operator!=(ManualMap const& Rhs) const
  {
    return !(*this == Rhs);
  }

  // Manually map DLL
  // FIXME: Support LoadLibrary/FreeLibrary style refcounting.
  HMODULE ManualMap::InjectDll(std::wstring const& Path, 
    std::wstring const& ParentPath, 
    std::string const& Export) const
  {
    std::wcout << "InjectDll called.\n";
    std::wcout << "Path: " << Path << ".\n";
    
    boost::filesystem::path const FullPath(ResolvePath(Path, ParentPath));
    
    std::wcout << "Resolved: " << FullPath << ".\n";
    
    HMODULE PrevInstance = LookupCache(FullPath.native());
    if (PrevInstance)
    {
      std::wcout << "InjectDll called on previously mapped module.\n";
      return PrevInstance;
    }
    
    AllocAndFree const FileLocal(OpenFile(FullPath.native()));
    char* const pBase = static_cast<char*>(FileLocal.GetBase());
    
    std::wcout << "Validating PE file.\n";
    
    MemoryMgr const MyMemoryLocal(GetCurrentProcessId());
    PeFile const MyPeFile(MyMemoryLocal, pBase, PeFile::FileType_Data);
    DosHeader const MyDosHeader(MyPeFile);
    NtHeaders const MyNtHeaders(MyPeFile);
    
    // Try to map module at preferred base address before forcing relocation
    DWORD const ImageSize = MyNtHeaders.GetSizeOfImage();
    PVOID const PreferredBase = reinterpret_cast<PVOID>(
      MyNtHeaders.GetImageBase());
    PVOID RemoteBase = VirtualAllocEx(m_Memory.GetProcessHandle(), 
      PreferredBase, ImageSize, MEM_COMMIT | MEM_RESERVE, 
      PAGE_EXECUTE_READWRITE);
    if (!RemoteBase)
    {
      RemoteBase = m_Memory.Alloc(ImageSize);
    }
    
    std::wcout << "Remote Base: " << RemoteBase << ".\n";
    std::wcout << "Remove Size: " << ImageSize << ".\n";
    
    AddToCache(FullPath.native(), reinterpret_cast<HMODULE>(RemoteBase));
    
    std::vector<PIMAGE_TLS_CALLBACK> TlsCallbacks;
    TlsDir const MyTlsDir(MyPeFile);
    if (MyTlsDir.IsValid())
    {
      std::wcout << "Image has TLS directory.\n";
      std::wcout << "Caching TLS callbacks before modifications.\n";
      TlsCallbacks = MyTlsDir.GetCallbacks();
    }
    
    MapHeaders(MyPeFile, RemoteBase);
    
    if (RemoteBase != PreferredBase)
    {
      FixRelocations(MyPeFile, RemoteBase);
    }
    
    MapSections(MyPeFile, RemoteBase);

    // Import table must be processed in remote process due to cyclic 
    // depdendencies.
    PeFile RemotePeFile(m_Memory, RemoteBase);
    FixImports(RemotePeFile, FullPath.native());
    
    CallInitRoutines(MyPeFile, TlsCallbacks, RemoteBase);
    
    CallExport(FullPath.native(), Export, RemoteBase);
    
    return reinterpret_cast<HMODULE>(RemoteBase);
  }
  
  // Call initialization routines
  void ManualMap::CallInitRoutines(PeFile const& MyPeFile, 
    std::vector<PIMAGE_TLS_CALLBACK> const& TlsCallbacks, 
    PVOID RemoteBase) const
  {
    // FIXME: Investigate whether anything else needs to be done for 'proper' 
    // TLS support, like allocating TLS slots etc.
    // FIXME: Register callback in remote process to ensure TLS callbacks are 
    // called on all new threads etc.
    // FIXME: Register an atexit handler to call DllMain/TLS again with 
    // DLL_PROCESS_DETACH on process termination.
    
    std::for_each(TlsCallbacks.cbegin(), TlsCallbacks.cend(), 
      [&] (PIMAGE_TLS_CALLBACK pCallback) 
    {
      std::wcout << "TLS Callback: " << pCallback << ".\n";
    });
    
    PVOID EntryPoint = nullptr;
    NtHeaders const MyNtHeaders(MyPeFile);
    DWORD AddressOfEP = MyNtHeaders.GetAddressOfEntryPoint();
    if (AddressOfEP)
    {
      EntryPoint = static_cast<PBYTE>(RemoteBase) + 
        MyNtHeaders.GetAddressOfEntryPoint();
    }
    
    std::wcout << "Entry Point: " << EntryPoint << ".\n";
    
    AsmJit::Assembler MyJitFunc;

    AllocAndFree const ReturnValueRemote(m_Memory, sizeof(DWORD_PTR));

#if defined(_M_AMD64) 
    // Prologue
    MyJitFunc.push(AsmJit::rbp);
    MyJitFunc.mov(AsmJit::rbp, AsmJit::rsp);
    
    // Set up parameters
    MyJitFunc.mov(AsmJit::rcx, reinterpret_cast<DWORD_PTR>(RemoteBase));
    MyJitFunc.mov(AsmJit::rdx, AsmJit::Imm(DLL_PROCESS_ATTACH));
    MyJitFunc.mov(AsmJit::r8, AsmJit::Imm(0));

    // Allocate ghost space
    MyJitFunc.sub(AsmJit::rsp, AsmJit::Imm(0x20));

    // Call all TLS callbacks
    std::for_each(TlsCallbacks.cbegin(), TlsCallbacks.cend(), 
      [&] (PIMAGE_TLS_CALLBACK pCallback) 
    {
      DWORD_PTR CallbackReal = reinterpret_cast<DWORD_PTR>(RemoteBase) + 
        reinterpret_cast<DWORD_PTR>(pCallback);
      MyJitFunc.mov(AsmJit::rax, CallbackReal);
      MyJitFunc.call(AsmJit::rax);
      // FIXME: Store return values somewhere
    });
    
    // Call entry point if available
    if (EntryPoint)
    {
      // Call entry point
      MyJitFunc.mov(AsmJit::rax, reinterpret_cast<DWORD_PTR>(EntryPoint));
      MyJitFunc.call(AsmJit::rax);

      // Write return value to memory
      MyJitFunc.mov(AsmJit::rcx, reinterpret_cast<DWORD_PTR>(
        ReturnValueRemote.GetBase()));
      MyJitFunc.mov(AsmJit::qword_ptr(AsmJit::rcx), AsmJit::rax);
    }
    
    // Cleanup ghost space
    MyJitFunc.add(AsmJit::rsp, AsmJit::Imm(0x20));
    
    // Epilogue
    MyJitFunc.mov(AsmJit::rsp, AsmJit::rbp);
    MyJitFunc.pop(AsmJit::rbp);

    // Return
    MyJitFunc.ret();
#elif defined(_M_IX86) 
    // Prologue
    MyJitFunc.push(AsmJit::ebp);
    MyJitFunc.mov(AsmJit::ebp, AsmJit::esp);

    // Call all TLS callbacks
    std::for_each(TlsCallbacks.cbegin(), TlsCallbacks.cend(), 
      [&] (PIMAGE_TLS_CALLBACK pCallback) 
    {
      DWORD_PTR CallbackReal = reinterpret_cast<DWORD_PTR>(RemoteBase) + 
        reinterpret_cast<DWORD_PTR>(pCallback);
      MyJitFunc.push(AsmJit::Imm(0));
      MyJitFunc.push(AsmJit::Imm(DLL_PROCESS_ATTACH));
      MyJitFunc.mov(AsmJit::eax, reinterpret_cast<DWORD_PTR>(RemoteBase));
      MyJitFunc.push(AsmJit::eax);
      MyJitFunc.mov(AsmJit::eax, CallbackReal);
      MyJitFunc.call(AsmJit::eax);
      // FIXME: Store return values somewhere
    });
    
    // Call entry point if available
    if (EntryPoint)
    {
      // Call entry point
      MyJitFunc.mov(AsmJit::eax, reinterpret_cast<DWORD_PTR>(EntryPoint));
      MyJitFunc.call(AsmJit::eax);
      
      // Write return value to memory
      MyJitFunc.mov(AsmJit::ecx, reinterpret_cast<DWORD_PTR>(
        ReturnValueRemote.GetBase()));
      MyJitFunc.mov(AsmJit::dword_ptr(AsmJit::ecx), AsmJit::eax);
    }
    
    // Epilogue
    MyJitFunc.mov(AsmJit::esp, AsmJit::ebp);
    MyJitFunc.pop(AsmJit::ebp);

    // Return
    MyJitFunc.ret(AsmJit::Imm(0x4));
#else 
#error "[HadesMem] Unsupported architecture."
#endif
    
    DWORD_PTR const StubSize = MyJitFunc.getCodeSize();
    
    AllocAndFree const StubMemRemote(m_Memory, StubSize);
    PBYTE const pRemoteStub = static_cast<PBYTE>(StubMemRemote.GetBase());
    DWORD_PTR const pRemoteStubTemp = reinterpret_cast<DWORD_PTR>(
      pRemoteStub);
    
    std::vector<BYTE> CodeReal(StubSize);
    MyJitFunc.relocCode(CodeReal.data(), reinterpret_cast<DWORD_PTR>(
      pRemoteStub));
    
    m_Memory.WriteList(pRemoteStub, CodeReal);
    
    Detail::EnsureCloseHandle const MyThread(CreateRemoteThread(m_Memory.
      GetProcessHandle(), nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(
      pRemoteStubTemp), nullptr, 0, nullptr));
    if (!MyThread)
    {
      DWORD const LastError = GetLastError();
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("ManualMap::CallInitRoutines") << 
        ErrorString("Could not create remote thread.") << 
        ErrorCodeWinLast(LastError));
    }
    
    if (WaitForSingleObject(MyThread, INFINITE) != WAIT_OBJECT_0)
    {
      DWORD const LastError = GetLastError();
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("ManualMap::CallInitRoutines") << 
        ErrorString("Could not wait for remote thread.") << 
        ErrorCodeWinLast(LastError));
    }
    
    if (EntryPoint)
    {
      DWORD_PTR const RetVal = m_Memory.Read<DWORD_PTR>(
        ReturnValueRemote.GetBase());
      std::wcout << "Entry Point Returned: " << RetVal << ".\n";
      if (!RetVal)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("ManualMap::CallInitRoutines") << 
          ErrorString("Entry point returned FALSE."));
      }
    }
  }
  
  // Call export
  void ManualMap::CallExport(std::wstring const& FullPath, 
    std::string const& Export, PVOID RemoteBase) const
  {
    Detail::EnsureFreeLibrary const LocalMod(LoadLibraryEx(
      FullPath.c_str(), nullptr, DONT_RESOLVE_DLL_REFERENCES));
    if (!LocalMod)
    {
      DWORD const LastError = GetLastError();
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("ManualMap::InjectDll") << 
        ErrorString("Could not load module locally.") << 
        ErrorCodeWinLast(LastError));
    }
    
    PVOID ExportAddr = nullptr;
    if (!Export.empty())
    {
      FARPROC const LocalFunc = GetProcAddress(LocalMod, Export.c_str());
      if (!LocalFunc)
      {
        DWORD const LastError = GetLastError();
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("ManualMap::InjectDll") << 
          ErrorString("Could not find target function.") << 
          ErrorCodeWinLast(LastError));
      }
      
      LONG_PTR const FuncDelta = reinterpret_cast<DWORD_PTR>(LocalFunc) - 
        reinterpret_cast<DWORD_PTR>(static_cast<HMODULE>(LocalMod));
      
      FARPROC const RemoteFunc = reinterpret_cast<FARPROC>(
        reinterpret_cast<DWORD_PTR>(RemoteBase) + FuncDelta);
      
      ExportAddr = reinterpret_cast<PVOID const>(reinterpret_cast<DWORD_PTR>(
        RemoteFunc));
    }
    
    std::wcout << "Export Address: " << ExportAddr << ".\n";
    
    if (ExportAddr)
    {
      std::vector<PVOID> ExpArgs;
      ExpArgs.push_back(RemoteBase);
      MemoryMgr::RemoteFunctionRet const ExpRet = m_Memory.Call(ExportAddr, 
        MemoryMgr::CallConv_Default, ExpArgs);
      std::wcout << "Export Returned: " << ExpRet.GetReturnValue() << ".\n";
    }
  }
  
  // Resolve path as per Windows loader
  // FIXME: Support SxS DLL isolation/redirection
  // FIXME: Support manifest DLL redirection
  std::wstring ManualMap::ResolvePath(std::wstring const& Path, 
    std::wstring const& Parent) const
  {
    boost::filesystem::path PathReal(ResolveApiSetSchema(Path, Parent));
    if (PathReal.is_absolute())
    {
      return PathReal.native();
    }
    
    boost::filesystem::path LocalPath(Detail::GetSelfDirPath() / 
      PathReal);
    if (boost::filesystem::exists(LocalPath))
    {
      return LocalPath.native();
    }
    
    wchar_t SysPathTmp[MAX_PATH] = { 0 };
    HRESULT const SysPathRet = SHGetFolderPath(NULL, CSIDL_SYSTEM, NULL, 0, 
      SysPathTmp);
    if(SUCCEEDED(SysPathRet)) 
    {
      boost::filesystem::path SystemPath(SysPathTmp);
      SystemPath /= PathReal;
      if (boost::filesystem::exists(SystemPath))
      {
        return SystemPath.native();
      }
    }
    
    wchar_t WinPathTmp[MAX_PATH] = { 0 };
    HRESULT const WinPathRet = SHGetFolderPath(NULL, CSIDL_WINDOWS, NULL, 0, 
      WinPathTmp);
    if(SUCCEEDED(WinPathRet)) 
    {
      boost::filesystem::path WindowsPath(WinPathTmp);
      WindowsPath /= PathReal;
      if (boost::filesystem::exists(WindowsPath))
      {
        return WindowsPath.native();
      }
    }
    
    std::vector<wchar_t> CurPathTmp(MAX_PATH);
    DWORD CurPathRet = GetCurrentDirectory(MAX_PATH, CurPathTmp.data());
    if (CurPathRet > MAX_PATH)
    {
      CurPathTmp.clear();
      CurPathTmp.resize(CurPathRet);
      CurPathRet = GetCurrentDirectory(CurPathRet, CurPathTmp.data());
    }
    if (!CurPathRet)
    {
      DWORD const LastError = GetLastError();
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("ManualMap::ResolvePath") << 
        ErrorString("Could not get current directory.") << 
        ErrorCodeWinLast(LastError));
    }
    else if (CurPathRet > CurPathTmp.size())
    {
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("ManualMap::ResolvePath") << 
        ErrorString("Could not get current directory. Length mismatch."));
    }
    else
    {
      boost::filesystem::path CurrentPath(CurPathTmp.data());
      CurrentPath /= PathReal;
      if (boost::filesystem::exists(CurrentPath))
      {
        return CurrentPath.native();
      }
    }
    
    std::vector<wchar_t> EnvPathsTmp(32767);
    DWORD EnvPathRet = GetEnvironmentVariable(L"PATH", EnvPathsTmp.data(), 
      static_cast<DWORD>(EnvPathsTmp.size()));
    if (!EnvPathRet)
    {
      DWORD const LastError = GetLastError();
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("ManualMap::ResolvePath") << 
        ErrorString("Could not get 'PATH' environmental variable.") << 
        ErrorCodeWinLast(LastError));
    }
    std::wstringstream EnvPaths(EnvPathsTmp.data());
    std::wstring EnvPathTmp;
    while (std::getline(EnvPaths, EnvPathTmp, L';'))
    {
      boost::filesystem::path EnvPath(EnvPathTmp);
      EnvPath /= PathReal;
      if (boost::filesystem::exists(EnvPath))
      {
        return EnvPath.native();
      }
    }
    
    BOOST_THROW_EXCEPTION(Error() << 
      ErrorFunction("ManualMap::ResolvePath") << 
      ErrorString("Could not resolve path. No suitable match found."));
  }
    
  // Perform ApiSetSchema redirection
  std::wstring ManualMap::ResolveApiSetSchema(std::wstring const& Path, 
    std::wstring const& Parent) const
  {
    boost::filesystem::path PathReal(Path);
    std::wstring const FileName(boost::to_lower_copy(
      PathReal.filename().native()));
    auto DefIter = m_ApiSchemaDefaults.find(FileName);
    if (DefIter != m_ApiSchemaDefaults.cend())
    {
      auto ExceptIter = m_ApiSchemaExceptions.find(FileName);
      if (ExceptIter != m_ApiSchemaExceptions.cend())
      {
        auto FoundIter = std::find_if(ExceptIter->second.cbegin(), 
          ExceptIter->second.cend(), 
          [&] (ApiSchemaExceptionPair const& Exception)
          {
            return boost::filesystem::equivalent(
              ResolvePath(Exception.first), 
              ResolvePath(Parent));
          });
        if (FoundIter != ExceptIter->second.cend())
        {
          std::wcout << "Detected API schema redirection (exception).\n";
          PathReal = ResolvePath(FoundIter->second);
          return PathReal.native();
        }
      }
      
      std::wcout << "Detected API schema redirection (default).\n";
      PathReal = ResolvePath(DefIter->second);
      return PathReal.native();
    }
    
    return PathReal.native();
  }
    
  // Perform ApiSetSchema redirection
  void ManualMap::InitializeApiSchema() const
  {
    OSVERSIONINFO VerInfo;
    ZeroMemory(&VerInfo, sizeof(VerInfo));
    VerInfo.dwOSVersionInfoSize = sizeof(VerInfo);
    if (!GetVersionEx(&VerInfo))
    {
      DWORD const LastError = GetLastError();
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("ManualMap::ResolveApiSetSchema") << 
        ErrorString("Could not get Windows version.") << 
        ErrorCodeWinLast(LastError));
    }
    
    // ApiSetSchema redirection is only present on Windows 7 and above
    if (VerInfo.dwMajorVersion < 6 || (VerInfo.dwMajorVersion == 6 && 
      VerInfo.dwMinorVersion < 1))
    {
      return;
    }
    
    // API Schema types (undocumented)
    struct ApiSetMapHeader
    {
      DWORD Version;
      DWORD NumModules;
    };
    struct ApiSetModuleEntry
    {
      DWORD OffsetToName;
      WORD NameSize;
      DWORD OffsetOfHosts;
    };
    struct ApiSetModuleHostsHeader
    {
      DWORD NumHosts;
    };
    struct ApiSetModuleHost
    {
      DWORD OffsetOfImportingName;
      WORD ImportingNameSize;
      DWORD OffsetOfHostName;
      WORD HostNameSize;
    };
    
    // 'Unofficial' notes from Microsoft seem to indicate that all processes 
    // get an exact duplicate of the same ApiSchema structure, so it's safe to 
    // read it locally.
#if defined(_M_AMD64) 
    unsigned long PebOffset = 0x60;
    unsigned long ApiSchemaOffset = 0x68;
  	ApiSetMapHeader* pHeader = *reinterpret_cast<ApiSetMapHeader**>(
  	  __readgsqword(PebOffset) + ApiSchemaOffset);
#elif defined(_M_IX86) 
    unsigned long PebOffset = 0x30;
    unsigned long ApiSchemaOffset = 0x38;
  	ApiSetMapHeader* pHeader = *reinterpret_cast<ApiSetMapHeader**>(
  	  __readfsdword(PebOffset) + ApiSchemaOffset);
#else 
#error "[HadesMem] Unsupported architecture."
#endif
    DWORD_PTR HeaderBase = reinterpret_cast<DWORD_PTR>(pHeader);
    
  	ApiSetModuleEntry* pEntries = reinterpret_cast<ApiSetModuleEntry*>(
  	  &pHeader[1]);
  	for (DWORD i = 0; i < pHeader->NumModules; ++i)
  	{
  		auto GetName = 
  		  [] (DWORD_PTR Base, DWORD Offset, WORD Size) -> std::wstring
  		  {
  		    wchar_t* Name = reinterpret_cast<wchar_t*>(Base + Offset);
  		    return std::wstring(Name, Name + Size / 2);
  		  };
  		
  		ApiSetModuleEntry* pEntry = &pEntries[i];
  		std::wstring EntryName(GetName(HeaderBase, pEntry->OffsetToName, 
  		  pEntry->NameSize));
  		EntryName = L"api-" + EntryName + L".dll";
  		boost::to_lower(EntryName);
  		
  		std::wcout << "ApiSetSchema Entry: " << EntryName << "\n";
  
  		auto pHostsHeader = reinterpret_cast<ApiSetModuleHostsHeader*>(
  		  reinterpret_cast<DWORD_PTR>(pHeader) + pEntry->OffsetOfHosts);
  		auto pHosts = reinterpret_cast<ApiSetModuleHost*>(&pHostsHeader[1]);
  		for (DWORD j = 0; j < pHostsHeader->NumHosts; ++j)
  		{
  			ApiSetModuleHost* pHost = &pHosts[j];
  			std::wstring HostName(GetName(reinterpret_cast<DWORD_PTR>(pHeader), 
  			  pHost->OffsetOfHostName, pHost->HostNameSize));
  			boost::to_lower(HostName);
  
  			if (j == 0)
  			{
				  std::wcout << "\tDefault: " << HostName << "\n";
				  
  			  m_ApiSchemaDefaults[EntryName] = HostName;
  			}
  			else
  			{
  			  std::wstring ImporterName(GetName(HeaderBase, 
  			    pHost->OffsetOfImportingName, pHost->ImportingNameSize));
  			  boost::to_lower(ImporterName);
  			  
  				std::wcout << "\t" << ImporterName << " -> " << HostName << "\n";
  				
  				m_ApiSchemaExceptions[EntryName].push_back(std::make_pair(
  				  ImporterName, HostName));
  			}
  		}
  	}
  	
  	return;
  }
  
  // Add module to cache
  void ManualMap::AddToCache(std::wstring const& Path, HMODULE Base) const
  {
    auto const Iter = m_MappedMods.find(boost::to_lower_copy(Path));
    if (Iter != m_MappedMods.end())
    {
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("ManualMap::AddToCache") << 
        ErrorString("Attempt to add existing module to cache."));
    }
    
    m_MappedMods[boost::to_lower_copy(Path)] = Base;
  }
  
  // Find module in cache
  HMODULE ManualMap::LookupCache(std::wstring const& Path) const
  {
    auto const Iter = m_MappedMods.find(boost::to_lower_copy(Path));
    if (Iter != m_MappedMods.end())
    {
      return Iter->second;
    }
    
    return nullptr;
  }
    
  // Map PE headers
  void ManualMap::MapHeaders(PeFile const& MyPeFile, PVOID RemoteBase) const
  {
    std::wcout << "Mapping PE headers.\n";
    
    m_Memory.Write(RemoteBase, *reinterpret_cast<PIMAGE_DOS_HEADER>(
      MyPeFile.GetBase()));
    
    // FIXME: Should other data such as the MS-DOS stub etc which lies in 
    // the header range but is not in the header structure be written?
    
    DosHeader const MyDosHeader(MyPeFile);
    NtHeaders const MyNtHeaders(MyPeFile);
    PBYTE const pNtBeg = reinterpret_cast<PBYTE>(MyNtHeaders.GetBase());
    // FIXME: Ensure that getting the end of the NT headers this way is 
    // correct.
    PBYTE const pNtEnd = static_cast<PBYTE>(Section(MyPeFile, 0).GetBase());
    std::vector<BYTE> const NtBuf(pNtBeg, pNtEnd);
    PBYTE const NtRemote = static_cast<PBYTE>(RemoteBase) + MyDosHeader.
      GetNewHeaderOffset();
    m_Memory.WriteList(NtRemote, NtBuf);
  }
    
  // Get local PE file memory
  AllocAndFree ManualMap::OpenFile(std::wstring const& Path) const
  {
    std::wcout << "Opening PE file.\n";
    
    std::uintmax_t const FileSize = boost::filesystem::file_size(Path);
    if (!FileSize || FileSize == static_cast<std::uintmax_t>(-1))
    {
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("ManualMap::OpenFile") << 
        ErrorString("Invalid file size."));
    }
    
    std::wcout << "File Size: " << FileSize << ".\n";
    
    boost::filesystem::basic_ifstream<char> File(Path, std::ios::binary);
    if (!File)
    {
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("ManualMap::OpenFile") << 
        ErrorString("Could not open file for reading."));
    }
    
    MemoryMgr const MyMemoryLocal(GetCurrentProcessId());
    AllocAndFree FileLocal(MyMemoryLocal, static_cast<SIZE_T>(FileSize));
    char* const pBase = static_cast<char*>(FileLocal.GetBase());
    
    if (!File.read(pBase, FileSize))
    {
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("ManualMap::OpenFile") << 
        ErrorString("Could not read file into buffer."));
    }
    
    return FileLocal;
  }

  // Fix relocations
  void ManualMap::FixRelocations(PeFile const& MyPeFile, 
    PVOID RemoteBase) const
  {
    std::wcout << "Fixing relocations.\n";
    
    NtHeaders const MyNtHeaders(MyPeFile);
    
    DWORD const RelocDirSize = MyNtHeaders.GetDataDirectorySize(
      NtHeaders::DataDir_BaseReloc);
    DWORD const RelocDirRva = MyNtHeaders.GetDataDirectoryVirtualAddress(
      NtHeaders::DataDir_BaseReloc);
    auto pRelocDir = static_cast<PIMAGE_BASE_RELOCATION>(MyPeFile.RvaToVa(
      RelocDirRva));
    if (!RelocDirSize || !pRelocDir)
    {
      std::wcout << "Warning! Image has no relocations.\n";
      return;
    }
    
    PVOID pRelocDirEnd = reinterpret_cast<PBYTE>(pRelocDir) + RelocDirSize;
    
    ULONG_PTR const ImageBase = MyNtHeaders.GetImageBase();
    
    LONG_PTR const Delta = reinterpret_cast<ULONG_PTR>(RemoteBase) - 
      ImageBase;
    
    while (pRelocDir < pRelocDirEnd && pRelocDir->SizeOfBlock > 0)
    {
      PBYTE const RelocBase = static_cast<PBYTE>(MyPeFile.RvaToVa(
        pRelocDir->VirtualAddress));
      
      DWORD const NumRelocs = (pRelocDir->SizeOfBlock - sizeof(
        IMAGE_BASE_RELOCATION)) / sizeof(WORD); 
      
      PWORD pRelocData = reinterpret_cast<PWORD>(pRelocDir + 1);
      
      for(DWORD i = 0; i < NumRelocs; ++i, ++pRelocData) 
      {
        BYTE RelocType = *pRelocData >> 12;
        WORD Offset = *pRelocData & 0xFFF;
        
        switch (RelocType)
        {
        case IMAGE_REL_BASED_ABSOLUTE:
          break;

        case IMAGE_REL_BASED_HIGHLOW:
          *reinterpret_cast<DWORD32*>(RelocBase + Offset) += 
            static_cast<DWORD32>(Delta);
          break;

        case IMAGE_REL_BASED_DIR64:
          *reinterpret_cast<DWORD64*>(RelocBase + Offset) += Delta;
          break;

        default:
          std::wcout << "Unsupported relocation type: " << RelocType << ".\n";

          BOOST_THROW_EXCEPTION(Error() << 
            ErrorFunction("ManualMap::FixRelocations") << 
            ErrorString("Unsuppported relocation type."));
        }
      }
      
      pRelocDir = reinterpret_cast<PIMAGE_BASE_RELOCATION>(pRelocData); 
    }
  }

  // Map sections
  void ManualMap::MapSections(PeFile const& MyPeFile, PVOID RemoteBase) const
  {
    std::wcout << "Mapping sections.\n";
    
    SectionList Sections(MyPeFile);
    std::for_each(Sections.cbegin(), Sections.cend(), 
      [&] (Section const& S)
      {
        std::string const Name(S.GetName());
        std::wcout << "Section Name: " << Name.c_str() << ".\n";
        
        PVOID const TargetAddr = reinterpret_cast<PBYTE>(RemoteBase) + 
          S.GetVirtualAddress();
        std::wcout << "Section Address: " << TargetAddr << ".\n";
        
        DWORD const VirtualSize = S.GetVirtualSize(); 
        std::wcout << "Section Size: " << VirtualSize << ".\n";
        
        DWORD const SizeOfRawData = S.GetSizeOfRawData();
        PBYTE const DataStart = static_cast<PBYTE>(MyPeFile.GetBase()) + 
          S.GetPointerToRawData();
        PBYTE const DataEnd = DataStart + SizeOfRawData;
        
        std::vector<BYTE> const SectionData(DataStart, DataEnd);
        
        if (SectionData.empty())
        {
          std::wcout << "Warning! Empty section.\n";
        }
        else
        {
          m_Memory.WriteList(TargetAddr, SectionData);
        }
        
        DWORD SecCharacteristics = S.GetCharacteristics();
        
        std::array<ULONG, 16> const SectionCharacteristicsToProtect = 
        {{
          PAGE_NOACCESS, 
          PAGE_NOACCESS, 
          PAGE_EXECUTE, 
          PAGE_EXECUTE, 
          PAGE_READONLY, 
          PAGE_READONLY, 
          PAGE_EXECUTE_READ, 
          PAGE_EXECUTE_READ, 
          PAGE_READWRITE, 
          PAGE_READWRITE, 
          PAGE_EXECUTE_READWRITE, 
          PAGE_EXECUTE_READWRITE, 
          PAGE_READWRITE, 
          PAGE_READWRITE, 
          PAGE_EXECUTE_READWRITE, 
          PAGE_EXECUTE_READWRITE, 
        }};
        
        // Handle case where no explicit protection is provided
        if((SecCharacteristics & (IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ | 
          IMAGE_SCN_MEM_WRITE)) == 0)
        {
          if(SecCharacteristics & IMAGE_SCN_CNT_CODE)
          {
            SecCharacteristics |= IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ;
          }
  
          if(SecCharacteristics & IMAGE_SCN_CNT_INITIALIZED_DATA)
          {
            SecCharacteristics |= IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;
          }
  
          if(SecCharacteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA)
          {
            SecCharacteristics |= IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;
          }
        }
        
        DWORD SecProtect = SectionCharacteristicsToProtect[
          SecCharacteristics >> 28];
        
        DWORD OldProtect = 0;
        if (!VirtualProtectEx(m_Memory.GetProcessHandle(), TargetAddr, 
          VirtualSize, SecProtect, &OldProtect))
        {
          DWORD const LastError = GetLastError();
          BOOST_THROW_EXCEPTION(ManualMap::Error() << 
            ErrorFunction("ManualMap::MapSections") << 
            ErrorString("Could not change page protections for section.") << 
            ErrorCodeWinLast(LastError));
        }
      });
  }

  // Fix imports
  // FIXME: Support delay loaded imports.
  // FIXME: Build hash tables for quick lookup.
  // FIXME: Parse EAT of each module and load all modules referenced by 
  // forwarders.
  // FIXME: Support bound imports.
  // FIXME: Do redirection on APIs that would otherwise fail (e.g. 
  // GetModuleHandle, GetModuleFileName, etc), including redirecting indirect 
  // calls through GetProcAddress. Allow manually mapped modules to see other 
  // manually mapped modules.
  void ManualMap::FixImports(PeFile const& MyPeFile, 
    std::wstring const& ParentPath) const
  {
    std::wcout << "Fixing imports.\n";
    
    ImportDir const ImpDir(MyPeFile);
    if (!ImpDir.IsValid())
    {
      std::wcout << "Warning! Image has no imports.\n";
      return;
    }
    
    ImportDirList ImportDirs(MyPeFile);
    std::for_each(ImportDirs.begin(), ImportDirs.end(), 
      [&] (ImportDir const& I)
      {
        std::wstring const ModuleName(boost::to_lower_copy(
          boost::lexical_cast<std::wstring>(I.GetName())));
        
        std::wcout << "Module Name: " << ModuleName << "." << std::endl;
        
        std::wstring const ModulePath(boost::to_lower_copy(
          ResolvePath(ModuleName, ParentPath)));
        
        std::wcout << "Module Path: " << ModulePath << ".\n";
        
        HMODULE CurModBase = nullptr;
        
        try
        {
          // FIXME: Bump load count of dependent module to ensure it's not 
          // unloaded prematurely
          
          Module RemoteMod(m_Memory, ModulePath);
          
          std::wcout << "Found existing instance of dependent DLL.\n";
          
          CurModBase = RemoteMod.GetHandle();
        }
        catch (std::exception const&)
        {
          HMODULE CacheBase = LookupCache(ModulePath);
          if (CacheBase)
          {
            std::wcout << "Found existing manually mapped instance of "
              "dependent DLL.\n";
            
            CurModBase = CacheBase;
          }
          else
          {
            std::wcout << "Manually mapping dependent DLL.\n";
            
            CurModBase = InjectDll(ModulePath, ParentPath);
          }          
        }
        
        PeFile DepPeFile(m_Memory, CurModBase);
        
        // Lift export list out of loop to allow for caching
        ExportList Exports(DepPeFile);
        
        ImportThunkList ImportOrigThunks(MyPeFile, I.GetCharacteristics());
        ImportThunkList ImportFirstThunks(MyPeFile, I.GetFirstThunk());
        for (auto j = ImportOrigThunks.cbegin(); j != ImportOrigThunks.cend(); ++j)
        {
          ImportThunk const& T = *j;
          
          boost::optional<Export> TargetExport;
          
          if (T.ByOrdinal())
          {
            std::wcout << "Function Ordinal: " << T.GetOrdinal() << ".\n";
            
            TargetExport = Export(DepPeFile, T.GetOrdinal());
          }
          // Attempt lookup by hint
          else
          {
            std::wcout << "Function Name: " << T.GetName().c_str() << ".\n";
            
            ExportDir DepExportDir(DepPeFile);
            DWORD const ImpHint = T.GetHint();
            DWORD const NumberOfNames = DepExportDir.GetNumberOfNames();
            if (DepExportDir.IsValid() && ImpHint && ImpHint < NumberOfNames)
            {
              try
              {
                DWORD* pNames = static_cast<DWORD*>(DepPeFile.RvaToVa(
                  DepExportDir.GetAddressOfNames()));              
                DWORD const HintNameRva = m_Memory.Read<DWORD>(pNames + ImpHint);
                std::string const HintName = m_Memory.ReadString<std::string>(
                  DepPeFile.RvaToVa(HintNameRva));
                
                if (HintName == T.GetName())
                {
                  WORD* pOrdinals = static_cast<WORD*>(DepPeFile.RvaToVa(
                    DepExportDir.GetAddressOfNameOrdinals()));
                  
                  WORD const HintOrdinal = m_Memory.Read<WORD>(pOrdinals + 
                    ImpHint);
                  
                  Export TempExport(DepPeFile, HintOrdinal + 
                    DepExportDir.GetOrdinalBase());
                  
                  if (TempExport.GetName() != HintName)
                  {
                    std::wcout << "Error! Hint name mismatch.\n";
                    throw std::exception();
                  }
                  
                  TargetExport = TempExport;
                }
                else
                {
                  std::wcout << "Hint invalid.\n";
                }
              }
              catch (std::exception const& /*e*/)
              { }
            }
          }
  
          // If lookup by ordinal or hint failed do a manual lookup
          if (!TargetExport)
          {
            TargetExport = FindExport(DepPeFile, T.GetName());
          }
          
          FARPROC FuncAddr = ResolveExport(*TargetExport, ParentPath);
          
          if (!FuncAddr)
          {
            BOOST_THROW_EXCEPTION(ManualMap::Error() << 
              ErrorFunction("ManualMap::FixImports") << 
              ErrorString("Could not find current import."));
          }
          
          auto ImpThunkFT = ImportFirstThunks.begin();
          std::advance(ImpThunkFT, std::distance(ImportOrigThunks.cbegin(), j));
          ImpThunkFT->SetFunction(reinterpret_cast<DWORD_PTR>(FuncAddr));
        }
      });
  }
  
  // Resolve export
  FARPROC ManualMap::ResolveExport(Export const& E, 
    std::wstring const& ParentPath) const
  {
    if (E.Forwarded())
    {
      std::wcout << "Forwarded export detected.\n";
      std::wcout << "Forwarder: " << E.GetForwarder().c_str() << ".\n";
      std::wcout << "Parent: " << ParentPath << ".\n";
      
      std::wstring ModuleName = boost::to_lower_copy(
        boost::lexical_cast<std::wstring>(E.GetForwarderModule()));
      // FIXME: This is a nasty hack. Perform GetModuleHandle style path 
      // checking.
      if (ModuleName.find(L'.') == std::string::npos)
      {
        ModuleName += L".dll";
      }
      
      bool IsNtdll = (ModuleName == L"ntdll.dll");
      
      boost::filesystem::path const ModulePath(ResolvePath(
        ModuleName, ParentPath));
      
      HMODULE NewTarget = nullptr;
      if (IsNtdll)
      {
        Module NtdllMod(m_Memory, L"ntdll.dll");
        NewTarget = NtdllMod.GetHandle();
      }
      else
      {
        HMODULE const CacheBase = LookupCache(ModulePath.native());
        if (!CacheBase)
        {
          BOOST_THROW_EXCEPTION(Error() << 
            ErrorFunction("ManualMap::ResolveExport") << 
            ErrorString("Found unknown forwarder module."));
        }
        
        NewTarget = CacheBase;
      }
      
      std::string const ForwarderFunction = E.GetForwarderFunction();
      bool ForwardedByOrdinal = (ForwarderFunction[0] == '#');
      WORD ForwarderOrdinal = 0;
      if (ForwardedByOrdinal)
      {
        try
        {
          ForwarderOrdinal = boost::lexical_cast<WORD>(ForwarderFunction.substr(1));
        }
        catch (std::exception const& /*e*/)
        {
          BOOST_THROW_EXCEPTION(Error() << 
            ErrorFunction("ManualMap::ResolveExport") << 
            ErrorString("Invalid forwarder ordinal detected."));
        }
      }
      
      PeFile NewTargetPe(m_Memory, NewTarget);
      if (ForwardedByOrdinal)
      {
        std::wcout << "Resolving forwarded export by ordinal.\n";
        return ResolveExport(Export(NewTargetPe, ForwarderOrdinal), 
          ModulePath.native());
      }
      else
      {
        std::wcout << "Resolving forwarded export by name.\n";
        return ResolveExport(FindExport(NewTargetPe, ForwarderFunction), 
          ModulePath.native());
      }
    }
    else
    {
      return reinterpret_cast<FARPROC>(reinterpret_cast<DWORD_PTR>(E.GetVa()));
    }
  }
  
  // Find export by name
  Export ManualMap::FindExport(PeFile const& MyPeFile, 
    std::string const& Name) const
  {
    std::wcout << "FindExport: " << Name.c_str() << ".\n";
      
    Export Target(MyPeFile, Name);
    
    if (Target.GetName() != Name)
    {
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("ManualMap::FindExport") << 
        ErrorString("Name mismatch."));
    }
    
    return Target;
  }
}
