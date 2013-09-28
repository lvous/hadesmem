// Copyright (C) 2010-2013 Joshua Boyce.
// See the file COPYING for copying permission.

#pragma once

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <limits>
#include <locale>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <hadesmem/detail/warning_disable_prefix.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_uint.hpp>
#include <hadesmem/detail/warning_disable_suffix.hpp>

#include <windows.h>

#include <hadesmem/config.hpp>
#include <hadesmem/detail/assert.hpp>
#include <hadesmem/detail/pattern_info.hpp>
#include <hadesmem/detail/static_assert.hpp>
#include <hadesmem/detail/str_conv.hpp>
#include <hadesmem/error.hpp>
#include <hadesmem/module_list.hpp>
#include <hadesmem/pelib/dos_header.hpp>
#include <hadesmem/pelib/nt_headers.hpp>
#include <hadesmem/pelib/pe_file.hpp>
#include <hadesmem/pelib/section.hpp>
#include <hadesmem/pelib/section_list.hpp>
#include <hadesmem/process.hpp>
#include <hadesmem/read.hpp>

// TODO: Review, refactor, rewrite, etc this entire module. Put TODOs where 
// appropriate, remove and add APIs, fix bugs, clean up code, etc. Use new 
// language features like noexcept, constexpr, etc.

// TODO: Add stream overloads.

// TODO: Rewrite to remove cyclic dependencies.

// TODO: Pattern generator.

// TODO: Multi-pass support (e.g. search for pattern, apply for manipulators, 
// use as starting point for second search).

// TODO: Arbitrary region support.

// Clang generates a warning for all inline classes with virtual methods, due 
// to the potential object file bloat it may cause.
// error: 'Manipulator' has no out-of-line virtual method definitions; its 
// vtable will be emitted in every translation unit
// TODO: Fix the code and remove this suppression.
#if defined(HADESMEM_CLANG)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wweak-vtables"
#endif // #if defined(HADESMEM_CLANG)

namespace hadesmem
{

struct FindPatternFlags
{
  enum
  {
    kNone = 0, 
    kThrowOnUnmatch = 1 << 0, 
    kRelativeAddress = 1 << 1, 
    kScanData = 1 << 2, 
    kInvalidFlagMaxValue = 1 << 3
  };
};

class FindPattern;

class Pattern
{
public:
  Pattern(FindPattern& finder, std::wstring const& data, std::uint32_t flags);
  
  Pattern(FindPattern& finder, std::wstring const& data, 
    std::wstring const& name, std::uint32_t flags);

#if !defined(HADESMEM_DETAIL_NO_DEFAULTED_FUNCTIONS)

  Pattern(Pattern const&) HADESMEM_DETAIL_DEFAULTED_FUNCTION;

  Pattern& operator=(Pattern const&) HADESMEM_DETAIL_DEFAULTED_FUNCTION;

  Pattern(Pattern&&) HADESMEM_DETAIL_DEFAULTED_FUNCTION;

  Pattern& operator=(Pattern&&) HADESMEM_DETAIL_DEFAULTED_FUNCTION;

#else // #if !defined(HADESMEM_DETAIL_NO_DEFAULTED_FUNCTIONS)

  Pattern(Pattern const& other)
    : finder_(other.finder_), 
    name_(other.name_), 
    address_(other.address_), 
    flags_(other.flags_)
  { }

  Pattern& operator=(Pattern const& other)
  {
    Pattern tmp(other);
    *this = std::move(tmp);

    return *this;
  }

  Pattern(Pattern&& other) HADESMEM_DETAIL_NOEXCEPT
    : finder_(other.finder_), 
    name_(std::move(other.name_)), 
    address_(other.address_), 
    flags_(other.flags_)
  {
    other.finder_ = nullptr;
    other.address_ = nullptr;
    other.flags_ = FindPatternFlags::kNone;
  }

  Pattern& operator=(Pattern&& other) HADESMEM_DETAIL_NOEXCEPT
  {
    finder_ = other.finder_;
    other.finder_ = nullptr;

    name_ = std::move(other.name_);

    address_ = other.address_;
    other.address_ = nullptr;

    flags_ = other.flags_;
    other.flags_ = FindPatternFlags::kNone;

    return *this;
  }

#endif // #if !defined(HADESMEM_DETAIL_NO_DEFAULTED_FUNCTIONS)

