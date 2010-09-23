if HadesMem.IsAMD64() then
	HadesMem.WriteLn("Creating Windbg (x64) and injecting Hades-MMHelper_AMD64");
	MyCreateInfo64 = HadesMem.CreateAndInject("C:/Program Files/Debugging Tools for Windows (x64)/Windbg.exe", "", "Hades-MMHelper_AMD64.dll", "Initialize");
	HadesMem.WriteLn("Success! Module Base: " .. HadesMem.ToHexStr(MyCreateInfo64.ModBase) .. ", Export Ret: " .. MyCreateInfo64.ExportRet);
else
	HadesMem.WriteLn("Creating Windbg (x86) and injecting Hades-MMHelper_IA32");
	MyCreateInfo32 = HadesMem.CreateAndInject("C:/Program Files (x86)/Debugging Tools for Windows (x86)/Windbg.exe", "", "Hades-MMHelper_IA32.dll", "_Initialize@4");
	HadesMem.WriteLn("Success! Module Base: " .. HadesMem.ToHexStr(MyCreateInfo32.ModBase) .. ", Export Ret: " .. MyCreateInfo32.ExportRet);
end
