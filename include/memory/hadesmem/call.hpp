// Copyright Joshua Boyce 2010-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// This file is part of HadesMem.
// <http://www.raptorfactor.com/> <raptorfactor@raptorfactor.com>

#pragma once

#include <vector>

#include <windows.h>

namespace hadesmem
{

class Process;

class RemoteFunctionRet
{
public:
  RemoteFunctionRet(DWORD_PTR ReturnValue, DWORD LastError);
  
  DWORD_PTR GetReturnValue() const;
  
  DWORD GetLastError() const;
  
private:
  DWORD_PTR m_ReturnValue;
  DWORD m_LastError;
};

enum class CallConv
{
  kDefault, 
  kCdecl, 
  kStdCall, 
  kThisCall, 
  kFastCall, 
  kX64
};

RemoteFunctionRet Call(Process const& process, 
  LPCVOID address, 
  CallConv call_conv, 
  std::vector<PVOID> const& args);

}
