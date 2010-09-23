HadesMem.WriteLn("Opening WinDbg.");
MyMem = HadesMem.MemoryMgr("windbg.exe");

Kernel32Mod = HadesMem.Module(MyMem, "kernel32.dll");
MyPeFile = HadesMem.PeFile(MyMem, Kernel32Mod:GetBase());
MyDosHeader = HadesMem.DosHeader(MyPeFile);
MyNtHeaders = HadesMem.NtHeaders(MyPeFile);

HadesMem.WriteLn("");
HadesMem.WriteLn("Dumping sections.");
MySectionEnum = HadesMem.SectionEnum(MyPeFile);
MySection = MySectionEnum:First();
while MySection do
	HadesMem.WriteLn(MySection:GetName());
	MySection = MySectionEnum:Next();
end

MyExportDir = HadesMem.ExportDir(MyPeFile);

HadesMem.WriteLn("");
HadesMem.WriteLn("Kernel32 export module name: " .. MyExportDir:GetName());

HadesMem.WriteLn("");
HadesMem.WriteLn("Dumping exports.");
MyExportEnum = HadesMem.ExportEnum(MyPeFile);
MyExport = MyExportEnum:First();
while MyExport do
	if MyExport:ByName() then
		if MyExport:Forwarded() then
			HadesMem.WriteLn(MyExport:GetOrdinal() .. " -> " .. MyExport:GetName() .. " -> " .. MyExport:GetForwarder());
		else
			HadesMem.WriteLn(MyExport:GetOrdinal() .. " -> " .. MyExport:GetName());
		end
	else
		HadesMem.WriteLn(MyExport:GetOrdinal());
	end
	MyExport = MyExportEnum:Next();
end
