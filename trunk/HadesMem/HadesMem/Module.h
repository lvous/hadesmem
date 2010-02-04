#pragma once

// Windows API
#include <Windows.h>
#include <TlHelp32.h>

// C++ Standard Library
#include <string>
#include <vector>
#include <iostream>

// Boost
#pragma warning(push, 1)
#pragma warning(disable: 4706)
#include <boost/exception.hpp>
#pragma warning(pop)

// HadesMem
#include "Error.h"
#include "Memory.h"

namespace Hades
{
  namespace Memory
  {
    // Module exception type
    class ModuleError : public virtual HadesMemError
    { };

    // Get modules list
    inline std::vector<std::shared_ptr<class Module>> GetModuleList(
      MemoryMgr const& MyMemory);

    // Module wide stream overload
    inline std::wostream& operator<< (std::wostream& Out, 
      class Module const& In);

    // Module managing class
    class Module
    {
    public:
      // Find module by name
      Module(std::wstring const& ModuleName, MemoryMgr const& MyMemory);
      // Find module by handle
      Module(HMODULE Handle, MemoryMgr const& MyMemory);

      // Get module base
      HMODULE GetBase() const;
      // Get module size
      DWORD GetSize() const;

      // Get module name
      std::wstring GetName() const;
      // Get module path
      std::wstring GetPath() const;

      // Whether module was found
      bool Found() const;

    private:
      // Disable assignment
      Module& operator= (Module const&);

      // Memory instance
      MemoryMgr const& m_Memory;

      // Module base address
      HMODULE m_Base;
      // Module size
      DWORD m_Size;
      // Module name
      std::wstring m_Name;
      // Module path
      std::wstring m_Path;

      // Whether module was found
      bool m_Found;
    };

    // Get module list
    inline std::vector<std::shared_ptr<class Module>> GetModuleList(
      MemoryMgr const& MyMemory) 
    {
      // Grab a new snapshot of the process
      EnsureCloseHandle const Snap(CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 
        MyMemory.GetProcessID()));
      if (Snap == INVALID_HANDLE_VALUE)
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(ModuleError() << 
          ErrorFunction("GetModuleList") << 
          ErrorString("Could not get module snapshot.") << 
          ErrorCodeWin(LastError));
      }

      // Container to hold module list
      std::vector<std::shared_ptr<Module>> ModList;

      // Search for process
      MODULEENTRY32 ModEntry = { sizeof(ModEntry) };
      for (BOOL MoreMods = Module32First(Snap, &ModEntry); MoreMods; 
        MoreMods = Module32Next(Snap, &ModEntry)) 
      {
        ModList.push_back(std::make_shared<Module>(ModEntry.hModule, MyMemory));
      }

      // Return module list
      return ModList;
    }

    // Module wide stream overload
    inline std::wostream& operator<< (std::wostream& Stream, 
      Module const& MyModule)
    {
      Stream << "Module Base: " << MyModule.GetBase() << "." << std::endl;
      Stream << "Module Size: " << MyModule.GetSize() << "." << std::endl;
      Stream << "Module Name: " << MyModule.GetName() << "." << std::endl;
      Stream << "Module Path: " << MyModule.GetPath() << "." << std::endl;
      return Stream;
    }

    // Find module by name
    Module::Module(std::wstring const& ModuleName, MemoryMgr const& MyMemory) 
      : m_Memory(MyMemory), 
      m_Base(nullptr), 
      m_Size(0), 
      m_Name(), 
      m_Path(), 
      m_Found(false) 
    {
      // Grab a new snapshot of the process
      EnsureCloseHandle const Snap(CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 
        MyMemory.GetProcessID()));
      if (Snap == INVALID_HANDLE_VALUE)
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(ModuleError() << 
          ErrorFunction("Module:Module") << 
          ErrorString("Could not get module snapshot.") << 
          ErrorCodeWin(LastError));
      }

      // Search for process
      MODULEENTRY32 ModEntry = { sizeof(ModEntry) };
      bool Found = false;
      for (BOOL MoreMods = Module32First(Snap, &ModEntry); MoreMods; 
        MoreMods = Module32Next(Snap, &ModEntry)) 
      {
        Found = (I18n::ToLower<wchar_t>(ModEntry.szModule) == 
          I18n::ToLower<wchar_t>(ModuleName));
        if (Found)
        {
          break;
        }
      }

      // Check if module was found
      if (Found)
      {
        m_Base = ModEntry.hModule;
        m_Size = ModEntry.modBaseSize;
        m_Name = ModEntry.szModule;
        m_Path = ModEntry.szExePath;
      }

      // Whether module was found
      m_Found = Found;
    }

    // Find module by handle
    Module::Module(HMODULE Handle, MemoryMgr const& MyMemory) 
      : m_Memory(MyMemory), 
      m_Base(nullptr), 
      m_Size(0), 
      m_Name(), 
      m_Path(), 
      m_Found(false)
    {
      // Grab a new snapshot of the process
      EnsureCloseHandle const Snap(CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 
        MyMemory.GetProcessID()));
      if (Snap == INVALID_HANDLE_VALUE)
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(ModuleError() << 
          ErrorFunction("Module:Module") << 
          ErrorString("Could not get module snapshot.") << 
          ErrorCodeWin(LastError));
      }

      // Search for process
      MODULEENTRY32 ModEntry = { sizeof(ModEntry) };
      bool Found = false;
      for (BOOL MoreMods = Module32First(Snap, &ModEntry); MoreMods; 
        MoreMods = Module32Next(Snap, &ModEntry)) 
      {
        Found = (ModEntry.hModule == Handle);
        if (Found)
        {
          break;
        }
      }

      // Check process was found
      if (!Found)
      {
        BOOST_THROW_EXCEPTION(ModuleError() << 
          ErrorFunction("Module::Module") << 
          ErrorString("Could not find module."));
      }

      // Get module data
      m_Base = ModEntry.hModule;
      m_Size = ModEntry.modBaseSize;
      m_Name = ModEntry.szModule;
      m_Path = ModEntry.szExePath;
    }

    // Get module base address
    HMODULE Module::GetBase() const
    {
      return m_Base;
    }

    // Get module size
    DWORD Module::GetSize() const
    {
      return m_Size;
    }

    // Get module name
    std::wstring Module::GetName() const
    {
      return m_Name;
    }

    // Get module path
    std::wstring Module::GetPath() const
    {
      return m_Path;
    }

    // Whether module was found
    bool Module::Found() const
    {
      return m_Found;
    }
  }
}
