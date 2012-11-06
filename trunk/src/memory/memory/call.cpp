// Copyright Joshua Boyce 2010-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// This file is part of HadesMem.
// <http://www.raptorfactor.com/> <raptorfactor@raptorfactor.com>

#include "hadesmem/call.hpp"

#include <cstddef>
#include <algorithm>
#include <type_traits>

#include "hadesmem/detail/warning_disable_prefix.hpp"
#include <boost/scope_exit.hpp>
#include "hadesmem/detail/warning_disable_suffix.hpp"

#include "hadesmem/detail/warning_disable_prefix.hpp"
#include <asmjit/asmjit.h>
#include "hadesmem/detail/warning_disable_suffix.hpp"

#include "hadesmem/read.hpp"
#include "hadesmem/alloc.hpp"
#include "hadesmem/error.hpp"
#include "hadesmem/write.hpp"
#include "hadesmem/module.hpp"
#include "hadesmem/process.hpp"
#include "hadesmem/detail/type_traits.hpp"
#include "hadesmem/detail/static_assert.hpp"

// TODO: Improve safety via EH.

// TODO: Add support for more 'complex' argument and return types, including 
// struct/class/union, long double, SIMD types, etc. A good reference for 
// calling conventions is available at http://goo.gl/5rUxn.

// TODO: Only JIT code for Call once, then cache. Rewrite to pull data 
// externally instead of being regenerated for every call.

namespace hadesmem
{

namespace
{

struct CallResultRemote
{
  DWORD_PTR return_value;
  DWORD32 return_value_32;
  DWORD64 return_value_64;
  float return_value_float;
  double return_value_double;
  DWORD last_error;
};

// CallResultRemote must be POD because of 'offsetof' usage.
HADESMEM_STATIC_ASSERT(std::is_pod<CallResultRemote>::value);

#if defined(_M_IX86)

class ArgVisitor32
{
public:
  ArgVisitor32(AsmJit::X86Assembler* assembler, std::size_t num_args, 
    CallConv call_conv) BOOST_NOEXCEPT;

  void operator()(DWORD32 arg) BOOST_NOEXCEPT;

  void operator()(DWORD64 arg) BOOST_NOEXCEPT;

  void operator()(float arg) BOOST_NOEXCEPT;

  void operator()(double arg) BOOST_NOEXCEPT;

private:
  AsmJit::X86Assembler* assembler_;
  std::size_t num_args_;
  std::size_t cur_arg_;
  CallConv call_conv_;
};

#endif // #if defined(_M_IX86)

#if defined(_M_AMD64)

class ArgVisitor64
{
public:
  ArgVisitor64(AsmJit::X86Assembler* assembler, std::size_t num_args) 
    BOOST_NOEXCEPT;

  void operator()(DWORD32 arg) BOOST_NOEXCEPT;

  void operator()(DWORD64 arg) BOOST_NOEXCEPT;

  void operator()(float arg) BOOST_NOEXCEPT;