  void Save();

  // TODO: More consistent naming.
  void Update(PBYTE address)
  {
    address_ = address;
  }

  PBYTE GetAddress() const HADESMEM_DETAIL_NOEXCEPT
  {
    return address_;
  }

  std::uint32_t GetFlags() const HADESMEM_DETAIL_NOEXCEPT
  {
    return flags_;
  }

  DWORD_PTR GetBase() const HADESMEM_DETAIL_NOEXCEPT;

  Process const* GetProcess() const HADESMEM_DETAIL_NOEXCEPT;
    
private:
  FindPattern* finder_;
  std::wstring name_;
  PBYTE address_;
  std::uint32_t flags_;
};

namespace pattern_manipulators
{

// TODO: Would templates make more sense here?

class ManipulatorBase
{
public:
  virtual ~ManipulatorBase() HADESMEM_DETAIL_NOEXCEPT
  { }
};

template <typename D>
class Manipulator : public ManipulatorBase
{
public:
  void Manipulate(Pattern& pattern) const
  {
    return static_cast<D const*>(this)->Manipulate(pattern);
  }
};

template <typename D>
inline Pattern& operator<<(Pattern& pattern, 
  Manipulator<D> const& manipulator)
{
  manipulator.Manipulate(pattern);
  return pattern;
}

class Save : public Manipulator<Save>
{
public:
  void Manipulate(Pattern& pattern) const;
};

class Add : public Manipulator<Add>
{
public:
  explicit Add(DWORD_PTR offset) HADESMEM_DETAIL_NOEXCEPT
    : offset_(offset)
  { }
  
  void Manipulate(Pattern& pattern) const;
  
private:
  DWORD_PTR offset_;
};

class Sub : public Manipulator<Sub>
{
public:
  explicit Sub(DWORD_PTR offset) HADESMEM_DETAIL_NOEXCEPT
    : offset_(offset)
  { }
  
  void Manipulate(Pattern& pattern) const;
    
private:
  DWORD_PTR offset_;
};

class Lea : public Manipulator<Lea>
{
public:
  void Manipulate(Pattern& pattern) const;
};

class Rel : public Manipulator<Rel>
{
public:
  explicit Rel(DWORD_PTR size, DWORD_PTR offset) HADESMEM_DETAIL_NOEXCEPT
    : size_(size), 
    offset_(offset)
  { }
  
  void Manipulate(Pattern& pattern) const;
  
private:
  DWORD_PTR size_;
  DWORD_PTR offset_;
};

}

class FindPattern
{
public:
  friend class Pattern;
  
  explicit FindPattern(Process const& process, HMODULE module)
    : process_(&process), 
    base_(0), 
    code_regions_(), 
    data_regions_(), 
    addresses_()
  {
    if (!module)
    {
      ModuleList const modules(process);
      auto const exe_mod = std::begin(modules);
      HADESMEM_DETAIL_ASSERT(exe_mod != std::end(modules));
      module = exe_mod->GetHandle();
    }
    
    PBYTE const base = reinterpret_cast<PBYTE>(module);
    base_ = reinterpret_cast<DWORD_PTR>(base);
    PeFile const pe_file(process, reinterpret_cast<PVOID>(base), 
      hadesmem::PeFileType::Image);
    DosHeader const dos_header(process, pe_file);
    NtHeaders const nt_headers(process, pe_file);
    
    SectionList const sections(process, pe_file);
    for (auto const& s : sections)
    {
      bool const is_code_section = 
        !!(s.GetCharacteristics() & IMAGE_SCN_CNT_CODE);
      bool const is_data_section = 
        !!(s.GetCharacteristics() & IMAGE_SCN_CNT_INITIALIZED_DATA);
      if (!is_code_section && !is_data_section)
      {
        continue;
      }

      PBYTE const section_beg = static_cast<PBYTE>(RvaToVa(process, 
        pe_file, s.GetVirtualAddress()));
      if (section_beg == nullptr)
      {
        HADESMEM_DETAIL_THROW_EXCEPTION(Error() << 
          ErrorString("Could not get section base address."));
      }
          
      PBYTE const section_end = section_beg + s.GetSizeOfRawData();

      std::vector<std::pair<PBYTE, PBYTE>>& region = 
        is_code_section ? code_regions_ : data_regions_;
      region.emplace_back(section_beg, section_end);
    }
    
    if (code_regions_.empty() && data_regions_.empty())
    {
      HADESMEM_DETAIL_THROW_EXCEPTION(Error() << 
        ErrorString("No valid sections to scan found."));
    }
  }

#if !defined(HADESMEM_DETAIL_NO_DEFAULTED_FUNCTIONS)

