// Copyright Joshua Boyce 2010-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// This file is part of HadesMem.
// <http://www.raptorfactor.com/> <raptorfactor@raptorfactor.com>

#pragma once

#include <string>
#include <functional>

#include <boost/config.hpp>

#include <windows.h>
#include <tlhelp32.h>

namespace hadesmem
{

class Process;

// hadesmem::Module causes the following warning under GCC:
// error: 'class hadesmem::Module' has pointer data members 
// but does not override 'hadesmem::Module(const hadesmem::Module&)' 
// or 'operator=(const hadesmem::Module&)' [-Werror=effc++]
// This can be ignored because the pointer data members are non-owning 
// and shared pointers.
#if defined(HADESMEM_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#endif // #if defined(HADESMEM_GCC)

class Module
{
public:
  Module(Process const* process, HMODULE handle);
  
  Module(Process const* process, std::wstring const& path);
  
  Module(Process const* process, MODULEENTRY32 const& entry);
  
  Module(Module const& other);
  
  Module& operator=(Module const& other);
  
  Module(Module&& other) BOOST_NOEXCEPT;
  
  Module& operator=(Module&& other) BOOST_NOEXCEPT;
  
  ~Module() BOOST_NOEXCEPT;
  
  HMODULE GetHandle() const BOOST_NOEXCEPT;
  
  DWORD GetSize() const BOOST_NOEXCEPT;
  
  std::wstring GetName() const;
  
  std::wstring GetPath() const;
  
  FARPROC FindProcedure(std::string const& Name) const;
  
  FARPROC FindProcedure(WORD Ordinal) const;
  
  bool operator==(Module const& other) const BOOST_NOEXCEPT;
  
  bool operator!=(Module const& other) const BOOST_NOEXCEPT;
  
  bool operator<(Module const& other) const BOOST_NOEXCEPT;
  
  bool operator<=(Module const& other) const BOOST_NOEXCEPT;
  
  bool operator>(Module const& other) const BOOST_NOEXCEPT;
  
  bool operator>=(Module const& other) const BOOST_NOEXCEPT;
  
private:
  void Initialize(HMODULE handle);
  
  void Initialize(std::wstring const& path);
  
  void Initialize(MODULEENTRY32 const& entry);
  
  void InitializeIf(std::function<bool (MODULEENTRY32 const&)> const& check_func);
  
  FARPROC FindProcedureInternal(LPCSTR name) const;
  
  Process const* process_;
  HMODULE handle_;
  DWORD size_;
  std::wstring name_;
  std::wstring path_;
};

#if defined(HADESMEM_GCC)
#pragma GCC diagnostic pop
#endif // #if defined(HADESMEM_GCC)

}
