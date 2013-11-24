// Copyright (C) 2010-2013 Joshua Boyce.
// See the file COPYING for copying permission.

#include <hadesmem/find_pattern.hpp>

#include <hadesmem/detail/warning_disable_prefix.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <hadesmem/detail/warning_disable_suffix.hpp>

#include <hadesmem/config.hpp>
#include <hadesmem/error.hpp>
#include <hadesmem/process.hpp>

// TODO: Clean up, expand, fix, etc these tests.
// TODO: Add more tests (e.g. stream overload tests).

void TestFindPattern()
{
    hadesmem::Process const process(::GetCurrentProcessId());

    HMODULE const self = GetModuleHandle(nullptr);
    BOOST_TEST_NE(self, static_cast<void*>(nullptr));

    hadesmem::FindPattern find_pattern(process, self);

    find_pattern = hadesmem::FindPattern(process, nullptr);
    // Ensure constructor throws if an invalid module handle is specified
    // TODO: Fix this.
    //BOOST_CHECK_THROW(find_pattern = hadesmem::FindPattern(process, 
    //  reinterpret_cast<HMODULE>(-1)), hadesmem::Error);

    // Scan for predicatable byte mask
    auto const nop = find_pattern.Find(
        L"90", 
        hadesmem::FindPatternFlags::kNone);
    BOOST_TEST_NE(nop, static_cast<void*>(nullptr));
    BOOST_TEST(nop > self);
    find_pattern.Find(L"90", L"Nop", hadesmem::FindPatternFlags::kNone);
    BOOST_TEST_EQ(nop, find_pattern[L"Nop"]);
    BOOST_TEST_EQ(find_pattern.GetAddresses().size(), 1UL);
    auto const nop_rel = find_pattern.Find(
        L"90",
        hadesmem::FindPatternFlags::kRelativeAddress);
    BOOST_TEST_EQ(static_cast<PBYTE>(nop_rel)+
        reinterpret_cast<DWORD_PTR>(self), nop);
    hadesmem::Pattern nop_pattern(
        find_pattern, 
        L"90", 
        L"NopPlus1",
        hadesmem::FindPatternFlags::kNone);
    Add(nop_pattern, 1);
    Save(find_pattern, nop_pattern);
    BOOST_TEST_EQ(nop_pattern.GetAddress(), static_cast<PBYTE>(nop)+1);
    BOOST_TEST_EQ(nop_pattern.GetAddress(), find_pattern[L"NopPlus1"]);
    BOOST_TEST_EQ(find_pattern.GetAddresses().size(), 2UL);

    auto const zeros = find_pattern.Find(
        L"00 ?? 00",
        hadesmem::FindPatternFlags::kNone);
    BOOST_TEST_NE(zeros, static_cast<void*>(nullptr));
    BOOST_TEST(zeros > ::GetModuleHandle(nullptr));
    find_pattern.Find(
        L"00 ?? 00", 
        L"Zeros",
        hadesmem::FindPatternFlags::kNone);
    BOOST_TEST_EQ(zeros, find_pattern[L"Zeros"]);
    BOOST_TEST_EQ(find_pattern.GetAddresses().size(), 3UL);
    BOOST_TEST_NE(nop, zeros);
    auto const zeros_rel = find_pattern.Find(
        L"00 ?? 00",
        hadesmem::FindPatternFlags::kRelativeAddress);
    BOOST_TEST_EQ(static_cast<PBYTE>(zeros_rel)+
        reinterpret_cast<DWORD_PTR>(self), zeros);
    hadesmem::Pattern zeros_pattern(
        find_pattern, 
        L"00 ?? 00",
        L"ZerosMinus1", 
        hadesmem::FindPatternFlags::kNone);
    Sub(zeros_pattern, 1);
    Save(find_pattern, zeros_pattern);
    BOOST_TEST_EQ(static_cast<PVOID>(zeros_pattern.GetAddress()),
        static_cast<PVOID>(static_cast<PBYTE>(zeros)-1));
    BOOST_TEST_EQ(zeros_pattern.GetAddress(),
        find_pattern[L"ZerosMinus1"]);
    BOOST_TEST_EQ(find_pattern.GetAddresses().size(), 4UL);

    // Test stream-based pattern scanner by scanning for an instruction with 
    // a known relative operand. We should ensure that we're not going to hit 
    // this particular byte accidently as part of the operand of another 
    // instruction, but for now lets just ignore that possibility. The test 
    // will (or should) fail in that case.
    hadesmem::Pattern call_pattern(find_pattern, L"E8",
        hadesmem::FindPatternFlags::kRelativeAddress);
    BOOST_TEST_NE(call_pattern.GetAddress(), static_cast<void*>(nullptr));
    Add(call_pattern, 1);
    Rel(call_pattern, 5, 1);
    Save(find_pattern, call_pattern);
    BOOST_TEST_NE(call_pattern.GetAddress(), static_cast<void*>(nullptr));

    // Todo: pattern_manipulators::Lea test

    std::wstring const pattern_file_data =
        LR"(
<?xml version="1.0" encoding="utf-8"?>
<HadesMem>
  <FindPattern>
    <Flag Name="RelativeAddress"/>
    <Flag Name="ThrowOnUnmatch"/>
    <Pattern Name="First Call" Data="E8">
      <Manipulator Name="Add" Operand1="1"/>
      <Manipulator Name="Rel" Operand1="5" Operand2="1"/>
    </Pattern>
    <Pattern Name="Zeros New" Data="00 ?? 00">
      <Manipulator Name="Add" Operand1="1"/>
      <Manipulator Name="Sub" Operand1="1"/>
    </Pattern>
    <Pattern Name="Nop Other" Data="90"/>
    <Pattern Name="Nop Second" Data="90" Start="Nop Other"/>
    <Pattern Name="FindPattern String" Data="46 69 6E 64 50 61 74 74 65 72 6E">
      <Flag Name="ScanData"/>
    </Pattern>
  </FindPattern>
</HadesMem>
)";
    find_pattern.LoadFileMemory(pattern_file_data);
    BOOST_TEST_EQ(find_pattern.Lookup(L"First Call"),
        call_pattern.GetAddress());
    BOOST_TEST_EQ(find_pattern.Lookup(L"First Call"),
        find_pattern[L"First Call"]);
    BOOST_TEST_EQ(find_pattern.Lookup(L"Zeros New"),
        zeros_rel);
    BOOST_TEST_EQ(find_pattern.Lookup(L"Zeros New"),
        find_pattern[L"Zeros New"]);
    BOOST_TEST_EQ(find_pattern.Lookup(L"Nop Other"),
        nop_rel);
    BOOST_TEST_EQ(find_pattern.Lookup(L"Nop Other"),
        find_pattern[L"Nop Other"]);
    BOOST_TEST_EQ(find_pattern.Lookup(L"Nop Second"),
        find_pattern[L"Nop Second"]);
    BOOST_TEST(find_pattern[L"Nop Second"] > find_pattern[L"Nop Other"]);
    BOOST_TEST_EQ(find_pattern.Lookup(L"FindPattern String"),
        find_pattern[L"FindPattern String"]);

    // TODO: Fix the test to ensure we get the error we're expecting, rather 
    // than just any error.
    std::wstring const pattern_file_data_invalid1 =
        LR"(