  FindPattern(FindPattern const&) HADESMEM_DETAIL_DEFAULTED_FUNCTION;

  FindPattern& operator=(FindPattern const&) 
    HADESMEM_DETAIL_DEFAULTED_FUNCTION;

  FindPattern(FindPattern&&) HADESMEM_DETAIL_DEFAULTED_FUNCTION;

  FindPattern& operator=(FindPattern&&) HADESMEM_DETAIL_DEFAULTED_FUNCTION;

#else // #if !defined(HADESMEM_DETAIL_NO_DEFAULTED_FUNCTIONS)

  FindPattern(FindPattern const& other)
    : process_(other.process_), 
    base_(other.base_), 
    code_regions_(other.code_regions_), 
    data_regions_(other.data_regions_), 
    addresses_(other.addresses_)
  { }

  FindPattern& operator=(FindPattern const& other)
  {
    FindPattern tmp(other);
    *this = std::move(tmp);

    return *this;
  }

  FindPattern(FindPattern&& other) HADESMEM_DETAIL_NOEXCEPT
    : process_(other.process_), 
    base_(other.base_), 
    code_regions_(std::move(other.code_regions_)), 
    data_regions_(std::move(other.data_regions_)), 
    addresses_(std::move(other.addresses_))
  {
    other.process_ = nullptr;
    other.base_ = 0;
  }

  FindPattern& operator=(FindPattern&& other) HADESMEM_DETAIL_NOEXCEPT
  {
    process_ = other.process_;
    other.process_ = nullptr;

    base_ = other.base_;
    other.base_ = 0;

    code_regions_ = std::move(other.code_regions_);

    data_regions_ = std::move(other.data_regions_);

    addresses_ = std::move(other.addresses_);

    return *this;
  }

#endif // #if !defined(HADESMEM_DETAIL_NO_DEFAULTED_FUNCTIONS)
  
  PVOID Find(std::wstring const& data, std::uint32_t flags) const
  {
    HADESMEM_DETAIL_ASSERT(!(flags & 
      ~(FindPatternFlags::kInvalidFlagMaxValue - 1UL)));

    HADESMEM_DETAIL_ASSERT(!data.empty());

    std::wstring const data_trimmed = data.substr(0, 
      data.find_last_not_of(L" \n\r\t") + 1);

    HADESMEM_DETAIL_ASSERT(!data_trimmed.empty());

    std::wistringstream data_str(data_trimmed);
    data_str.imbue(std::locale::classic());
    std::vector<std::pair<BYTE, bool>> data_real;
    for (;;)
    {
      std::wstring data_cur_str;
      if (!(data_str >> data_cur_str))
      {
        HADESMEM_DETAIL_THROW_EXCEPTION(Error() << 
          ErrorString("Data parsing failed."));
      }

      bool const is_wildcard = (data_cur_str == L"??");
      std::uint32_t current = 0U;
      if (!is_wildcard)
      {
        std::wistringstream conv(data_cur_str);
        conv.imbue(std::locale::classic());
        if (!(conv >> std::hex >> current))
        {
          HADESMEM_DETAIL_THROW_EXCEPTION(Error() << 
            ErrorString("Data conversion failed."));
        }

        if (current > 0xFFU)
        {
          HADESMEM_DETAIL_THROW_EXCEPTION(Error() << 
            ErrorString("Invalid data."));
        }
      }

      data_real.emplace_back(static_cast<BYTE>(current), !is_wildcard);

      if (data_str.eof())
      {
        break;
      }
    }
    
    bool const scan_data_secs = !!(flags & FindPatternFlags::kScanData);
    
    PVOID address = Find(data_real, scan_data_secs);
    
    if (!address && !!(flags & FindPatternFlags::kThrowOnUnmatch))
    {
      HADESMEM_DETAIL_THROW_EXCEPTION(Error() << 
        ErrorString("Could not match pattern."));
    }
    
    if (address && !!(flags & FindPatternFlags::kRelativeAddress))
    {
      address = static_cast<PBYTE>(address) - base_;
    }
    
    return address;
  }
  
