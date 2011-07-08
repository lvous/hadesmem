# This file is part of HadesMem.
# Copyright (C) 2010 Joshua Boyce (aka RaptorFactor, Cypherjb, Cypher, Chazwazza).
# <http://www.raptorfactor.com/> <raptorfactor@raptorfactor.com>
# 
# HadesMem is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# HadesMem is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with HadesMem.  If not, see <http://www.gnu.org/licenses/>.

ProcName = raw_input("Process name: ")
MyMem = PyHadesMem.MemoryMgr(ProcName)
MyManMap = PyHadesMem.ManualMap(MyMem)
ModPath = raw_input("Module path: ")
ExpName = raw_input("Export name (optional): ")
PathRes = int(raw_input("Path resolution (1/0): "))
print("Manual mapping DLL")
ModRemote = MyManMap.Map(ModPath, ExpName, PathRes)
print("Module Base: " + hex(ModRemote))