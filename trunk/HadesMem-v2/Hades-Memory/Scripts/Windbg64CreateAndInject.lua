HadesMem.WriteLn("Creating Windbg (x64) and injecting Hades-MMHelper_AMD64");
MyCreateInfo64 = HadesMem.CreateAndInject("C:/Program Files/Debugging Tools for Windows (x64)/Windbg.exe", "", "Hades-MMHelper_AMD64.dll", "Initialize");
HadesMem.WriteLn("Success! Module Base: " .. HadesMem.ToHexStr(MyCreateInfo64.ModBase) .. ", Export Ret: " .. MyCreateInfo64.ExportRet);