  PVOID Find(std::wstring const& data, std::wstring const& name, std::uint32_t flags)
  {
    PVOID const address = Find(data, flags);
    
    if (!name.empty())
    {
      addresses_[name] = address;
    }
    
    return address;
  }

  std::map<std::wstring, PVOID> GetAddresses() const
  {
    return addresses_;
  }

  PVOID operator[](std::wstring const& name) const
  {
    auto const iter = addresses_.find(name);
    return (iter != addresses_.end()) ? iter->second : nullptr;
  }

  PVOID Lookup(std::wstring const& name) const
  {
    auto const iter = addresses_.find(name);
    if (iter == std::end(addresses_))
    {
      HADESMEM_DETAIL_THROW_EXCEPTION(Error() << 
        ErrorString("Could not find target pattern."));
    }

    return iter->second;
  }
      
  void LoadFile(std::wstring const& path)
  {
#if defined(HADESMEM_MSVC) || defined(HADESMEM_INTEL)
    std::wifstream pattern_file(path, 
      std::ios::binary | std::ios::ate);
#else // #if defined(HADESMEM_MSVC) || defined(HADESMEM_INTEL)
    // TODO: Fix this for compilers other than MSVC and ICC.
    std::wifstream pattern_file(
      hadesmem::detail::WideCharToMultiByte(path), 
      std::ios::binary | std::ios::ate);
#endif // #if defined(HADESMEM_MSVC) || defined(HADESMEM_INTEL)
    if (!pattern_file)
    {
      HADESMEM_DETAIL_THROW_EXCEPTION(Error() << 
        ErrorString("Could not open pattern file."));
    }
    
    std::istreambuf_iterator<wchar_t> const pat_file_beg(pattern_file);
    std::istreambuf_iterator<wchar_t> const pat_file_end;
    std::vector<wchar_t> pat_file_buf(pat_file_beg, pat_file_end);
    pat_file_buf.push_back(L'\0');
    
    LoadFileMemory(pat_file_buf.data());
  }
    
