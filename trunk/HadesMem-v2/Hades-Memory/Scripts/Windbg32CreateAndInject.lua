HadesMem.WriteLn("Creating Windbg (x86) and injecting Hades-MMHelper_IA32");
MyCreateInfo32 = HadesMem.CreateAndInject("C:/Program Files (x86)/Debugging Tools for Windows (x86)/Windbg.exe", "", "Hades-MMHelper_IA32.dll", "_Initialize@4");
HadesMem.WriteLn("Success! Module Base: " .. HadesMem.ToHexStr(MyCreateInfo32.ModBase) .. ", Export Ret: " .. MyCreateInfo32.ExportRet);
