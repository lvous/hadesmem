// Copyright (C) 2010-2014 Joshua Boyce.
// See the file COPYING for copying permission.

#pragma once

#include <cstdint>

struct Offsets
{
  enum : std::uint32_t
  {
    g_character_manager = 0x00E4C524 - 0x00400000,
    g_translated_string_repository = 0x00DEC4C4 - 0x00400000,
  };
};