  void LoadFileMemory(std::wstring const& data)
  {
    typedef std::wstring::const_iterator DataIter;
    typedef boost::spirit::qi::standard::space_type SkipWsT;
    
    typedef boost::spirit::qi::symbols<wchar_t, int> FlagsParser;
    FlagsParser flags_parser;
    flags_parser.add
      (L"None", FindPatternFlags::kNone)
      (L"ThrowOnUnmatch", FindPatternFlags::kThrowOnUnmatch)
      (L"RelativeAddress", FindPatternFlags::kRelativeAddress)
      (L"ScanData", FindPatternFlags::kScanData);
    
    boost::spirit::qi::rule<DataIter, std::vector<int>(), SkipWsT> 
      const flags_rule = '(' >> *(flags_parser % ',') >> ')';
    boost::spirit::qi::rule<DataIter, std::wstring()> const name_rule = 
      boost::spirit::qi::lexeme[*(~boost::spirit::qi::char_(','))] >> ',';
    boost::spirit::qi::rule<DataIter, std::wstring()> const data_rule = 
      boost::spirit::qi::lexeme[*(~boost::spirit::qi::char_('}'))];
    boost::spirit::qi::rule<DataIter, detail::PatternInfo(), SkipWsT> 
      const pattern_rule = '{' >> name_rule >> data_rule >> '}';
    
    typedef boost::spirit::qi::symbols<wchar_t, int> ManipParser;
    ManipParser manip_parser;
    manip_parser.add
      (L"Add", detail::ManipInfo::Manipulator::kAdd)
      (L"Sub", detail::ManipInfo::Manipulator::kSub)
      (L"Rel", detail::ManipInfo::Manipulator::kRel)
      (L"Lea", detail::ManipInfo::Manipulator::kLea);
    
    boost::spirit::qi::rule<DataIter, int(), SkipWsT> const manip_name_rule = 
      manip_parser >> ',';
    boost::spirit::qi::rule<DataIter, std::vector<DWORD_PTR>(), SkipWsT> 
#if defined(HADESMEM_DETAIL_ARCH_X86)
      const operand_rule = (boost::spirit::ulong_ % ',');
#elif defined(HADESMEM_DETAIL_ARCH_X64)
      const operand_rule = (boost::spirit::ulong_long % ',');
#endif
    boost::spirit::qi::rule<DataIter, detail::ManipInfo(), SkipWsT> 
      const manip_rule = ('[' >> manip_name_rule >> operand_rule >> ']');
    boost::spirit::qi::rule<DataIter, detail::PatternInfoFull(), SkipWsT> 
      const pattern_full_rule = (pattern_rule >> *manip_rule);
    
    std::vector<int> flags_list;
    std::vector<detail::PatternInfoFull> pattern_list;
    
    auto data_beg = std::begin(data);
    auto const data_end = std::end(data);
    bool const parsed = boost::spirit::qi::phrase_parse(
      data_beg, 
      data_end, 
      (L"HadesMem Patterns" >> flags_rule >> *pattern_full_rule), 
      boost::spirit::qi::space, 
      flags_list, 
      pattern_list);
    if (!parsed || data_beg != data_end)
    {
      HADESMEM_DETAIL_THROW_EXCEPTION(Error() << 
        ErrorString("Parsing failed."));
    }
    
    std::uint32_t flags = FindPatternFlags::kNone;
    std::for_each(std::begin(flags_list), std::end(flags_list), 
      [&] (std::uint32_t flag)
      {
        flags |= flag;
      });
    
    for (auto const& p : pattern_list)
    {
      detail::PatternInfo const& pat_info = p.pattern;
      Pattern pattern(*this, pat_info.data, pat_info.name, flags);
      
      std::vector<detail::ManipInfo> const& manip_list = p.manipulators;
      for (auto const& m : manip_list)
      {
        switch (m.type)
        {
        case detail::ManipInfo::Manipulator::kAdd:
          if (m.operands.size() != 1)
          {
            HADESMEM_DETAIL_THROW_EXCEPTION(Error() << 
              ErrorString("Invalid manipulator operands for 'Add'."));
          }
          
          pattern << pattern_manipulators::Add(m.operands[0]);
          
          break;
          
        case detail::ManipInfo::Manipulator::kSub:
          if (m.operands.size() != 1)
          {
            HADESMEM_DETAIL_THROW_EXCEPTION(Error() << 
              ErrorString("Invalid manipulator operands for 'Sub'."));
          }
          
          pattern << pattern_manipulators::Sub(m.operands[0]);
          
          break;
          
        case detail::ManipInfo::Manipulator::kRel:
          if (m.operands.size() != 2)
          {
            HADESMEM_DETAIL_THROW_EXCEPTION(Error() << 
              ErrorString("Invalid manipulator operands for 'Rel'."));
          }
          
          pattern << pattern_manipulators::Rel(m.operands[0], m.operands[1]);
          
          break;
          
        case detail::ManipInfo::Manipulator::kLea:
          if (m.operands.size() != 0)
          {
            HADESMEM_DETAIL_THROW_EXCEPTION(Error() << 
              ErrorString("Invalid manipulator operands for 'Lea'."));
          }
          
          pattern << pattern_manipulators::Lea();
          
          break;
          
        default:
          HADESMEM_DETAIL_THROW_EXCEPTION(Error() << 
            ErrorString("Unknown manipulator."));
        }
      }
      
      pattern.Save();
    }
  }


  friend bool operator==(FindPattern const& lhs, FindPattern const& rhs)
  {
    return lhs.process_ == rhs.process_ && 
      lhs.base_ == rhs.base_ && 
      lhs.addresses_ == rhs.addresses_;
  }