  void operator()(double arg) BOOST_NOEXCEPT;

private:
  AsmJit::X86Assembler* assembler_;
  std::size_t num_args_;
  std::size_t cur_arg_;
};

#endif // #if defined(_M_AMD64)

#if defined(_M_IX86)

void GenerateCallCode32(AsmJit::X86Assembler* assembler, 
  std::vector<LPCVOID> const& addresses, 
  std::vector<CallConv> const& call_convs, 
  std::vector<std::vector<CallArg>> const& args_full, 
  DWORD_PTR get_last_error, 
  DWORD_PTR set_last_error, 
  DWORD_PTR is_debugger_present, 
  DWORD_PTR debug_break, 
  PVOID return_values_remote)
{
  AsmJit::Label label_nodebug(assembler->newLabel());

  assembler->push(AsmJit::ebp);
  assembler->mov(AsmJit::ebp, AsmJit::esp);

  assembler->mov(AsmJit::eax, static_cast<sysint_t>(is_debugger_present));
  assembler->call(AsmJit::eax);

  assembler->test(AsmJit::eax, AsmJit::eax);
  assembler->jz(label_nodebug);

  assembler->mov(AsmJit::eax, static_cast<sysint_t>(debug_break));
  assembler->call(AsmJit::eax);

  assembler->bind(label_nodebug);

  assembler->push(AsmJit::imm(0x0));
  assembler->mov(AsmJit::eax, static_cast<sysint_t>(set_last_error));
  assembler->call(AsmJit::eax);

  for (std::size_t i = 0; i < addresses.size(); ++i)
  {
    LPCVOID address = addresses[i];
    CallConv call_conv = call_convs[i];
    std::vector<CallArg> const& args = args_full[i];
    std::size_t const num_args = args.size();

    ArgVisitor32 arg_visitor(assembler, num_args, call_conv);
    std::for_each(args.rbegin(), args.rend(), 
      [&] (CallArg const& arg)
    {
      arg.Visit(&arg_visitor);
    });

    assembler->mov(AsmJit::eax, reinterpret_cast<sysint_t>(address));
    assembler->call(AsmJit::eax);

    assembler->mov(AsmJit::ecx, static_cast<sysint_t>(
      reinterpret_cast<DWORD_PTR>(return_values_remote) + 
      i * sizeof(CallResultRemote) + 
      offsetof(CallResultRemote, return_value)));
    assembler->mov(AsmJit::dword_ptr(AsmJit::ecx), AsmJit::eax);

    assembler->mov(AsmJit::ecx, static_cast<sysint_t>(
      reinterpret_cast<DWORD_PTR>(return_values_remote) + 
      i * sizeof(CallResultRemote) + 
      offsetof(CallResultRemote, return_value_32)));
    assembler->mov(AsmJit::dword_ptr(AsmJit::ecx), AsmJit::eax);

    assembler->mov(AsmJit::ecx, static_cast<sysint_t>(
      reinterpret_cast<DWORD_PTR>(return_values_remote) + 
      i * sizeof(CallResultRemote) + 
      offsetof(CallResultRemote, return_value_64)));
    assembler->mov(AsmJit::dword_ptr(AsmJit::ecx), AsmJit::eax);
    assembler->mov(AsmJit::dword_ptr(AsmJit::ecx, 4), AsmJit::edx);

    assembler->mov(AsmJit::ecx, static_cast<sysint_t>(
      reinterpret_cast<DWORD_PTR>(return_values_remote) + 
      i * sizeof(CallResultRemote) + 
      offsetof(CallResultRemote, return_value_float)));
    assembler->fst(AsmJit::dword_ptr(AsmJit::ecx));

    assembler->mov(AsmJit::ecx, static_cast<sysint_t>(
      reinterpret_cast<DWORD_PTR>(return_values_remote) + 
      i * sizeof(CallResultRemote) + 
      offsetof(CallResultRemote, return_value_double)));
    assembler->fst(AsmJit::qword_ptr(AsmJit::ecx));

    assembler->mov(AsmJit::eax, static_cast<sysint_t>(get_last_error));
    assembler->call(AsmJit::eax);

    assembler->mov(AsmJit::ecx, static_cast<sysint_t>(
      reinterpret_cast<DWORD_PTR>(return_values_remote) + 
      i * sizeof(CallResultRemote) + 
      offsetof(CallResultRemote, last_error)));
    assembler->mov(AsmJit::dword_ptr(AsmJit::ecx), AsmJit::eax);

    if (call_conv == CallConv::kDefault || call_conv == CallConv::kCdecl)
    {
      assembler->add(AsmJit::esp, static_cast<sysint_t>(
        num_args * sizeof(PVOID)));
    }
  }

  assembler->mov(AsmJit::esp, AsmJit::ebp);
  assembler->pop(AsmJit::ebp);

  assembler->ret(0x4);
}

#endif // #if defined(_M_IX86)

#if defined(_M_AMD64)

void GenerateCallCode64(AsmJit::X86Assembler* assembler, 
  std::vector<LPCVOID> const& addresses, 
  std::vector<CallConv> const& call_convs, 
  std::vector<std::vector<CallArg>> const& args_full, 
  DWORD_PTR get_last_error, 
  DWORD_PTR set_last_error, 
  DWORD_PTR is_debugger_present, 
  DWORD_PTR debug_break, 
  PVOID return_values_remote)
{
  AsmJit::Label label_nodebug(assembler->newLabel());

  auto const max_args_list = std::max_element(std::begin(args_full), 
    std::end(args_full), 
    [] (std::vector<CallArg> const& args1, std::vector<CallArg> const& args2)
  {
    return args1.size() < args2.size();
  });
  std::size_t const max_num_args = max_args_list->size();

  std::size_t stack_offs = (std::max)(static_cast<std::size_t>(0x20), 
    max_num_args * 0x8);
  BOOST_ASSERT(stack_offs % 16 == 0 || stack_offs % 16 == 8);
  stack_offs = (stack_offs % 16) ? (stack_offs + 8) : stack_offs;
  stack_offs += 16;
  stack_offs += 8;

  assembler->sub(AsmJit::rsp, stack_offs);

  assembler->mov(AsmJit::rax, is_debugger_present);
  assembler->call(AsmJit::rax);

  assembler->test(AsmJit::rax, AsmJit::rax);
  assembler->jz(label_nodebug);

  assembler->mov(AsmJit::rax, debug_break);
  assembler->call(AsmJit::rax);

  assembler->bind(label_nodebug);

  assembler->mov(AsmJit::rcx, 0);
  assembler->mov(AsmJit::rax, set_last_error);
  assembler->call(AsmJit::rax);

  for (std::size_t i = 0; i < addresses.size(); ++i)
  {
    LPCVOID address = addresses[i];
    std::vector<CallArg> const& args = args_full[i];
    std::size_t const num_args = args.size();

    (void)call_convs;
    BOOST_ASSERT(call_convs[i] == CallConv::kDefault || 
      call_convs[i] == CallConv::kWinApi || 
      call_convs[i] == CallConv::kX64);

    ArgVisitor64 arg_visitor(assembler, num_args);
    std::for_each(args.rbegin(), args.rend(), 
      [&] (CallArg const& arg)
    {
      arg.Visit(&arg_visitor);
    });

    assembler->mov(AsmJit::rax, reinterpret_cast<DWORD_PTR>(address));
    assembler->call(AsmJit::rax);

    assembler->mov(AsmJit::rcx, reinterpret_cast<DWORD_PTR>(
      return_values_remote) + i * sizeof(CallResultRemote) + 
      offsetof(CallResultRemote, return_value));
    assembler->mov(AsmJit::qword_ptr(AsmJit::rcx), AsmJit::rax);

    assembler->mov(AsmJit::rcx, reinterpret_cast<DWORD_PTR>(
      return_values_remote) + i * sizeof(CallResultRemote) + 
      offsetof(CallResultRemote, return_value_32));
    assembler->mov(AsmJit::dword_ptr(AsmJit::rcx), AsmJit::eax);

    assembler->mov(AsmJit::rcx, reinterpret_cast<DWORD_PTR>(
      return_values_remote) + i * sizeof(CallResultRemote) + 
      offsetof(CallResultRemote, return_value_64));
    assembler->mov(AsmJit::qword_ptr(AsmJit::rcx), AsmJit::rax);

    assembler->mov(AsmJit::rcx, reinterpret_cast<DWORD_PTR>(
      return_values_remote) + i * sizeof(CallResultRemote) + 
      offsetof(CallResultRemote, return_value_float));
    assembler->movd(AsmJit::dword_ptr(AsmJit::rcx), AsmJit::xmm0);

    assembler->mov(AsmJit::rcx, reinterpret_cast<DWORD_PTR>(
      return_values_remote) + i * sizeof(CallResultRemote) + 
      offsetof(CallResultRemote, return_value_double));
    assembler->movq(AsmJit::qword_ptr(AsmJit::rcx), AsmJit::xmm0);

    assembler->mov(AsmJit::rax, get_last_error);
    assembler->call(AsmJit::rax);

    assembler->mov(AsmJit::rcx, reinterpret_cast<DWORD_PTR>(
      return_values_remote) + i * sizeof(CallResultRemote) + 
      offsetof(CallResultRemote, last_error));
    assembler->mov(AsmJit::dword_ptr(AsmJit::rcx), AsmJit::eax);
  }

  assembler->add(AsmJit::rsp, stack_offs);

  assembler->ret();
}

#endif // #if defined(_M_AMD64)

Allocator GenerateCallCode(Process const& process, 
  std::vector<LPCVOID> const& addresses, 
  std::vector<CallConv> const& call_convs, 
  std::vector<std::vector<CallArg>> const& args_full, 
  PVOID return_values_remote)
{
  Module const kernel32(&process, L"kernel32.dll");
  auto const get_last_error = reinterpret_cast<DWORD_PTR>(FindProcedure(
    kernel32, "GetLastError"));
  auto const set_last_error = reinterpret_cast<DWORD_PTR>(FindProcedure(
    kernel32, "SetLastError"));
  auto const is_debugger_present = reinterpret_cast<DWORD_PTR>(FindProcedure(
    kernel32, "IsDebuggerPresent"));
  auto const debug_break = reinterpret_cast<DWORD_PTR>(FindProcedure(
    kernel32, "DebugBreak"));

  AsmJit::X86Assembler assembler;
  
#if defined(_M_AMD64)
  GenerateCallCode64(&assembler, addresses, call_convs, args_full, 
    get_last_error, set_last_error, is_debugger_present, debug_break, 
    return_values_remote);
#elif defined(_M_IX86)
  GenerateCallCode32(&assembler, addresses, call_convs, args_full, 
    get_last_error, set_last_error, is_debugger_present, debug_break, 
    return_values_remote);
#else
#error "[HadesMem] Unsupported architecture."
#endif
  
  DWORD_PTR const stub_size = assembler.getCodeSize();
  
  Allocator stub_mem_remote(&process, stub_size);
  
  std::vector<BYTE> code_real(stub_size);
  assembler.relocCode(code_real.data(), reinterpret_cast<DWORD_PTR>(
    stub_mem_remote.GetBase()));
  
  WriteVector(process, stub_mem_remote.GetBase(), code_real);

  return stub_mem_remote;
}

}

#if defined(_M_IX86)

ArgVisitor32::ArgVisitor32(AsmJit::X86Assembler* assembler, 
  std::size_t num_args, CallConv call_conv) BOOST_NOEXCEPT
  : assembler_(assembler), 
  num_args_(num_args), 
  cur_arg_(num_args), 
  call_conv_(call_conv)
{ }

void ArgVisitor32::operator()(DWORD32 arg) BOOST_NOEXCEPT
{
  switch (cur_arg_)
  {
  case 1:
    switch (call_conv_)
    {
    case CallConv::kThisCall:
    case CallConv::kFastCall:
      assembler_->mov(AsmJit::ecx, static_cast<sysint_t>(arg));
      break;
    case CallConv::kDefault:
    case CallConv::kWinApi:
    case CallConv::kCdecl:
    case CallConv::kStdCall:
    case CallConv::kX64:
      assembler_->mov(AsmJit::eax, static_cast<sysint_t>(arg));
      assembler_->push(AsmJit::eax);
      break;
    }
    break;
  case 2:
    switch (call_conv_)
    {
    case CallConv::kFastCall:
      assembler_->mov(AsmJit::edx, static_cast<sysint_t>(arg));
      break;
    case CallConv::kDefault:
    case CallConv::kWinApi:
    case CallConv::kCdecl:
    case CallConv::kStdCall:
    case CallConv::kThisCall:
    case CallConv::kX64:
      assembler_->mov(AsmJit::eax, static_cast<sysint_t>(arg));
      assembler_->push(AsmJit::eax);
      break;
    }
    break;
  default:
    assembler_->mov(AsmJit::eax, static_cast<sysint_t>(arg));
    assembler_->push(AsmJit::eax);
    break;
  }

  --cur_arg_;
}

void ArgVisitor32::operator()(DWORD64 arg) BOOST_NOEXCEPT
{
  assembler_->mov(AsmJit::eax, static_cast<sysint_t>(static_cast<DWORD>(
    (arg >> 32) & 0xFFFFFFFF)));
  assembler_->push(AsmJit::eax);

  assembler_->mov(AsmJit::eax, static_cast<sysint_t>(static_cast<DWORD>(arg)));
  assembler_->push(AsmJit::eax);

  --cur_arg_;
}

void ArgVisitor32::operator()(float arg) BOOST_NOEXCEPT
{
  HADESMEM_STATIC_ASSERT(sizeof(float) == sizeof(DWORD));

  union FloatConv
  {
    float f;
    DWORD i;
  };

  FloatConv float_conv;
  float_conv.f = arg;

  assembler_->mov(AsmJit::eax, static_cast<sysint_t>(float_conv.i));
  assembler_->push(AsmJit::eax);

  --cur_arg_;
}

void ArgVisitor32::operator()(double arg) BOOST_NOEXCEPT
{
  HADESMEM_STATIC_ASSERT(sizeof(double) == sizeof(DWORD64));

  union DoubleConv
  {
    double d;
    DWORD64 i;
  };

  DoubleConv double_conv;
  double_conv.d = arg;

  assembler_->mov(AsmJit::eax, static_cast<sysint_t>(static_cast<DWORD>(
    (double_conv.i >> 32) & 0xFFFFFFFF)));
  assembler_->push(AsmJit::eax);

  assembler_->mov(AsmJit::eax, static_cast<sysint_t>(static_cast<DWORD>(
    double_conv.i)));
  assembler_->push(AsmJit::eax);

  --cur_arg_;
}

#endif // #if defined(_M_IX86)

#if defined(_M_AMD64)

ArgVisitor64::ArgVisitor64(AsmJit::X86Assembler* assembler, 
  std::size_t num_args) BOOST_NOEXCEPT
  : assembler_(assembler), 
  num_args_(num_args), 
  cur_arg_(num_args)
{ }

void ArgVisitor64::operator()(DWORD32 arg) BOOST_NOEXCEPT
{
  return (*this)(static_cast<DWORD64>(arg));
}

void ArgVisitor64::operator()(DWORD64 arg) BOOST_NOEXCEPT
{
  switch (cur_arg_)
  {
  case 1:
    assembler_->mov(AsmJit::rcx, arg);
    break;
  case 2:
    assembler_->mov(AsmJit::rdx, arg);
    break;
  case 3:
    assembler_->mov(AsmJit::r8, arg);
    break;
  case 4:
    assembler_->mov(AsmJit::r9, arg);
    break;
  default:
    assembler_->mov(AsmJit::rax, arg);
    assembler_->mov(AsmJit::qword_ptr(AsmJit::rsp, cur_arg_ * 8 - 8), 
      AsmJit::rax);
    break;
  }

  --cur_arg_;
}

void ArgVisitor64::operator()(float arg) BOOST_NOEXCEPT
{
  HADESMEM_STATIC_ASSERT(sizeof(float) == sizeof(DWORD));

  union FloatConv
  {
    float f;
    DWORD i;
  };

  FloatConv float_conv;
  float_conv.f = arg;

  switch (cur_arg_)
  {
  case 1:
    assembler_->mov(AsmJit::dword_ptr(AsmJit::rsp, num_args_ * 8), 
      static_cast<DWORD>(float_conv.i));
    assembler_->movss(AsmJit::xmm0, AsmJit::dword_ptr(AsmJit::rsp, 
      num_args_ * 8));
    break;
  case 2:
    assembler_->mov(AsmJit::dword_ptr(AsmJit::rsp, num_args_ * 8), 
      static_cast<DWORD>(float_conv.i));
    assembler_->movss(AsmJit::xmm1, AsmJit::dword_ptr(AsmJit::rsp, 
      num_args_ * 8));
    break;
  case 3:
    assembler_->mov(AsmJit::dword_ptr(AsmJit::rsp, num_args_ * 8), 
      static_cast<DWORD>(float_conv.i));
    assembler_->movss(AsmJit::xmm2, AsmJit::dword_ptr(AsmJit::rsp, 
      num_args_ * 8));
    break;
  case 4:
    assembler_->mov(AsmJit::dword_ptr(AsmJit::rsp, num_args_ * 8), 
      static_cast<DWORD>(float_conv.i));
    assembler_->movss(AsmJit::xmm3, AsmJit::dword_ptr(AsmJit::rsp, 
      num_args_ * 8));
    break;
  default:
    assembler_->xor_(AsmJit::rax, AsmJit::rax);
    assembler_->mov(AsmJit::rax, float_conv.i);
    assembler_->mov(AsmJit::qword_ptr(AsmJit::rsp, cur_arg_ * 8 - 8), 
      AsmJit::rax);
    break;
  }

  --cur_arg_;
}

void ArgVisitor64::operator()(double arg) BOOST_NOEXCEPT
{
  HADESMEM_STATIC_ASSERT(sizeof(double) == sizeof(DWORD64));

  union DoubleConv
  {
    double d;
    DWORD64 i;
  };

  DoubleConv double_conv;
  double_conv.d = arg;

  switch (cur_arg_)
  {
  case 1:
    assembler_->mov(AsmJit::dword_ptr(AsmJit::rsp, num_args_ * 8), 
      static_cast<DWORD>(double_conv.i));
    assembler_->mov(AsmJit::dword_ptr(AsmJit::rsp, num_args_ * 8 + 4), 
      static_cast<DWORD>((double_conv.i >> 32) & 0xFFFFFFFF));
    assembler_->movsd(AsmJit::xmm0, AsmJit::qword_ptr(AsmJit::rsp, 
      num_args_ * 8));
    break;
  case 2:
    assembler_->mov(AsmJit::dword_ptr(AsmJit::rsp, num_args_ * 8), 
      static_cast<DWORD>(double_conv.i));
    assembler_->mov(AsmJit::dword_ptr(AsmJit::rsp, num_args_ * 8 + 4), 
      static_cast<DWORD>((double_conv.i >> 32) & 0xFFFFFFFF));
    assembler_->movsd(AsmJit::xmm1, AsmJit::qword_ptr(AsmJit::rsp, 
      num_args_ * 8));
    break;
  case 3:
    assembler_->mov(AsmJit::dword_ptr(AsmJit::rsp, num_args_ * 8), 
      static_cast<DWORD>(double_conv.i));
    assembler_->mov(AsmJit::dword_ptr(AsmJit::rsp, num_args_ * 8 + 4), 
      static_cast<DWORD>((double_conv.i >> 32) & 0xFFFFFFFF));
    assembler_->movsd(AsmJit::xmm2, AsmJit::qword_ptr(AsmJit::rsp, 
      num_args_ * 8));
    break;
  case 4:
    assembler_->mov(AsmJit::dword_ptr(AsmJit::rsp, num_args_ * 8), 
      static_cast<DWORD>(double_conv.i));
    assembler_->mov(AsmJit::dword_ptr(AsmJit::rsp, num_args_ * 8 + 4), 
      static_cast<DWORD>((double_conv.i >> 32) & 0xFFFFFFFF));
    assembler_->movsd(AsmJit::xmm3, AsmJit::qword_ptr(AsmJit::rsp, 
      num_args_ * 8));
    break;
  default:
    assembler_->mov(AsmJit::rax, double_conv.i);
    assembler_->mov(AsmJit::qword_ptr(AsmJit::rsp, cur_arg_ * 8 - 8), 
      AsmJit::rax);
    break;
  }

  --cur_arg_;
}

#endif // #if defined(_M_AMD64)
  
CallResultRaw::CallResultRaw(DWORD_PTR return_int_ptr, 
  DWORD32 return_int_32, 
  DWORD64 return_int_64, 
  float return_float, 
  double return_double, 
  DWORD last_error) BOOST_NOEXCEPT
  : int_ptr_(return_int_ptr), 
  int_32_(return_int_32), 
  int_64_(return_int_64), 
  float_(return_float), 
  double_(return_double), 
  last_error_(last_error)
{ }

DWORD_PTR CallResultRaw::GetReturnValueIntPtr() const BOOST_NOEXCEPT
{
  return int_ptr_;
}

DWORD32 CallResultRaw::GetReturnValueInt32() const BOOST_NOEXCEPT
{
  return int_32_;
}

DWORD64 CallResultRaw::GetReturnValueInt64() const BOOST_NOEXCEPT
{
  return int_64_;
}

float CallResultRaw::GetReturnValueFloat() const BOOST_NOEXCEPT
{
  return float_;
}

double CallResultRaw::GetReturnValueDouble() const BOOST_NOEXCEPT
{
  return double_;
}

DWORD CallResultRaw::GetLastError() const BOOST_NOEXCEPT
{
  return last_error_;
}

CallResultRaw Call(Process const& process, 
  LPCVOID address, 
  CallConv call_conv, 
  std::vector<CallArg> const& args)
{
  std::vector<LPCVOID> addresses;
  addresses.push_back(address);
  std::vector<CallConv> call_convs;
  call_convs.push_back(call_conv);
  std::vector<std::vector<CallArg>> args_full;
  args_full.push_back(args);
  return CallMulti(process, addresses, call_convs, args_full)[0];
}

std::vector<CallResultRaw> CallMulti(Process const& process, 
  std::vector<LPCVOID> const& addresses, 
  std::vector<CallConv> const& call_convs, 
  std::vector<std::vector<CallArg>> const& args_full) 
{
  BOOST_ASSERT(addresses.size() == call_convs.size() && 
    addresses.size() == args_full.size());

  Allocator const return_values_remote(&process, 
    sizeof(CallResultRemote) * addresses.size());

  Allocator const code_remote(GenerateCallCode(process, addresses, 
    call_convs, args_full, return_values_remote.GetBase()));
  LPTHREAD_START_ROUTINE code_remote_pfn = 
    reinterpret_cast<LPTHREAD_START_ROUTINE>(
    reinterpret_cast<DWORD_PTR>(code_remote.GetBase()));

  HANDLE const thread_remote = ::CreateRemoteThread(process.GetHandle(), 
    nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(code_remote_pfn), 
    nullptr, 0, nullptr);
  if (!thread_remote)
  {
    DWORD const last_error = ::GetLastError();
    BOOST_THROW_EXCEPTION(Error() << 
      ErrorString("Could not create remote thread.") << 
      ErrorCodeWinLast(last_error));
  }
  
  BOOST_SCOPE_EXIT_ALL(&)
  {
    // WARNING: Handle is leaked if CloseHandle fails.
    BOOST_VERIFY(::CloseHandle(thread_remote));
  };

  // TODO: Allow customizable timeout.
  if (::WaitForSingleObject(thread_remote, INFINITE) != WAIT_OBJECT_0)
  {
    DWORD const LastError = ::GetLastError();
    BOOST_THROW_EXCEPTION(Error() << 
      ErrorString("Could not wait for remote thread.") << 
      ErrorCodeWinLast(LastError));
  }

  std::vector<CallResultRemote> return_vals_remote = 
    ReadVector<CallResultRemote>(process, 
    return_values_remote.GetBase(), addresses.size());
  
  std::vector<CallResultRaw> return_vals;
  return_vals.reserve(addresses.size());
  
  std::transform(std::begin(return_vals_remote), std::end(return_vals_remote), 
    std::back_inserter(return_vals), 
    [] (CallResultRemote const& r)
    {
      return CallResultRaw(r.return_value, 
        r.return_value_32, 
        r.return_value_64, 
        r.return_value_float, 
        r.return_value_double, 
        r.last_error);
    });

  return return_vals;
}

MultiCall::MultiCall(Process const* process)
  : process_(process), 
  addresses_(), 
  call_convs_(), 
  args_()
{
  BOOST_ASSERT(process != nullptr);
}

MultiCall::MultiCall(MultiCall const& other)
  : process_(other.process_), 
  addresses_(other.addresses_), 
  call_convs_(other.call_convs_), 
  args_(other.args_)
{ }

MultiCall& MultiCall::operator=(MultiCall const& other)
{
  process_ = other.process_;
  addresses_ = other.addresses_;
  call_convs_ = other.call_convs_;
  args_ = other.args_;

  return *this;
}

MultiCall::MultiCall(MultiCall&& other) BOOST_NOEXCEPT
  : process_(other.process_), 
  addresses_(std::move(other.addresses_)), 
  call_convs_(std::move(other.call_convs_)), 
  args_(std::move(other.args_))
{
  other.process_ = nullptr;
}

MultiCall& MultiCall::operator=(MultiCall&& other) BOOST_NOEXCEPT
{
  process_ = other.process_;
  other.process_ = nullptr;

  addresses_ = std::move(other.addresses_);

  call_convs_ = std::move(other.call_convs_);

  args_ = std::move(other.args_);

  return *this;
}

MultiCall::~MultiCall()
{ }

std::vector<CallResultRaw> MultiCall::Call() const
{
  return CallMulti(*process_, addresses_, call_convs_, args_);
}

}