<?xml version="1.0" encoding="utf-8"?>
<HadesMem>
  <FindPattern>
    <Flag Name="InvalidFlag"/>
  </FindPattern>
</HadesMem>
)";
    BOOST_TEST_THROWS(find_pattern.LoadFileMemory(
        pattern_file_data_invalid1),
        hadesmem::Error);

    std::wstring const pattern_file_data_invalid2 =
        LR"(
<?xml version="1.0" encoding="utf-8"?>
<HadesMem>
  <FindPattern>
    <Flag Name="RelativeAddress"/>
    <Flag Name="ThrowOnUnmatch"/>
    <Pattern Name="Foo2" Data="ZZ"/>
  </FindPattern>
</HadesMem>
)";
    BOOST_TEST_THROWS(find_pattern.LoadFileMemory(
        pattern_file_data_invalid2),
        hadesmem::Error);

    std::wstring const pattern_file_data_invalid3 =
        LR"(
<?xml version="1.0" encoding="utf-8"?>
<HadesMem>
  <FindPattern>
    <Flag Name="RelativeAddress"/>
    <Flag Name="ThrowOnUnmatch"/>
    <Pattern/>
  </FindPattern>
</HadesMem>
)";
    BOOST_TEST_THROWS(find_pattern.LoadFileMemory(
        pattern_file_data_invalid3),
        hadesmem::Error);

    std::wstring const pattern_file_data_invalid4 =
        LR"(
