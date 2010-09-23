HadesMem.WriteLn("Opening WinDbg.");
MyMem = HadesMem.MemoryMgr("windbg.exe");

MyModEnum = HadesMem.ModuleEnum(MyMem)
MyMod = MyModEnum:First();

while MyMod do
	MyPeFile = HadesMem.PeFile(MyMem, MyMod:GetBase());
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
	
	MyTlsDir = HadesMem.TlsDir(MyPeFile);
	if MyTlsDir:IsValid() then
		HadesMem.WriteLn("");
		HadesMem.WriteLn("Dumping TLS callbacks.");
		TlsCallbacks = MyTlsDir:GetCallbacks();
		for i in TlsCallbacks.List do
			HadesMem.WriteLn(HadesMem.ToHexStr(i));
		end
	else
		HadesMem.WriteLn("");
		HadesMem.WriteLn("Image has no TLS directory.");
	end
	
	MyExportDir = HadesMem.ExportDir(MyPeFile);
	if MyExportDir:IsValid() then
		HadesMem.WriteLn("");
		HadesMem.WriteLn("Export dir module name: " .. MyExportDir:GetName());
		
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
				if MyExport:Forwarded() then
					HadesMem.WriteLn(MyExport:GetOrdinal() .. " -> " .. MyExport:GetForwarder());
				else
					HadesMem.WriteLn(MyExport:GetOrdinal() .. "");
				end
			end
			MyExport = MyExportEnum:Next();
		end
	else
		HadesMem.WriteLn("");
		HadesMem.WriteLn("Image has no export directory.");
	end
	MyMod = MyModEnum:Next();
end
