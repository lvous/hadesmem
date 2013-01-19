// Copyright (C) 2010-2013 Joshua Boyce.
// See the file COPYING for copying permission.

#pragma once

#include <iosfwd>
#include <memory>
#include <string>

#include <windows.h>

#include "hadesmem/config.hpp"

namespace hadesmem
{

class Process;

class PeFile;

class ImportDir
{
public:
  explicit ImportDir(Process const& process, PeFile const& pe_file, 
    PIMAGE_IMPORT_DESCRIPTOR imp_desc);

  ImportDir(ImportDir const& other);
  
  ImportDir& operator=(ImportDir const& other);

  ImportDir(ImportDir&& other) HADESMEM_NOEXCEPT;
  
  ImportDir& operator=(ImportDir&& other) HADESMEM_NOEXCEPT;
  
  ~ImportDir();

  PVOID GetBase() const HADESMEM_NOEXCEPT;

  DWORD GetCharacteristics() const;

  DWORD GetTimeDateStamp() const;

  DWORD GetForwarderChain() const;

  DWORD GetNameRaw() const;

  std::string GetName() const;

  DWORD GetFirstThunk() const;

  void SetCharacteristics(DWORD characteristics) const;

  void SetTimeDateStamp(DWORD time_date_stamp) const;

  void SetForwarderChain(DWORD forwarder_chain) const;

  void SetNameRaw(DWORD name) const;

  void SetName(std::string const& name) const;

  void SetFirstThunk(DWORD first_thunk) const;
  
private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

bool operator==(ImportDir const& lhs, ImportDir const& rhs) HADESMEM_NOEXCEPT;

bool operator!=(ImportDir const& lhs, ImportDir const& rhs) HADESMEM_NOEXCEPT;

bool operator<(ImportDir const& lhs, ImportDir const& rhs) HADESMEM_NOEXCEPT;

bool operator<=(ImportDir const& lhs, ImportDir const& rhs) HADESMEM_NOEXCEPT;

bool operator>(ImportDir const& lhs, ImportDir const& rhs) HADESMEM_NOEXCEPT;

bool operator>=(ImportDir const& lhs, ImportDir const& rhs) HADESMEM_NOEXCEPT;

std::ostream& operator<<(std::ostream& lhs, ImportDir const& rhs);

std::wostream& operator<<(std::wostream& lhs, ImportDir const& rhs);

}
