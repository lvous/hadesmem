ProcName = raw_input("Process name: ")
MyMem = PyHadesMem.MemoryMgr(ProcName)
MyInjector = PyHadesMem.Injector(MyMem)
ModPath = raw_input("Module path: ")
PathRes = int(raw_input("Path resolution (1/0): "))
print("Injecting DLL")
ModRemote = MyInjector.InjectDll(ModPath, PathRes)
print("Module Base: " + hex(ModRemote))
ExpName = raw_input("Export name (optional): ")
if ExpName:
  print("Calling export")
  ExpRet = MyInjector.CallExport(ModPath, ModRemote, ExpName)
  print("Export Ret: " + hex(ExpRet) + " " + str(ExpRet))
