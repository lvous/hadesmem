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
    Task_FreeMem
  };

  // Declare fixed-size types
  typedef char                  Byte;
  typedef short                 Int16;
  typedef unsigned short        UInt16;
  typedef int                   Int32;
  typedef unsigned int          UInt32;
  typedef long long             Int64;
  typedef unsigned long long    UInt64;
  typedef float                 Float;
  typedef double                Double;
  typedef std::string           StrNarrow;
  typedef std::wstring          StrWide;
  typedef char                  CharNarrow;
  typedef wchar_t               CharWide;
  typedef void*                 Pointer;

  // Ensure data type are correct
  static_assert(sizeof(char) == 1, "Size of Byte is wrong.");
  static_assert(sizeof(Int16) == 2, "Size of Int16 is wrong.");
  static_assert(sizeof(UInt16) == 2, "Size of UInt16 is wrong.");
  static_assert(sizeof(Int32) == 4, "Size of Int32 is wrong.");
  static_assert(sizeof(UInt32) == 4, "Size of UInt32 is wrong.");
  static_assert(sizeof(Int64) == 8, "Size of Int64 is wrong.");
  static_assert(sizeof(UInt64) == 8, "Size of ""UInt64 is wrong.");
  static_assert(sizeof(Float) == 4, "Size of Float is wrong.");
  static_assert(sizeof(Double) == 8, "Size of Double is wrong.");
}
