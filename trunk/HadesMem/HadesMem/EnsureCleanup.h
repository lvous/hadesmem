#pragma once

// Windows API
#include <Windows.h>

// C++ Standard Library
#include <string>

namespace Hades
{
  // Data type representing the address of the object's cleanup function.
  // I used UINT_PTR so that this class works properly in 64-bit Windows.
  typedef VOID (WINAPI* FnCleanup)(UINT_PTR);

  // Each template instantiation requires a data type, address of cleanup 
  // function, and a value that indicates an invalid value.
  template<typename T, FnCleanup pfn, UINT_PTR Invalid = 0> 
  class EnsureCleanup 
  {
  public:
    // Default constructor assumes an invalid value (nothing to cleanup)
    EnsureCleanup() 
    { m_t = Invalid; }

    // This constructor sets the value to the specified value
    EnsureCleanup(T t) : m_t((UINT_PTR) t) 
    { }

    // The destructor performs the cleanup.
    ~EnsureCleanup() 
    { Cleanup(); }

    // Helper methods to tell if the value represents a valid object or not..
    BOOL IsValid() const 
    { return(m_t != Invalid); }
    BOOL IsInvalid() const 
    { return(!IsValid()); }

    // Re-assigning the object forces the current object to be cleaned-up.
    T operator= (T t) 
    { 
      Cleanup(); 
      m_t = (UINT_PTR) t;
      return(*this);  
    }

    // Returns the value (supports both 32-bit and 64-bit Windows).
    operator T() const 
    { return (T) m_t; }

    // Cleanup the object if the value represents a valid object
    void Cleanup() 
    { 
      if (IsValid())
      {
        // In 64-bit Windows, all parameters are 64-bits, 
        // so no casting is required
        pfn(m_t);         // Close the object.
        m_t = Invalid;   // We no longer represent a valid object.
      }
    }

  protected:
    EnsureCleanup(const EnsureCleanup&);

  private:
    UINT_PTR m_t;           // The member representing the object
  };

  // Macros to make it easier to declare instances of the template 
  // class for specific data types.

#define MakeCleanupClass(className, tData, pfnCleanup) \
  typedef EnsureCleanup<tData, (FnCleanup) pfnCleanup> className

#define MakeCleanupClassX(className, tData, pfnCleanup, Invalid) \
  typedef EnsureCleanup<tData, (FnCleanup) pfnCleanup, \
  (INT_PTR) Invalid> className

  // Instances of the template C++ class for common data types.
  MakeCleanupClass(EnsureFindClose, HANDLE, FindClose);
  MakeCleanupClass(EnsureCloseHandle, HANDLE, CloseHandle);
  MakeCleanupClassX(EnsureCloseSnap, HANDLE, CloseHandle, INVALID_HANDLE_VALUE);
  MakeCleanupClass(EnsureLocalFree, HLOCAL, LocalFree);
  MakeCleanupClass(EnsureGlobalFree, HGLOBAL, GlobalFree);
  MakeCleanupClass(EnsureGlobalUnlock, LPVOID, GlobalUnlock);
  MakeCleanupClass(EnsureRegCloseKey, HKEY, RegCloseKey);
  MakeCleanupClass(EnsureCloseServiceHandle, SC_HANDLE, CloseServiceHandle);
  MakeCleanupClass(EnsureCloseWindowStation, HWINSTA, CloseWindowStation);
  MakeCleanupClass(EnsureCloseDesktop, HDESK, CloseDesktop);
  MakeCleanupClass(EnsureUnmapViewOfFile, PVOID, UnmapViewOfFile);
  MakeCleanupClass(EnsureFreeLibrary, HMODULE, FreeLibrary);
  MakeCleanupClass(EnsureRemoveVEH, PVOID, RemoveVectoredExceptionHandler);
  MakeCleanupClass(EnsureResumeThread, HANDLE, ResumeThread);
  MakeCleanupClassX(EnsureCloseFile, HANDLE, CloseHandle, 
    INVALID_HANDLE_VALUE);
  MakeCleanupClass(EnsureUnhookWindowsHookEx, HHOOK, UnhookWindowsHookEx);
  MakeCleanupClass(EnsureDestroyWindow, HWND, DestroyWindow);
  MakeCleanupClass(EnsureFreeSid, PSID, FreeSid);
  MakeCleanupClass(EnsureFreeResource, HGLOBAL, FreeResource);
  MakeCleanupClass(EnsureDeleteDc, HDC, DeleteDC);
  MakeCleanupClass(EnsureDeleteObject, HBITMAP, DeleteObject);

  // Special class for releasing a reserved region.
  // Special class is required because VirtualFree requires 3 parameters
  class EnsureReleaseRegion 
  {
  public:
    EnsureReleaseRegion(PVOID pv = nullptr) : m_pv(pv) 
    { }

    ~EnsureReleaseRegion() 
    { Cleanup(); }

    PVOID operator= (PVOID pv) 
    { 
      Cleanup(); 
      m_pv = pv; 
      return(m_pv); 
    }

    operator PVOID() const 
    { return(m_pv); }

