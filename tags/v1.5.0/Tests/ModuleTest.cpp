// Copyright Joshua Boyce 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// This file is part of HadesMem.
// <http://www.raptorfactor.com/> <raptorfactor@raptorfactor.com>

// Hades
#include <HadesMemory/Module.hpp>
#include <HadesMemory/MemoryMgr.hpp>

// C++ Standard Library
#include <algorithm>

// Boost
#define BOOST_TEST_MODULE ModuleTest
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(ConstructorsTest)
{
  // Create memory manager for self
  HadesMem::MemoryMgr MyMemory(GetCurrentProcessId());
      
  // Open module by handle
  HadesMem::Module SelfModule(MyMemory, NULL);
  BOOST_CHECK_EQUAL(SelfModule.GetHandle(), GetModuleHandle(NULL));
  HadesMem::Module NewSelfModule(MyMemory, SelfModule.GetHandle());
  BOOST_CHECK(SelfModule == NewSelfModule);
      
  // Open module by name
  HadesMem::Module K32Module(MyMemory, L"kernel32.dll");
  BOOST_CHECK_EQUAL(K32Module.GetHandle(), GetModuleHandle(L"kernel32.dll"));
  HadesMem::Module NewK32Module(MyMemory, K32Module.GetName());
  BOOST_CHECK(K32Module == NewK32Module);
  HadesMem::Module NewNewK32Module(MyMemory, K32Module.GetPath());
  BOOST_CHECK(K32Module == NewNewK32Module);
  
  // Test inequality
  BOOST_CHECK(SelfModule != K32Module);
  
  // Test module handle failure
  BOOST_CHECK_THROW(HadesMem::Module InvalidModuleHandle(MyMemory, 
    reinterpret_cast<HMODULE>(-1)), HadesMem::HadesMemError);
  
  // Test module name failure
  BOOST_CHECK_THROW(HadesMem::Module InvalidModuleName(MyMemory, 
    L"InvalidModuleXYZQQ.dll"), HadesMem::HadesMemError);
      
  // Test copying, assignement, and moving
  HadesMem::Module OtherSelfModule(SelfModule);
  BOOST_CHECK(OtherSelfModule == SelfModule);
  SelfModule = OtherSelfModule;
  BOOST_CHECK(OtherSelfModule == SelfModule);
  HadesMem::Module MovedSelfModule(std::move(OtherSelfModule));
  BOOST_CHECK(MovedSelfModule == SelfModule);
  SelfModule = std::move(MovedSelfModule);
  BOOST_CHECK_EQUAL(SelfModule.GetHandle(), GetModuleHandle(NULL));
}

BOOST_AUTO_TEST_CASE(DataTest)
{
  // Create memory manager for self
  HadesMem::MemoryMgr MyMemory(GetCurrentProcessId());
      
  // Open module by name
  HadesMem::Module K32Module(MyMemory, L"kernel32.dll");
  
  // Test GetHandle
  BOOST_CHECK_EQUAL(K32Module.GetHandle(), GetModuleHandle(L"kernel32.dll"));
  
  // Test GetSize
  BOOST_CHECK(K32Module.GetSize() != 0);
  
  // Test GetName
  BOOST_CHECK(K32Module.GetName() == L"kernel32.dll");
  
  // Test GetPath
  BOOST_CHECK(!K32Module.GetPath().empty());
  BOOST_CHECK(K32Module.GetPath().find(L"kernel32.dll") != std::wstring::npos);
}

BOOST_AUTO_TEST_CASE(ProcedureTest)
{
  // Create memory manager for self
  HadesMem::MemoryMgr MyMemory(GetCurrentProcessId());
      
  // Open NTDLL module by name
  HadesMem::Module NtdllMod(MyMemory, L"ntdll.dll");
  
  // Find NtQueryInformationProcess
  FARPROC pQueryInfo = NtdllMod.FindProcedure(
    "NtQueryInformationProcess");
  BOOST_CHECK_EQUAL(pQueryInfo, GetProcAddress(GetModuleHandle(
    L"ntdll.dll"), "NtQueryInformationProcess"));
    
  // Find ordinal 0
  FARPROC pOrdinal0 = NtdllMod.FindProcedure(1);
  BOOST_CHECK_EQUAL(pOrdinal0, GetProcAddress(GetModuleHandle(
    L"ntdll.dll"), MAKEINTRESOURCEA(1)));
}

