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

#include <Shlobj.h>

// MinGW compatibility workaround
#if defined(HADES_GCC)
#if defined(_M_AMD64) 
extern unsigned __int64 __readgsqword(unsigned long offset);
#elif defined(_M_IX86) 
extern unsigned long __readfsdword(unsigned long offset);
#else 
#error "[HadesMem] Unsupported architecture."
#endif
#endif

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
    
    DWORD const ImageSize = MyNtHeaders.GetSizeOfImage();
    // FIXME: Try to allocate module at preferred base address
    PVOID const RemoteBase = m_Memory.Alloc(ImageSize);
    
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
    
    FixRelocations(MyPeFile, RemoteBase);
    
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
    // FIXME: TLS callbacks should be called from the same thread the EP is 
    // called from.
    // FIXME: Add callback in remote process to ensure TLS callbacks are 
    // called on all new threads etc.
    std::for_each(TlsCallbacks.cbegin(), TlsCallbacks.cend(), 
      [&] (PIMAGE_TLS_CALLBACK pCallback) 
    {
      std::wcout << "TLS Callback: " << pCallback << ".\n";
      std::vector<PVOID> TlsCallArgs;
      TlsCallArgs.push_back(0);
      TlsCallArgs.push_back(reinterpret_cast<PVOID>(DLL_PROCESS_ATTACH));
      TlsCallArgs.push_back(RemoteBase);
      MemoryMgr::RemoteFunctionRet const TlsRet = m_Memory.Call(
        static_cast<PBYTE>(RemoteBase) + reinterpret_cast<DWORD_PTR>(
        pCallback), MemoryMgr::CallConv_Default, TlsCallArgs);
      std::wcout << "TLS Callback Returned: " << TlsRet.GetReturnValue() 
        << ".\n";
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
    
    // FIXME: Investigate whether lpReserved should be 0 or 1 (i.e. dynamic or 
    // static).
    // FIXME: Register an atexit handler to call DllMain again with 
    // DLL_PROCESS_DETACH on process termination.
    if (EntryPoint)
    {
      std::vector<PVOID> EpArgs;
      EpArgs.push_back(0);
      EpArgs.push_back(reinterpret_cast<PVOID>(DLL_PROCESS_ATTACH));
      EpArgs.push_back(RemoteBase);
      MemoryMgr::RemoteFunctionRet const EpRet = m_Memory.Call(EntryPoint, 
        MemoryMgr::CallConv_Default, EpArgs);
      std::wcout << "Entry Point Returned: " << EpRet.GetReturnValue() 
        << ".\n";
      if (!EpRet.GetReturnValue())
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("ManualMap::InjectDll") << 
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

#if 0
// FIXME: Full cleanup and refactor of this module

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

  // Manually map DLL
  // FIXME: Support LoadLibrary/FreeLibrary style refcounting.
  HMODULE ManualMap::InjectDll(std::wstring const& Path, 
      std::string const& Export, 
      InjectFlags Flags, 
      std::wstring const& Parent) const
  {
    std::wcout << Path << " - InjectDll called." << std::endl;
    
    // Do not continue if Shim Engine is enabled for local process, 
    // otherwise it could interfere with the address resolution.
    HMODULE const ShimEngMod = GetModuleHandle(L"ShimEng.dll");
    if (ShimEngMod)
    {
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("ManualMap::Map") << 
        ErrorString("Shims enabled for local process."));
    }
      
    bool PathResolution = ((Flags & InjectFlag_PathResolution) == 
      InjectFlag_PathResolution);
    
    std::wcout << Path << " - Path resolution flag: " << PathResolution 
      << "." << std::endl;
    
    boost::filesystem::path PathTemp(ResolvePath(Path, PathResolution));
    std::wstring const FileNameLower(boost::to_lower_copy(
      PathTemp.filename().native()));
    std::wcout << Path << " - Looking in API Schema list for: " << 
      FileNameLower << std::endl;
    boost::filesystem::path ParentPath(Parent);
    std::wstring const ParentFileNameLower(boost::to_lower_copy(
      ParentPath.filename().native()));
    std::wcout << Path << " - Parent file name is: " << 
      ParentFileNameLower << std::endl;
    
    // Check if module is in the API Schema list
    boost::filesystem::path PathReal;
    auto SchemaIter = m_ApiSchemaDefaults.find(FileNameLower);
    if (SchemaIter != m_ApiSchemaDefaults.cend())
    {
      auto SchemaExceptIter = m_ApiSchemaExceptions.find(FileNameLower);
      if (SchemaExceptIter != m_ApiSchemaExceptions.cend())
      {
        auto FoundIter = std::find_if(SchemaExceptIter->second.cbegin(), 
          SchemaExceptIter->second.cend(), 
          [&] (ApiSchemaExceptionPair const& Exception)
          {
            return boost::filesystem::equivalent(
              ResolvePath(Exception.first, false), 
              ParentPath);
          });
        if (FoundIter != SchemaExceptIter->second.cend())
        {
          std::cout << "Detected API schema exception." << std::endl;
          PathReal = ResolvePath(FoundIter->second, false);
        }
      }
      
      if (PathReal.empty())
      {
        std::cout << "Detected API schema default." << std::endl;
        PathReal = ResolvePath(SchemaIter->second, false);
      }
    }
    
    if (PathReal.empty())
    {
      PathReal = PathTemp;
    }
    
    auto const MappedModIter = m_MappedMods.find(boost::to_lower_copy(
      PathReal.native()));
    if (MappedModIter != m_MappedMods.end())
    {
      std::cout << "InjectDll called on previously mapped or redirected "
        "module." << std::endl;
      return MappedModIter->second;
    }
    
    std::wcout << "Original Path: " << Path << std::endl;
    std::wcout << "Real Path: " << PathReal << std::endl;
      
    // Handle NTDLL.dll as a special case.
    // FIXME: Don't assume NTDLL at any location on disk is the 'real' NTDLL
    std::wstring const ResolvedFileName = boost::to_lower_copy(
      boost::filesystem::path(PathReal).filename().native());
    if (ResolvedFileName == L"ntdll.dll")
    {
      std::wcout << "Detected NTDLL as special case." << std::endl;
      Module NtdllMod(m_Memory, L"ntdll.dll");
      return NtdllMod.GetHandle();
    }
    
    std::wcout << PathReal << " - Reading file." << std::endl;
    
    boost::filesystem::basic_ifstream<char> ModuleFile(PathReal, 
      std::ios::binary | std::ios::ate);
    if (!ModuleFile)
    {
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("ManualMap::Map") << 
        ErrorString("Could not open image file."));
    }
    
    std::streamsize const FileSize = ModuleFile.tellg();

    // Allocate memory to hold file data
    // Doing this rather than copying data into a vector to avoid having to 
    // play with the page protection flags on the heap.
    char* const pBase = static_cast<char*>(VirtualAlloc(nullptr, 
      static_cast<SIZE_T>(FileSize), MEM_COMMIT | MEM_RESERVE, 
      PAGE_READWRITE));
    if (!pBase)
    {
      DWORD const LastError = GetLastError();
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("ManualMap::Map") << 
        ErrorString("Could not allocate memory for image data.") << 
        ErrorCodeWinLast(LastError));
    }
    Detail::EnsureReleaseRegion const EnsureFreeLocalMod(pBase);
    
    if (!ModuleFile.seekg(0, std::ios::beg))
    {
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("ManualMap::Map") << 
        ErrorString("Could not seek to beginning of file."));
    }
    
    if (!ModuleFile.read(pBase, FileSize))
    {
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("ManualMap::Map") << 
        ErrorString("Could not read file into memory."));
    }
    
    MemoryMgr MyMemoryLocal(GetCurrentProcessId());
    
    std::wcout << PathReal << " - Performing PE file format validation." 
      << std::endl;
    PeFile MyPeFile(MyMemoryLocal, pBase, PeFile::FileType_Data);
    DosHeader const MyDosHeader(MyPeFile);
    NtHeaders const MyNtHeaders(MyPeFile);
    
    std::wcout << PathReal << " - Allocating remote memory for image." 
      << std::endl;
    PVOID const RemoteBase = m_Memory.Alloc(MyNtHeaders.GetSizeOfImage());
    std::wcout << PathReal << " - Image base address: " << RemoteBase << "." 
      << std::endl;
    std::wcout << PathReal << " - Image size: " << std::hex << 
      MyNtHeaders.GetSizeOfImage() << std::dec << "." << std::endl;
    
    m_MappedMods[boost::to_lower_copy(PathReal.native())] = 
      reinterpret_cast<HMODULE>(RemoteBase);
    
    std::vector<PIMAGE_TLS_CALLBACK> TlsCallbacks;
    TlsDir const MyTlsDir(MyPeFile);
    if (MyTlsDir.IsValid())
    {
      TlsCallbacks = MyTlsDir.GetCallbacks();
      std::for_each(TlsCallbacks.cbegin(), TlsCallbacks.cend(), 
        [&] (PIMAGE_TLS_CALLBACK pCurrent)
      {
        std::wcout << PathReal << " - TLS Callback: " << pCurrent << std::endl;
      });
    }
    
    std::wcout << PathReal << " - Writing DOS header." << std::endl;
    std::wcout << PathReal << " - DOS Header: " << RemoteBase << std::endl;
    m_Memory.Write(RemoteBase, *reinterpret_cast<PIMAGE_DOS_HEADER>(
      pBase));
    
    PBYTE const NtHeadersStart = reinterpret_cast<PBYTE>(MyNtHeaders.
      GetBase());
    PBYTE const NtHeadersEnd = static_cast<PBYTE>(
      Section(MyPeFile, 0).GetBase());
    std::vector<BYTE> const PeHeaderBuf(NtHeadersStart, NtHeadersEnd);
    PBYTE const TargetAddr = static_cast<PBYTE>(RemoteBase) + MyDosHeader.
      GetNewHeaderOffset();
    std::wcout << PathReal << " - Writing NT header." << std::endl;
    std::wcout << PathReal << " - NT Header: " << 
      static_cast<PVOID>(TargetAddr) << std::endl;
    m_Memory.WriteList(TargetAddr, PeHeaderBuf);
    
    FixRelocations(MyPeFile, RemoteBase);
    
    MapSections(MyPeFile, RemoteBase);

    // Import table must be processed in remote process due to cyclic 
    // depdendencies.
    PeFile RemotePeFile(m_Memory, RemoteBase);
    FixImports(RemotePeFile, PathReal.native());
    
    // FIXME: Should TLS callbacks be called before or after the entry point?
    std::for_each(TlsCallbacks.cbegin(), TlsCallbacks.cend(), 
      [&] (PIMAGE_TLS_CALLBACK pCallback) 
    {
      std::wcout << PathReal << " - TLS Callback: " << pCallback << "." 
        << std::endl;
      std::vector<PVOID> TlsCallArgs;
      TlsCallArgs.push_back(0);
      TlsCallArgs.push_back(reinterpret_cast<PVOID>(DLL_PROCESS_ATTACH));
      TlsCallArgs.push_back(RemoteBase);
      MemoryMgr::RemoteFunctionRet const TlsRet = 
        m_Memory.Call(reinterpret_cast<PBYTE>(RemoteBase) + 
        reinterpret_cast<DWORD_PTR>(pCallback), 
        MemoryMgr::CallConv_Default, TlsCallArgs);
      std::wcout << PathReal << " - TLS Callback Returned: " << 
        TlsRet.GetReturnValue() << "." << std::endl;
    });
    
    PVOID EntryPoint = nullptr;
    DWORD AddressOfEP = MyNtHeaders.GetAddressOfEntryPoint();
    if (AddressOfEP)
    {
      EntryPoint = static_cast<PBYTE>(RemoteBase) + 
        MyNtHeaders.GetAddressOfEntryPoint();
    }
    
    std::wcout << PathReal << " - Entry Point: " << EntryPoint << "." 
      << std::endl;
    
    // FIXME: Throw an exception if the entry point returns failure.
    // FIXME: Investigate whether lpReserved should be 0 or 1 (i.e. dynamic or 
    // static).
    // FIXME: Register an atexit handler to call DllMain again with 
    // DLL_PROCESS_DETACH on process termination.
    if (EntryPoint)
    {
      std::vector<PVOID> EpArgs;
      EpArgs.push_back(0);
      EpArgs.push_back(reinterpret_cast<PVOID>(DLL_PROCESS_ATTACH));
      EpArgs.push_back(RemoteBase);
      MemoryMgr::RemoteFunctionRet const EpRet = m_Memory.Call(EntryPoint, 
        MemoryMgr::CallConv_Default, EpArgs);
      std::wcout << PathReal << " - Entry Point Returned: " << 
        EpRet.GetReturnValue() << "." << std::endl;
    }
    
    Detail::EnsureFreeLibrary const LocalMod(LoadLibraryEx(
      PathReal.c_str(), nullptr, DONT_RESOLVE_DLL_REFERENCES));
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
    
    std::wcout << PathReal << " - Export Address: " << ExportAddr << "." 
      << std::endl;
    
    if (ExportAddr)
    {
      std::vector<PVOID> ExpArgs;
      ExpArgs.push_back(RemoteBase);
      MemoryMgr::RemoteFunctionRet const ExpRet = m_Memory.Call(ExportAddr, 
        MemoryMgr::CallConv_Default, ExpArgs);
      std::wcout << PathReal << " - Export Returned: " << 
        ExpRet.GetReturnValue() << "." << std::endl;
    }
    
    return reinterpret_cast<HMODULE>(RemoteBase);
  }

  // Map sections
  void ManualMap::MapSections(PeFile& MyPeFile, PVOID RemoteAddr) const
  {
    std::cout << "Mapping sections." << std::endl;
    
    SectionList Sections(MyPeFile);
    std::for_each(Sections.cbegin(), Sections.cend(), 
      [&] (Section const& S)
      {
        std::string const Name(S.GetName());
        std::cout << "Section Name: " << Name.c_str() << std::endl;
        
        PVOID const TargetAddr = reinterpret_cast<PBYTE>(RemoteAddr) + 
          S.GetVirtualAddress();
        std::cout << "Target Address: " << TargetAddr << std::endl;
        
        DWORD const VirtualSize = S.GetVirtualSize(); 
        std::cout << "Virtual Size: " << std::hex << VirtualSize << std::dec 
          << std::endl;
        
        DWORD const SizeOfRawData = S.GetSizeOfRawData();
        PBYTE const DataStart = static_cast<PBYTE>(MyPeFile.GetBase()) + 
          S.GetPointerToRawData();
        PBYTE const DataEnd = DataStart + SizeOfRawData;
        
        std::vector<BYTE> const SectionData(DataStart, DataEnd);
        
        if (!SectionData.empty())
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
        
        DWORD OldProtect;
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
  void ManualMap::FixImports(PeFile& MyPeFile, 
    std::wstring const& ParentPath) const
  {
    NtHeaders const MyNtHeaders(MyPeFile);

    ImportDir const CheckImpDir(MyPeFile);
    if (!CheckImpDir.IsValid())
    {
      std::cout << "Image has no imports." << std::endl;
      return;
    }
    
    std::cout << "Fixing imports." << std::endl;
    
    ImportDirList ImportDirs(MyPeFile);
    std::for_each(ImportDirs.begin(), ImportDirs.end(), 
      [&] (ImportDir const& I)
      {
        std::string const ModuleName(I.GetName());
        std::wstring const ModuleNameW(boost::lexical_cast<std::wstring>(
          ModuleName));
        std::wstring const ModuleNameLowerW(boost::to_lower_copy(
          ModuleNameW));
        std::cout << "Module Name: " << ModuleName << "." << std::endl;
        
        // FIXME: Implement proper path resolution logic as per the Windows 
        // PE loader
        std::wstring ModulePathReal(ResolvePath(ModuleNameLowerW, false));
        std::wcout << "Module Path: " << ModulePathReal << "." << std::endl;
        
        HMODULE CurModBase = nullptr;
        
        try
        {
          // FIXME: Bump load count of dependent module to ensure it's not 
          // unloaded prematurely
          
          Module RemoteMod(m_Memory, ModulePathReal);
          std::wcout << "Found existing instance of dependent DLL." 
            << std::endl;
          CurModBase = RemoteMod.GetHandle();
        }
        catch (std::exception const&)
        {
          auto const MappedModIter = m_MappedMods.find(boost::to_lower_copy(
            ModulePathReal));
          if (MappedModIter != m_MappedMods.end())
          {
            std::cout << "Found existing manually mapped instance of dependent "
              "DLL." << std::endl;
            CurModBase = MappedModIter->second;
          }
          else
          {
            std::wcout << "Manually mapping dependent DLL. " << ModulePathReal 
              << "." << std::endl;
            std::wcout << "Attempting without path resolution." << std::endl;
            CurModBase = InjectDll(ModulePathReal, "", InjectFlag_None, 
              ParentPath);
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
          
          if (T.ByOrdinal())
          {
            std::cout << "Function Ordinal: " << T.GetOrdinal() << "." 
              << std::endl;
          }
          else
          {
            std::cout << "Function Name: " << T.GetName() << "." << std::endl;
          }
          
          boost::optional<Export> TargetExport;
          
          if (T.ByOrdinal())
          {
            TargetExport = Export(DepPeFile, T.GetOrdinal());
          }
          // Attempt lookup by hint
          else
          {
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
                  std::cout << "Hint matched!" << std::endl;
                    
                  WORD* pOrdinals = static_cast<WORD*>(DepPeFile.RvaToVa(
                    DepExportDir.GetAddressOfNameOrdinals()));
                  
                  WORD const HintOrdinal = m_Memory.Read<WORD>(pOrdinals + 
                    ImpHint);
                  
                  TargetExport = Export(DepPeFile, HintOrdinal + 
                    DepExportDir.GetOrdinalBase());
                  
                  if (TargetExport->GetName() != HintName)
                  {
                    std::cout << "Hint name mismatch!" << std::endl;
                    throw std::exception();
                  }
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

  // Fix relocations
  void ManualMap::FixRelocations(PeFile& MyPeFile, PVOID pRemoteBase) const
  {
    NtHeaders const MyNtHeaders(MyPeFile);
    
    DWORD const RelocDirSize = MyNtHeaders.GetDataDirectorySize(NtHeaders::
      DataDir_BaseReloc);
    PIMAGE_BASE_RELOCATION pRelocDir = 
      static_cast<PIMAGE_BASE_RELOCATION>(MyPeFile.RvaToVa(MyNtHeaders.
      GetDataDirectoryVirtualAddress(NtHeaders::DataDir_BaseReloc)));
    if (!RelocDirSize || !pRelocDir)
    {
      std::cout << "Image has no relocations." << std::endl;
      return;
    }
    
    PVOID pRelocDirEnd = reinterpret_cast<PBYTE>(pRelocDir) + RelocDirSize;
    
    std::cout << "Fixing relocations." << std::endl;
    
    ULONG_PTR const ImageBase = MyNtHeaders.GetImageBase();
    
    LONG_PTR const Delta = reinterpret_cast<ULONG_PTR>(pRemoteBase) - 
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
          std::cout << "Unsupported relocation type: " << RelocType << 
            std::endl;

          BOOST_THROW_EXCEPTION(Error() << 
            ErrorFunction("ManualMap::FixRelocations") << 
            ErrorString("Unsuppported relocation type."));
        }
      }
      
      pRelocDir = reinterpret_cast<PIMAGE_BASE_RELOCATION>(pRelocData); 
    }
  }
    
  // Perform path resolution
  std::wstring ManualMap::ResolvePath(std::wstring const& Path, bool PathResolution) const
  {
    boost::filesystem::path PathReal(Path);
    
    if (PathResolution)
    {
      if (PathReal.is_relative())
      {
        PathReal = boost::filesystem::absolute(PathReal, 
          Detail::GetSelfDirPath());
      }
      
      // Ensure target file exists
      // Note: Only performing this check when path resolution is enabled, 
      // because otherwise we would need to perform the check in the context 
      // of the remote process, which is not possible to do without 
      // introducing race conditions and other potential problems. So we just 
      // let LoadLibraryW do the check for us.
      if (!boost::filesystem::exists(PathReal))
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("ManualMap::ResolvePath") << 
          ErrorString("Could not find module file."));
      }
    }

    // If path resolution is disabled, replicate the Windows DLL search order 
    // to try and find the target.
    // FIXME: Not a complete implementation of the Windows DLL search order 
    // algorithm. The following conditions need to be supported:
    // 1. If a DLL with the same module name is already loaded in memory, the 
    // system checks only for redirection and a manifest before resolving to 
    // the loaded DLL, no matter which directory it is in. The system does not 
    // search for the DLL.
    // 2. If the DLL is on the list of known DLLs for the version of Windows 
    // on which the application is running, the system uses its copy of the 
    // known DLL (and the known DLL's dependent DLLs, if any) instead of 
    // searching for the DLL. For a list of known DLLs on the current system, 
    // see the following registry key: HKEY_LOCAL_MACHINE\SYSTEM
    // \CurrentControlSet\Control\Session Manager\KnownDLLs.
    // 3. If a DLL has dependencies, the system searches for the dependent 
    // DLLs as if they were loaded with just their module names. This is true 
    // even if the first DLL was loaded by specifying a full path.
    // FIXME: Furthermore, this implementation does not search the 16-bit 
    // system directory, nor does it search the current working directory (as 
    // that is only meaningful in the context of the remote process), lastly, 
    // it does not search in %PATH%.
    // Note: Should we search in the client library directory rather than the 
    // target process directory?
    if (!PathResolution && PathReal.is_relative())
    {
      boost::filesystem::path AppLoadDir = m_Memory.GetProcessPath();
      AppLoadDir = AppLoadDir.parent_path();
      
      boost::filesystem::path SystemDir;
      wchar_t Temp;
      UINT SysDirLen = GetSystemDirectory(&Temp, 1);
      if (!SysDirLen)
      {
        DWORD const LastError = GetLastError();
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("ManualMap::ResolvePath") << 
          ErrorString("Could not get length of system dir.") << 
          ErrorCodeWinLast(LastError));
      }
      std::vector<wchar_t> SysDirTemp(SysDirLen);
      if (!GetSystemDirectory(SysDirTemp.data(), SysDirLen))
      {
        DWORD const LastError = GetLastError();
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("ManualMap::ResolvePath") << 
          ErrorString("Could not get system dir.") << 
          ErrorCodeWinLast(LastError));
      }
      SystemDir = SysDirTemp.data();
      
      boost::filesystem::path WindowsDir;
      UINT WinDirLen = GetSystemDirectory(&Temp, 1);
      if (!WinDirLen)
      {
        DWORD const LastError = GetLastError();
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("ManualMap::ResolvePath") << 
          ErrorString("Could not get length of windows dir.") << 
          ErrorCodeWinLast(LastError));
      }
      std::vector<wchar_t> WinDirTemp(WinDirLen);
      if (!GetSystemDirectory(WinDirTemp.data(), WinDirLen))
      {
        DWORD const LastError = GetLastError();
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("ManualMap::ResolvePath") << 
          ErrorString("Could not get windows dir.") << 
          ErrorCodeWinLast(LastError));
      }
      WindowsDir = WinDirTemp.data();
      
      std::vector<boost::filesystem::path> SearchDirList;
      SearchDirList.push_back(AppLoadDir);
      SearchDirList.push_back(SystemDir);
      SearchDirList.push_back(WindowsDir);
      
      boost::filesystem::path ResolvedPath;
      for (auto i = SearchDirList.cbegin(); i != SearchDirList.cend(); ++i)
      {
        boost::filesystem::path const& Current = *i;
        boost::filesystem::path ResolvedPathTemp = boost::filesystem::absolute(
          PathReal, Current);
        if (boost::filesystem::exists(ResolvedPathTemp))
        {
          ResolvedPath = ResolvedPathTemp;
          break;
        }
      }
      
      if (ResolvedPath.empty())
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("ManualMap::ResolvePath") << 
          ErrorString("Could not find module file."));
      }
      
      PathReal = ResolvedPath;
    }
    
    PathReal.make_preferred();
    
    return PathReal.native();
  }
  
  // Resolve export
  FARPROC ManualMap::ResolveExport(Export const& E, 
    std::wstring const& ParentPath) const
  {
    if (E.Forwarded())
    {
      std::cout << "Forwarded export detected. Forwarder: " << 
        E.GetForwarder() << "." << std::endl;
      std::wcout << "Parent: " << ParentPath << "." << std::endl;
      
      std::string ForwarderModuleLower = boost::to_lower_copy(
        E.GetForwarderModule());
      // FIXME: This is a nasty hack. Perform GetModuleHandle style path 
      // checking.
      if (ForwarderModuleLower.find('.') == std::string::npos)
      {
        ForwarderModuleLower += ".dll";
      }
      
      bool IsNtdll = ForwarderModuleLower == "ntdll.dll";
      
      std::wstring const ForwarderModuleLowerW(boost::lexical_cast<std::wstring>(
        ForwarderModuleLower));
      
      // Check if module is in the API Schema list
      boost::filesystem::path PathReal;
      auto SchemaIter = m_ApiSchemaDefaults.find(ForwarderModuleLowerW);
      if (SchemaIter != m_ApiSchemaDefaults.cend())
      {
        auto SchemaExceptIter = m_ApiSchemaExceptions.find(ForwarderModuleLowerW);
        if (SchemaExceptIter != m_ApiSchemaExceptions.cend())
        {
          auto FoundIter = std::find_if(SchemaExceptIter->second.cbegin(), 
            SchemaExceptIter->second.cend(), 
            [&] (ApiSchemaExceptionPair const& Exception)
            {
              return boost::filesystem::equivalent(
                ResolvePath(Exception.first, false), 
                ParentPath);
            });
          if (FoundIter != SchemaExceptIter->second.cend())
          {
            std::cout << "Detected API schema exception." << std::endl;
            PathReal = ResolvePath(FoundIter->second, false);
          }
        }
        
        if (PathReal.empty())
        {
          std::cout << "Detected API schema default." << std::endl;
          PathReal = ResolvePath(SchemaIter->second, false);
        }
      }
      
      if (PathReal.empty())
      {
        // FIXME: Support both path resolution cases
        PathReal = ResolvePath(boost::lexical_cast<std::wstring>(
          ForwarderModuleLower), false);
      }
      
      auto const ForwardedModIter = m_MappedMods.find(boost::to_lower_copy(
        PathReal.native()));
      
      if (ForwardedModIter == m_MappedMods.end() && !IsNtdll)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("ManualMap::ResolveExport") << 
          ErrorString("Found unknown forwarder module."));
      }
      
      HMODULE NewTarget = nullptr;
      if (IsNtdll)
      {
        Module NtdllMod(m_Memory, L"ntdll.dll");
        NewTarget = NtdllMod.GetHandle();
      }
      else
      {
        NewTarget = ForwardedModIter->second;
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
        std::cout << "Resolving forwarded export by ordinal." << std::endl;
        return ResolveExport(Export(NewTargetPe, ForwarderOrdinal), 
          PathReal.native());
      }
      else
      {
        std::cout << "Resolving forwarded export by name." << std::endl;
        return ResolveExport(FindExport(NewTargetPe, ForwarderFunction), 
          PathReal.native());
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
    std::cout << "FindExport - " << Name << "." << std::endl;
      
    Export Target(MyPeFile, Name);
    
    if (Target.GetName() != Name)
    {
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("ManualMap::FindExport") << 
        ErrorString("Name mismatch."));
    }
    
    return Target;
  }
  
  namespace
  {
    std::wstring GetName(wchar_t* str, WORD size)
    {
      return std::wstring(str, str + size / 2);
    }
  }
    
  // Initialize API set schema defaults
  void ManualMap::InitializeApiSchema() const
  {
    // FIXME: Do version detection. This should only be done on Windows 7 
    // (and above?).
    
    std::wcout << "Initializing API Schema." << std::endl;
    
#if defined(_M_AMD64) 
  	DWORD_PTR headerBase = *reinterpret_cast<DWORD_PTR *>(
  	  __readgsqword(0x60) + 0x68);
#elif defined(_M_IX86) 
  	DWORD_PTR headerBase = *reinterpret_cast<DWORD_PTR *>(
  	  __readfsdword(0x30) + 0x38);
#else 
#error "[HadesMem] Unsupported architecture."
#endif
  	ApiSetMapHeader *header = reinterpret_cast<ApiSetMapHeader *>(headerBase);
  
  	ApiSetModuleEntry *entries = reinterpret_cast<ApiSetModuleEntry *>(
  	  &header[1]);
  	for (DWORD i = 0; i < header->NumModules; ++i)
  	{
  		ApiSetModuleEntry *entry = &entries[i];
  		std::wstring name = GetName(reinterpret_cast<wchar_t *>(
  		  headerBase + entry->OffsetToName), entry->NameSize);
  		
  		std::wstring dllName = L"API-" + name + L".dll";
  		boost::to_lower(dllName);
  		std::wcout << dllName << ":\n";
  
  		auto hostsHeader = reinterpret_cast<ApiSetModuleHostsHeader*>(
  		  headerBase + entry->OffsetOfHosts);
  		auto hosts = reinterpret_cast<ApiSetModuleHost *>(&hostsHeader[1]);
  		for (DWORD j = 0; j < hostsHeader->NumHosts; ++j)
  		{
  			ApiSetModuleHost *host = &hosts[j];
  			std::wstring hostName = GetName(reinterpret_cast<wchar_t *>(
  			  headerBase + host->OffsetOfHostName), host->HostNameSize);
  			boost::to_lower(hostName);
  
  			if (j == 0)
  			{
				  std::wcout << "\tDefault: " << hostName << "\n";
  			  m_ApiSchemaDefaults[dllName] = hostName;
  			}
  			else
  			{
  				std::wstring importerName = GetName(reinterpret_cast<wchar_t *>(
  				  headerBase + host->OffsetOfImportingName), host->ImportingNameSize);
  			  boost::to_lower(importerName);
  				std::wcout << "\t" << importerName << " -> " << hostName << "\n";
  				m_ApiSchemaExceptions[dllName].push_back(
  				  std::make_pair(importerName, hostName));
  			}
  		}
  	}
  }
  
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
}
#endif