    void Cleanup() 
    { 
      if (m_pv != nullptr) 
      { 
        VirtualFree(m_pv, 0, MEM_RELEASE); 
        m_pv = nullptr; 
      } 
    }

  private:
    PVOID m_pv;
  };

  // Special class for releasing a reserved region.
  // Special class is required because VirtualFree requires 3 parameters
  class EnsureEndUpdateResource 
  {
  public:
    EnsureEndUpdateResource(HANDLE File = nullptr) : m_File(File) 
    { }

    ~EnsureEndUpdateResource() 
    { Cleanup(); }

    PVOID operator= (HANDLE File) 
    { 
      Cleanup(); 
      m_File = File; 
      return(m_File); 
    }

    operator HANDLE() const 
    { return(m_File); }

    void Cleanup() 
    { 
      if (m_File != nullptr) 
      { 
        EndUpdateResource(m_File, FALSE); 
        m_File = nullptr; 
      } 
    }

  private:
    HANDLE m_File;
  };

  // Special class for freeing a block from a heap
  // Special class is required because HeapFree requires 3 parameters
  class EnsureHeapFree 
  {
  public:
    EnsureHeapFree(PVOID pv = nullptr, HANDLE hHeap = GetProcessHeap()) 
      : m_pv(pv), m_hHeap(hHeap) 
    { }
    ~EnsureHeapFree() 
    { Cleanup(); }

    PVOID operator= (PVOID pv) 
    { 
      Cleanup(); 
      m_pv = pv; 
      return(m_pv); 
    }

    operator PVOID() const 
    { return(m_pv); }

    void Cleanup() 
    { 
      if (m_pv != nullptr) 
      { 
        HeapFree(m_hHeap, 0, m_pv); 
        m_pv = nullptr; 
      } 
    }

  private:
    HANDLE m_hHeap;
    PVOID m_pv;
  };

  // Special class for releasing a remote reserved region.
  // Special class is required because VirtualFreeEx requires 4 parameters
  class EnsureReleaseRegionEx
  {
  public:
    EnsureReleaseRegionEx(PVOID pv = nullptr, HANDLE proc = nullptr) 
      : m_pv(pv), m_proc(proc) 
    { }
    ~EnsureReleaseRegionEx() 
    { Cleanup(); }

    PVOID operator= (PVOID pv) 
    { 
      Cleanup(); 
      m_pv = pv; 
      return(m_pv); 
    }

    operator PVOID() const 
    { return(m_pv); }

    void Cleanup() 
    { 
      if (m_pv != nullptr && m_proc != nullptr) 
      { 
        VirtualFreeEx(m_proc, m_pv, 0, MEM_RELEASE); 
        m_pv = nullptr; 
      } 
    }

  private:
    PVOID m_pv;
    HANDLE m_proc;
  };

  // Special class for closing the clipboard.
  // Special class is required because no params are required.
  class EnsureCloseClipboard
  {
  public:
    EnsureCloseClipboard(BOOL Success) : m_Success(Success) 
    { }
    ~EnsureCloseClipboard() 
    { Cleanup(); }

    BOOL operator= (BOOL Success) 
    { 
      Cleanup(); 
      m_Success = Success;
      return(m_Success); 
    }

    operator BOOL() const 
    { return(m_Success); }

    void Cleanup() 
    { 
      if (m_Success) 
      {
        CloseClipboard();
        m_Success = FALSE;
      } 
    }

  private:
    BOOL m_Success;
  };

  // Special class for releasing a window class.
  class EnsureUnregisterClassW
  {
  public:
    EnsureUnregisterClassW(const std::wstring& ClassName, HINSTANCE Instance) 
      : m_ClassName(ClassName), m_Instance(Instance) 
    { }
    ~EnsureUnregisterClassW() 
    { Cleanup(); }

    // Disable assignment
  protected:
    EnsureUnregisterClassW& operator= (const EnsureUnregisterClassW&);

    void Cleanup() 
    { 
      if (!m_ClassName.empty() && m_Instance) 
      { 
        UnregisterClassW(m_ClassName.c_str(), m_Instance); 
        m_ClassName.clear();
        m_Instance = 0; 
      } 
    }

  private:
    std::wstring m_ClassName;
    HINSTANCE m_Instance;
  };


  // Special class for releasing a DC.
  class EnsureReleaseDc
  {
  public:
    EnsureReleaseDc(HWND Wnd = nullptr, HDC Dc = nullptr) 
      : m_Wnd(Wnd), 
      m_Dc(Dc) 
    { }
    ~EnsureReleaseDc() 
    { Cleanup(); }

    HDC operator= (HDC Dc) 
    { 
      Cleanup(); 
      m_Dc = Dc; 
      return m_Dc; 
    }

    operator HDC() const 
    { return m_Dc; }

    void Cleanup() 
    { 
      if (m_Wnd != nullptr && m_Dc != nullptr) 
      { 
        ReleaseDC(m_Wnd, m_Dc); 
        m_Wnd = nullptr; 
        m_Dc = nullptr; 
      } 
    }

  private:
    HWND m_Wnd;
    HDC m_Dc;
  };

}
