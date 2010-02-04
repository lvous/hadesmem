#pragma once

// Windows API
#include <Windows.h>
#include <TlHelp32.h>

// C++ Standard Library
#include <string>

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
    class ModuleError : public virtual HadesMemError
    { };

    class Module
    {
    public:
      Module(std::wstring const& ModuleName, MemoryMgr const& MyMemory);
      Module(HMODULE Handle, MemoryMgr const& MyMemory);

      HMODULE GetBase() const;
      DWORD GetSize() const;

      std::wstring GetName() const;
      std::wstring GetPath() const;

    private:
      // Disable assignment
      Module& operator= (Module const&);

      MemoryMgr const& m_Memory;

      HMODULE m_Base;
      DWORD m_Size;
      std::wstring m_Name;
      std::wstring m_Path;

      bool m_Found;
    };

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

    HMODULE Module::GetBase() const
    {
      return m_Base;
    }

    DWORD Module::GetSize() const
    {
      return m_Size;
    }

    std::wstring Module::GetName() const
    {
      return m_Name;
    }

    std::wstring Module::GetPath() const
    {
      return m_Path;
    }
  }
}
