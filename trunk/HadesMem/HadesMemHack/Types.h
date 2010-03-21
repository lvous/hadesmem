/*
This file is part of HadesMem.
Copyright © 2010 Cypherjb (aka Chazwazza, aka Cypher). 
<http://www.cypherjb.com/> <cypher.jb@gmail.com>

HadesMem is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

HadesMem is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

// C++ Standard Library
#include <string>

namespace Detail
{
  // Types
  enum DataType
  {
    DataType_Byte = 1, 
    DataType_Int16, 
    DataType_UInt16, 
    DataType_Int32, 
    DataType_UInt32, 
    DataType_Int64, 
    DataType_UInt64, 
    DataType_Float, 
    DataType_Double, 
    DataType_StrNarrow, 
    DataType_StrWide, 
    DataType_CharNarrow, 
    DataType_CharWide, 
    DataType_Pointer, 
    DataType_Unknown 
  };

  // Process selection methods
  enum ProcSelect
  {
    ProcSelect_ProcName = 1, 
    ProcSelect_ProcID, 
    ProcSelect_WindowName
  };

  // Tasks
  enum Task
  {
    Task_ReadMem = 1, 
    Task_WriteMem, 
    Task_FindMod, 
    Task_CallFunc, 
    Task_AllocMem, 
    Task_FreeMem, 
    Task_Disassemble, 
    Task_PatternScan, 
    Task_InjectDLL, 
    Task_ManualMap, 
    Task_SearchMem, 
    Task_QueryMem, 
    Task_RunScript
  };
  
  // Module selection methods
  enum ModSelect
  {
    ModSelect_Name = 1, 
    ModSelect_Base, 
    ModSelect_List
  };
}
