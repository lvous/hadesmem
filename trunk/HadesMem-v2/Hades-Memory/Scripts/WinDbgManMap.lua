HadesMem.WriteLn("Opening WinDbg.");
MyMem = HadesMem.MemoryMgr("windbg.exe");

if HadesMem.IsAMD64() then
	HadesMem.WriteLn("");
	HadesMem.WriteLn("Manually mapping Hades-MMHelper_AMD64");
	MyManMap = HadesMem.ManualMap(MyMem);
	ManModBase = MyManMap:Map("../Hades-MMHelper_AMD64.dll", "Test", false);
	HadesMem.WriteLn("Success! Module Base: " .. HadesMem.ToHexStr(ManModBase));	
else
	HadesMem.WriteLn("");
	HadesMem.WriteLn("Manually mapping Hades-MMHelper_IA32");
	MyManMap = HadesMem.ManualMap(MyMem);
	ManModBase = MyManMap:Map("../Hades-MMHelper_IA32.dll", "_Test@4", false);
	HadesMem.WriteLn("Success! Module Base: " .. HadesMem.ToHexStr(ManModBase));
end
