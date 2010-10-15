HadesMem.WriteLn("Opening WinDbg.");
MyMem = HadesMem.MemoryMgr("windbg.exe");

MyInjector = HadesMem.Injector(MyMem);

HadesMem.WriteLn("");
HadesMem.WriteLn("Injecting patcher test module.");
	
if HadesMem.IsAMD64() then
	TestModBase = MyInjector:InjectDll("Hades-MMHelper_AMD64.dll", true);
	ExportRet = MyInjector:CallExport("Hades-MMHelper_AMD64.dll", TestModBase, "TestPatcher");
else
	TestModBase = MyInjector:InjectDll("Hades-MMHelper_IA32.dll", true);
	ExportRet = MyInjector:CallExport("Hades-MMHelper_IA32.dll", TestModBase, "_TestPatcher@4");
end

HadesMem.WriteLn("Success! Module Base: " .. HadesMem.ToHexStr(TestModBase) .. ", Export Ret: " .. ExportRet);
