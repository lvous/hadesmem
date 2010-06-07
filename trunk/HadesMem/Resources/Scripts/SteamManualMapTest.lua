MyMem = HadesMem.CreateMemoryMgr("Steam.exe");

HadesMem.WriteLn("Process ID: " .. MyMem:GetProcessID());

MyManualMap = HadesMem.CreateManualMap(MyMem);

HadesMem.WriteLn("");
HadesMem.WriteLn("Manually Mapping Hades-MMHelper.")
if HadesMem.IsAMD64() then
	HadesMem.WriteLn("Hades-MMHelper Base: " .. HadesMem.ToHexStr(MyManualMap:Map("Hades-MMHelper_AMD64.dll", "Initialize", false)));
else
	HadesMem.WriteLn("Hades-MMHelper Base: " .. HadesMem.ToHexStr(MyManualMap:Map("Hades-MMHelper_IA32.dll", "_Initialize@4", false)));
end