<?xml version="1.0" encoding="utf-8"?>
<HadesMem>
  <FindPattern>
    <Flag Name="RelativeAddress"/>
    <Flag Name="ThrowOnUnmatch"/>
    <Pattern Name="Foo4" Data="90" Start="DoesNotExist"/>
  </FindPattern>
</HadesMem>
)";
    BOOST_TEST_THROWS(find_pattern.LoadFileMemory(
        pattern_file_data_invalid4),
        hadesmem::Error);

    // Todo: LoadFile test

    auto const nops_any = find_pattern.Find(
        L"?? ?? ?? ?? ??",
        hadesmem::FindPatternFlags::kNone);
    auto const int3s_any = find_pattern.Find(
        L"?? ?? ?? ?? ??",
        hadesmem::FindPatternFlags::kNone);
    BOOST_TEST_EQ(nops_any, int3s_any);
    BOOST_TEST(nops_any > ::GetModuleHandle(nullptr));
    auto const nops_any_rel = find_pattern.Find(
        L"?? ?? ?? ?? ??",
        hadesmem::FindPatternFlags::kRelativeAddress);
    BOOST_TEST_EQ(static_cast<PBYTE>(nops_any_rel)+
        reinterpret_cast<DWORD_PTR>(self), nops_any);

    BOOST_TEST_THROWS(find_pattern.Find(
        L"AA BB CC DD EE FF 11 22 33 44 55 66 77 88 99 00 11 33 33 77",
        hadesmem::FindPatternFlags::kThrowOnUnmatch),
        hadesmem::Error);

    // Pattern is for narrow string 'FindPattern' (without quotes)
    auto const find_pattern_str = find_pattern.Find(
        L"46 69 6E 64 50 61 74 74 65 72 6E", 
        hadesmem::FindPatternFlags::kScanData |
        hadesmem::FindPatternFlags::kRelativeAddress);
    BOOST_TEST_NE(find_pattern_str, static_cast<void*>(nullptr));
    BOOST_TEST_EQ(find_pattern_str, find_pattern[L"FindPattern String"]);

    auto const nop_second = find_pattern.Find(L"90",
        hadesmem::FindPatternFlags::kRelativeAddress |
        hadesmem::FindPatternFlags::kThrowOnUnmatch,
        L"Nop Other");
    BOOST_TEST_NE(nop_second, static_cast<void*>(nullptr));
    BOOST_TEST_EQ(nop_second, find_pattern[L"Nop Second"]);
    BOOST_TEST_THROWS(find_pattern.Find(L"90",
        hadesmem::FindPatternFlags::kRelativeAddress |
        hadesmem::FindPatternFlags::kThrowOnUnmatch |
        hadesmem::FindPatternFlags::kScanData,
        L"Nop Other"), hadesmem::Error);
    BOOST_TEST_NE(nop_second, static_cast<void*>(nullptr));
    BOOST_TEST_EQ(nop_second, find_pattern[L"Nop Second"]);

    BOOST_TEST_THROWS(find_pattern.Find(L"ZZ",
        hadesmem::FindPatternFlags::kNone), hadesmem::Error);
}

int main()
{
    TestFindPattern();
    return boost::report_errors();
}