  friend bool operator!=(FindPattern const& lhs, FindPattern const& rhs)
  {
    return !(lhs == rhs);
  }
  
private:
  PVOID Find(std::vector<std::pair<BYTE, bool>> const& data, 
    bool scan_data_secs) const
  {
    HADESMEM_DETAIL_ASSERT(!data.empty());
    
    std::vector<std::pair<PBYTE, PBYTE>> const& scan_regions = 
      scan_data_secs ? data_regions_ : code_regions_;
    for (auto const& region : scan_regions)
    {
      PBYTE const s_beg = region.first;
      PBYTE const s_end = region.second;
      HADESMEM_DETAIL_ASSERT(s_end > s_beg);
      
      std::ptrdiff_t const mem_size = s_end - s_beg;
      HADESMEM_DETAIL_ASSERT(s_beg <= s_end);
      std::vector<BYTE> const buffer(ReadVector<BYTE>(*process_, s_beg, 
        static_cast<std::size_t>(mem_size)));
      
      auto const iter = std::search(
        std::begin(buffer), 
        std::end(buffer), 
        std::begin(data), 
        std::end(data), 
        [] (BYTE h_cur, std::pair<BYTE, bool> const& n_cur)
        {
          return (!n_cur.second) || (h_cur == n_cur.first);
        });
      
      if (iter != std::end(buffer))
      {
        return (s_beg + std::distance(std::begin(buffer), iter));
      }
    }
    
    return nullptr;
  }

  Process const* process_;
  DWORD_PTR base_;
  std::vector<std::pair<PBYTE, PBYTE>> code_regions_;
  std::vector<std::pair<PBYTE, PBYTE>> data_regions_;
  std::map<std::wstring, PVOID> addresses_;
};

inline Pattern::Pattern(FindPattern& finder, 
  std::wstring const& data, 
  std::uint32_t flags)
  : finder_(&finder), 
  name_(), 
  address_(static_cast<PBYTE>(finder.Find(data, flags))), 
  flags_(flags)
{ }

inline Pattern::Pattern(FindPattern& finder, 
  std::wstring const& data, 
  std::wstring const& name, 
  std::uint32_t flags)
  : finder_(&finder), 
  name_(name), 
  address_(static_cast<PBYTE>(finder.Find(data, flags))), 
  flags_(flags)
{ }

inline void Pattern::Save()
{
  if (name_.empty())
  {
    return;
  }

  // TODO: This feels like a hack. Investigate and fix this. (And if 
  // appropriate, remove friendship requirement.)
  finder_->addresses_[name_] = address_;
}

inline DWORD_PTR Pattern::GetBase() const HADESMEM_DETAIL_NOEXCEPT
{
  // TODO: This feels like a hack. Investigate and fix this. (And if 
  // appropriate, remove friendship requirement.)
  return finder_->base_;
}

// TODO: This feels like a hack. Investigate and fix this.
inline Process const* Pattern::GetProcess() const HADESMEM_DETAIL_NOEXCEPT
{
  return finder_->process_;
}

namespace pattern_manipulators
{

inline void Save::Manipulate(Pattern& pattern) const
{
  pattern.Save();
}

inline void Add::Manipulate(Pattern& pattern) const
{
  PBYTE const address = pattern.GetAddress();
  if (!address)
  {
    return;
  }

  pattern.Update(address + offset_);
}

inline void Sub::Manipulate(Pattern& pattern) const
{
  PBYTE const address = pattern.GetAddress();
  if (!address)
  {
    return;
  }
  
  pattern.Update(address - offset_);
}

inline void Lea::Manipulate(Pattern& pattern) const
{
  PBYTE address = pattern.GetAddress();
  if (!address)
  {
    return;
  }

  try
  {
    bool const is_relative_address = 
      !!(pattern.GetFlags() & FindPatternFlags::kRelativeAddress);
    DWORD_PTR base = is_relative_address ? pattern.GetBase() : 0;
    address = Read<PBYTE>(*pattern.GetProcess(), address + base);
  }
  catch (std::exception const& /*e*/)
  {
    address = nullptr;
  }

  pattern.Update(address);
}

inline void Rel::Manipulate(Pattern& pattern) const
{
  PBYTE address = pattern.GetAddress();
  if (!address)
  {
    return;
  }

  try
  {
    bool const is_relative_address = 
      !!(pattern.GetFlags() & FindPatternFlags::kRelativeAddress);
    DWORD_PTR const base = is_relative_address ? pattern.GetBase() : 0;
    address = Read<PBYTE>(*pattern.GetProcess(), address + base) + 
      reinterpret_cast<DWORD_PTR>(address + base) + size_ - offset_;
  }
  catch (std::exception const& /*e*/)
  {
    address = nullptr;
  }

  pattern.Update(address);
}

}

}

#if defined(HADESMEM_CLANG)
#pragma GCC diagnostic pop
#endif // #if defined(HADESMEM_CLANG)
