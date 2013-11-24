// Copyright (C) 2010-2013 Joshua Boyce.
// See the file COPYING for copying permission.

#include <hadesmem/patcher.hpp>

#include <memory>
#include <sstream>
#include <string>
#include <utility>

#include <hadesmem/detail/warning_disable_prefix.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <hadesmem/detail/warning_disable_suffix.hpp>

#include <hadesmem/detail/warning_disable_prefix.hpp>
#include <asmjit/asmjit.h>
#include <hadesmem/detail/warning_disable_suffix.hpp>

#include <hadesmem/config.hpp>
#include <hadesmem/detail/make_unique.hpp>
#include <hadesmem/error.hpp>
#include <hadesmem/process.hpp>

// TODO: Patcher constructor tests
// TODO: Address tests.
// TODO: Stream overload tests.

// TODO: Fix the code so this hack can be removed.
#if defined(HADESMEM_CLANG)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#endif

extern std::unique_ptr<hadesmem::PatchDetour> g_detour;
std::unique_ptr<hadesmem::PatchDetour> g_detour;

#if defined(HADESMEM_CLANG)
#pragma GCC diagnostic pop
#endif

std::uint32_t HookMe(
    std::int32_t i1,
    std::int32_t i2,
    std::int32_t i3,
    std::int32_t i4,
    std::int32_t i5,
    std::int32_t i6,
    std::int32_t i7,
    std::int32_t i8)
{
    std::string const foo("Foo");
    BOOST_TEST_EQ(foo, "Foo");

    BOOST_TEST_EQ(i1, 1);
    BOOST_TEST_EQ(i2, 2);
    BOOST_TEST_EQ(i3, 3);
    BOOST_TEST_EQ(i4, 4);
    BOOST_TEST_EQ(i5, 5);
    BOOST_TEST_EQ(i6, 6);
    BOOST_TEST_EQ(i7, 7);
    BOOST_TEST_EQ(i8, 8);

    return 0x1234;
}

std::uint32_t HookMeHk(
    std::int32_t i1,
    std::int32_t i2,
    std::int32_t i3,
    std::int32_t i4,
    std::int32_t i5,
    std::int32_t i6,
    std::int32_t i7,
    std::int32_t i8)
{
    BOOST_TEST(g_detour->GetTrampoline() != nullptr);
    auto const orig = g_detour->GetTrampoline<decltype(&HookMe)>();
    BOOST_TEST_EQ(orig(i1, i2, i3, i4, i5, i6, i7, i8), 0x1234UL);
    return 0x1337;
}

void TestPatchRaw()
{
    hadesmem::Process const process(::GetCurrentProcessId());

    hadesmem::Allocator const test_mem(process, 0x1000);

    std::vector<BYTE> data = { 0x00, 0x11, 0x22, 0x33, 0x44 };
    BOOST_TEST_EQ(data.size(), 5);

    hadesmem::PatchRaw patch(process, test_mem.GetBase(), data);

    auto const orig = hadesmem::ReadVector<BYTE>(
        process,
        test_mem.GetBase(),
        5);

    patch.Apply();

    auto const apply = hadesmem::ReadVector<BYTE>(
        process,
        test_mem.GetBase(),
        5);

    patch.Remove();

    auto const remove = hadesmem::ReadVector<BYTE>(
        process,
        test_mem.GetBase(),
        5);

    BOOST_TEST(orig == remove);
    BOOST_TEST(orig != apply);
    BOOST_TEST(data == apply);
}

void TestPatchDetour()
{
    typedef AsmJit::FuncBuilder8<
        std::uint32_t,
        std::int32_t,
        std::int32_t,
        std::int32_t,
        std::int32_t,
        std::int32_t,
        std::int32_t,
        std::int32_t,
        std::int32_t> HookMeFuncBuilderT;

    // TODO: Generate different kinds of code to test all instruction 
    // resolution code and ensure we're covering all the important cases.

    // TODO: Test different calling conventions etc.

    AsmJit::X86Compiler c;
    c.newFunc(AsmJit::kX86FuncConvDefault, HookMeFuncBuilderT());

    c.nop();
    c.nop();
    c.nop();
    c.nop();
    c.nop();
    c.nop();
    c.nop();
    c.nop();
    c.nop();
    c.nop();
    c.nop();

    AsmJit::GpVar a1(c.getGpArg(0));
    AsmJit::GpVar a2(c.getGpArg(1));
    AsmJit::GpVar a3(c.getGpArg(2));
    AsmJit::GpVar a4(c.getGpArg(3));
    AsmJit::GpVar a5(c.getGpArg(4));
    AsmJit::GpVar a6(c.getGpArg(5));
    AsmJit::GpVar a7(c.getGpArg(6));
    AsmJit::GpVar a8(c.getGpArg(7));

    AsmJit::GpVar address(c.newGpVar());
    c.mov(address, AsmJit::imm(reinterpret_cast<sysint_t>(&HookMe)));

    AsmJit::GpVar var(c.newGpVar());
    AsmJit::X86CompilerFuncCall* ctx = c.call(address);
    ctx->setPrototype(AsmJit::kX86FuncConvDefault, HookMeFuncBuilderT());
    ctx->setArgument(0, a1);
    ctx->setArgument(1, a2);
    ctx->setArgument(2, a3);
    ctx->setArgument(3, a4);
    ctx->setArgument(4, a5);
    ctx->setArgument(5, a6);
    ctx->setArgument(6, a7);
    ctx->setArgument(7, a8);
    ctx->setReturn(var);

    c.ret(var);

    c.endFunc();

    auto const free_asmjit_func = [](void* func)
    {
        AsmJit::MemoryManager::getGlobal()->free(func);
    };
    void* hook_me_wrapper_raw = c.make();
    std::unique_ptr<void, decltype(free_asmjit_func)> hook_me_wrapper_cleanup(
        hook_me_wrapper_raw,
        free_asmjit_func);

    auto const hook_me_wrapper = reinterpret_cast<decltype(&HookMe)>(
        reinterpret_cast<DWORD_PTR>(hook_me_wrapper_raw));

    auto const hook_me_packaged =
        [&]()
    {
        return hook_me_wrapper(1, 2, 3, 4, 5, 6, 7, 8);
    };
    BOOST_TEST_EQ(hook_me_packaged(), 0x1234UL);

    hadesmem::Process const process(::GetCurrentProcessId());

    DWORD_PTR const target_ptr = reinterpret_cast<DWORD_PTR>(hook_me_wrapper);
    DWORD_PTR const detour_ptr = reinterpret_cast<DWORD_PTR>(&HookMeHk);
    g_detour = hadesmem::detail::make_unique<hadesmem::PatchDetour>(
        process,
        reinterpret_cast<PVOID>(target_ptr),
        reinterpret_cast<PVOID>(detour_ptr));

    BOOST_TEST_EQ(hook_me_packaged(), 0x1234UL);

    g_detour->Apply();

    BOOST_TEST_EQ(hook_me_packaged(), 0x1337UL);

    g_detour->Remove();

    BOOST_TEST_EQ(hook_me_packaged(), 0x1234UL);

    g_detour->Apply();

    BOOST_TEST_EQ(hook_me_packaged(), 0x1337UL);

    g_detour->Remove();

    BOOST_TEST_EQ(hook_me_packaged(), 0x1234UL);
}

int main()
{
    TestPatchRaw();
    TestPatchDetour();
    return boost::report_errors();
}
