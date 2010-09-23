HadesMem.WriteLn("Opening WinDbg.");
MyMem = HadesMem.MemoryMgr("windbg.exe");

HadesMem.WriteLn("");
HadesMem.WriteLn("Dumping module list.");
MyModEnum = HadesMem.ModuleEnum(MyMem);
MyMod = MyModEnum:First();
while MyMod do
	HadesMem.WriteLn("Base: " .. HadesMem.ToHexStr(MyMod:GetBase()) .. ", Size: " .. HadesMem.ToHexStr(MyMod:GetSize()) .. ", Name: " .. MyMod:GetName() .. ", Path: " .. MyMod:GetPath());
	MyMod = MyModEnum:Next();
end
