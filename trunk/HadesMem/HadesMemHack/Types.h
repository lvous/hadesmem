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
