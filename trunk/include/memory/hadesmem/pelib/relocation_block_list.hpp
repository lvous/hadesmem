// Copyright (C) 2010-2014 Joshua Boyce.
// See the file COPYING for copying permission.

#pragma once

#include <iterator>
#include <memory>
#include <utility>

#include <windows.h>

#include <hadesmem/config.hpp>
#include <hadesmem/detail/assert.hpp>
#include <hadesmem/detail/optional.hpp>
#include <hadesmem/error.hpp>
#include <hadesmem/pelib/nt_headers.hpp>
#include <hadesmem/pelib/relocation_block.hpp>
#include <hadesmem/pelib/pe_file.hpp>
#include <hadesmem/process.hpp>
#include <hadesmem/read.hpp>

namespace hadesmem
{

// RelocationBlockIterator satisfies the requirements of an input iterator
// (C++ Standard, 24.2.1, Input Iterators [input.iterators]).
template <typename RelocationBlockT>
class RelocationBlockIterator
  : public std::iterator<std::input_iterator_tag, RelocationBlockT>
{
public:
  using BaseIteratorT =
    std::iterator<std::input_iterator_tag, RelocationBlockT>;
  using value_type = typename BaseIteratorT::value_type;
  using difference_type = typename BaseIteratorT::difference_type;
  using pointer = typename BaseIteratorT::pointer;
  using reference = typename BaseIteratorT::reference;
  using iterator_category = typename BaseIteratorT::iterator_category;

  HADESMEM_DETAIL_CONSTEXPR RelocationBlockIterator() HADESMEM_DETAIL_NOEXCEPT
    : impl_()
  {
  }

  explicit RelocationBlockIterator(Process const& process,
                                   PeFile const& pe_file)
    : impl_(std::make_shared<Impl>(process, pe_file))
  {
    try
    {
      NtHeaders const nt_headers(process, pe_file);

      DWORD const data_dir_va =
        nt_headers.GetDataDirectoryVirtualAddress(PeDataDir::BaseReloc);
      DWORD const size = nt_headers.GetDataDirectorySize(PeDataDir::BaseReloc);
      if (!data_dir_va || !size)
      {
        impl_.reset();
        return;
      }

      auto base = static_cast<PBYTE>(RvaToVa(process, pe_file, data_dir_va));
      if (!base)
      {
        impl_.reset();
        return;
      }

      // Cast to integer and back to avoid pointer overflow UB.
      impl_->reloc_dir_end_ = reinterpret_cast<void const*>(
        reinterpret_cast<std::uintptr_t>(base) + size);
      auto const file_end =
        static_cast<std::uint8_t*>(pe_file.GetBase()) + pe_file.GetSize();
      // Sample: virtrelocXP.exe
      if (pe_file.GetType() == PeFileType::Data &&
          (impl_->reloc_dir_end_ < base || impl_->reloc_dir_end_ > file_end))
      {
        impl_.reset();
        return;
      }

      impl_->relocation_block_ =
        RelocationBlock(process,
                        pe_file,
                        reinterpret_cast<PIMAGE_BASE_RELOCATION>(base),
                        impl_->reloc_dir_end_);
      if (impl_->relocation_block_->IsInvalid())
      {
        impl_.reset();
        return;
      }
    }
    catch (std::exception const& /*e*/)
    {
      impl_.reset();
    }
  }

#if defined(HADESMEM_DETAIL_NO_RVALUE_REFERENCES_V3)

  RelocationBlockIterator(RelocationBlockIterator const&) = default;

  RelocationBlockIterator& operator=(RelocationBlockIterator const&) = default;

  RelocationBlockIterator(RelocationBlockIterator&& other)
HADESMEM_DETAIL_NOEXCEPT:
  impl_(std::move(other.impl_))
  {
  }

  RelocationBlockIterator& operator=(RelocationBlockIterator&& other)
    HADESMEM_DETAIL_NOEXCEPT
  {
    impl_ = std::move(other.impl_);

    return *this;
  }

#endif // #if defined(HADESMEM_DETAIL_NO_RVALUE_REFERENCES_V3)

  reference operator*() const HADESMEM_DETAIL_NOEXCEPT
  {
    HADESMEM_DETAIL_ASSERT(impl_.get());
    return *impl_->relocation_block_;
  }

  pointer operator->() const HADESMEM_DETAIL_NOEXCEPT
  {
    HADESMEM_DETAIL_ASSERT(impl_.get());
    return &*impl_->relocation_block_;
  }

  RelocationBlockIterator& operator++()
  {
    try
    {
      HADESMEM_DETAIL_ASSERT(impl_.get());

      auto const next_base = reinterpret_cast<PIMAGE_BASE_RELOCATION>(
        reinterpret_cast<std::uintptr_t>(
          impl_->relocation_block_->GetRelocationDataStart()) +
        (impl_->relocation_block_->GetNumberOfRelocations() * sizeof(WORD)));
      if (next_base < impl_->relocation_block_->GetBase() ||
          next_base >= impl_->reloc_dir_end_)
      {
        impl_.reset();
        return *this;
      }
      impl_->relocation_block_ = RelocationBlock(
        *impl_->process_, *impl_->pe_file_, next_base, impl_->reloc_dir_end_);
      if (impl_->relocation_block_->IsInvalid())
      {
        impl_.reset();
        return *this;
      }
    }
    catch (std::exception const& /*e*/)
    {
      impl_.reset();
    }

    return *this;
  }

  RelocationBlockIterator operator++(int)
  {
    RelocationBlockIterator const iter(*this);
    ++*this;
    return iter;
  }

  bool operator==(RelocationBlockIterator const& other) const
    HADESMEM_DETAIL_NOEXCEPT
  {
    return impl_ == other.impl_;
  }

  bool operator!=(RelocationBlockIterator const& other) const
    HADESMEM_DETAIL_NOEXCEPT
  {
    return !(*this == other);
  }

private:
  struct Impl
  {
    explicit Impl(Process const& process,
                  PeFile const& pe_file) HADESMEM_DETAIL_NOEXCEPT
      : process_(&process),
        pe_file_(&pe_file),
        relocation_block_(),
        reloc_dir_end_(nullptr)
    {
    }

    Process const* process_;
    PeFile const* pe_file_;
    hadesmem::detail::Optional<RelocationBlock> relocation_block_;
    void const* reloc_dir_end_;
  };

  // Using a shared_ptr to provide shallow copy semantics, as
  // required by InputIterator.
  std::shared_ptr<Impl> impl_;
};

class RelocationBlockList
{
public:
  using value_type = RelocationBlock;
  using iterator = RelocationBlockIterator<RelocationBlock>;
  using const_iterator = RelocationBlockIterator<RelocationBlock const>;

  explicit RelocationBlockList(Process const& process, PeFile const& pe_file)
    : process_(&process), pe_file_(&pe_file)
  {
  }

  iterator begin()
  {
    return iterator(*process_, *pe_file_);
  }

  const_iterator begin() const
  {
    return const_iterator(*process_, *pe_file_);
  }

  const_iterator cbegin() const
  {
    return const_iterator(*process_, *pe_file_);
  }

  iterator end() HADESMEM_DETAIL_NOEXCEPT
  {
    return iterator();
  }

  const_iterator end() const HADESMEM_DETAIL_NOEXCEPT
  {
    return const_iterator();
  }

  const_iterator cend() const HADESMEM_DETAIL_NOEXCEPT
  {
    return const_iterator();
  }

private:
  Process const* process_;
  PeFile const* pe_file_;
};
}