BOOST_AUTO_TEST_CASE(IteratorTest)
{
  // Create memory manager for self
  HadesMem::MemoryMgr MyMemory(GetCurrentProcessId());
  
  // Test non-const module iterator
  HadesMem::ModuleList Modules(MyMemory);
  BOOST_CHECK(Modules.begin() != Modules.end());
  std::for_each(Modules.begin(), Modules.end(), 
    [&] (HadesMem::Module& M)
    {
      // Ensure module APIs execute without exception and return valid data
      BOOST_CHECK(M.GetHandle() != 0);
      BOOST_CHECK(M.GetSize() != 0);
      BOOST_CHECK(!M.GetName().empty());
      BOOST_CHECK(!M.GetPath().empty());
      
      // Ensure GetRemoteModuleHandle works as expected
      // Note: The module name check could possibly fail if multiple modules 
      // with the same name but a different path are loaded in the process, 
      // but this is currently not the case with any of the testing binaries.
      BOOST_CHECK(M == HadesMem::GetRemoteModule(MyMemory, M.GetName().c_str()));
      BOOST_CHECK(M == HadesMem::GetRemoteModule(MyMemory, M.GetPath().c_str()));
    });
  
  // Test implicit const module iteratator
  HadesMem::ModuleList const ModulesImpC(MyMemory);
  BOOST_CHECK(ModulesImpC.begin() != ModulesImpC.end());
  std::for_each(ModulesImpC.begin(), ModulesImpC.end(), 
    [&] (HadesMem::Module const& M)
    {
      // Ensure module APIs execute without exception and return valid data
      BOOST_CHECK(M.GetHandle() != 0);
      BOOST_CHECK(M.GetSize() != 0);
      BOOST_CHECK(!M.GetName().empty());
      BOOST_CHECK(!M.GetPath().empty());
      
      // Ensure GetRemoteModuleHandle works as expected
      // Note: The module name check could possibly fail if multiple modules 
      // with the same name but a different path are loaded in the process, 
      // but this is currently not the case with any of the testing binaries.
      BOOST_CHECK(M == HadesMem::GetRemoteModule(MyMemory, M.GetName().c_str()));
      BOOST_CHECK(M == HadesMem::GetRemoteModule(MyMemory, M.GetPath().c_str()));
    });
  
  // Test explicit const module iteratator
  HadesMem::ModuleList ModulesExpC(MyMemory);
  BOOST_CHECK(ModulesExpC.cbegin() != ModulesExpC.cend());
  std::for_each(ModulesExpC.cbegin(), ModulesExpC.cend(), 
    [&] (HadesMem::Module const& M)
    {
      // Ensure module APIs execute without exception and return valid data
      BOOST_CHECK(M.GetHandle() != 0);
      BOOST_CHECK(M.GetSize() != 0);
      BOOST_CHECK(!M.GetName().empty());
      BOOST_CHECK(!M.GetPath().empty());
      
      // Ensure GetRemoteModuleHandle works as expected
      // Note: The module name check could possibly fail if multiple modules 
      // with the same name but a different path are loaded in the process, 
      // but this is currently not the case with any of the testing binaries.
      BOOST_CHECK(M == HadesMem::GetRemoteModule(MyMemory, M.GetName().c_str()));
      BOOST_CHECK(M == HadesMem::GetRemoteModule(MyMemory, M.GetPath().c_str()));
    });
    
  // Check known module is found
  HadesMem::ModuleList ModulesK32Check(MyMemory);
  auto Iter1 = std::find_if(ModulesK32Check.cbegin(), ModulesK32Check.cend(), 
    [&] (HadesMem::Module const& M)
    {
      return M.GetName() == L"kernel32.dll";
    });
  BOOST_CHECK(Iter1 != ModulesK32Check.cend());
  // Check again with different criteria, testing multipass
  auto Iter2 = std::find_if(ModulesK32Check.cbegin(), ModulesK32Check.cend(), 
    [&] (HadesMem::Module const& M)
    {
      return M.GetHandle() == GetModuleHandleW(L"kernel32.dll");
    });
  BOOST_CHECK(Iter2 != ModulesK32Check.cend());
}
