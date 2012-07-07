// [AsmJit]
// Complete JIT Assembler for C++ Language.
//
// [License]
// Zlib - See COPYING file in this package.

// [Guard]
#ifndef _ASMJIT_CORE_CONTEXT_H
#define _ASMJIT_CORE_CONTEXT_H

// [Dependencies - AsmJit]
#include "../Core/Build.h"

namespace AsmJit {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct Assembler;
struct MemoryManager;
struct MemoryMarker;

// ============================================================================
// [AsmJit::Context]
// ============================================================================

//! @brief Class for changing behavior of code generated by @ref Assembler and
//! @ref Compiler.
struct Context
{
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a @c Context instance.
  ASMJIT_API Context();
  //! @brief Destroy the @c Context instance.
  ASMJIT_API virtual ~Context();

  // --------------------------------------------------------------------------
  // [Interface]
  // --------------------------------------------------------------------------

  //! @brief Allocate memory for code generated in @a assembler and reloc it
  //! to target location.
  //!
  //! This method is universal allowing any pre-process / post-process work
  //! with code generated by @c Assembler or @c Compiler. Because @c Compiler
  //! always uses @c Assembler it's allowed to access only the @c Assembler
  //! instance.
  //!
  //! This method is always last step when using code generation. You can use
  //! it to allocate memory for JIT code, saving code to remote process or a 
  //! shared library.
  //!
  //! @retrurn Error value, see @c kError.
  virtual uint32_t generate(void** dest, Assembler* assembler) = 0;

  ASMJIT_NO_COPY(Context)
};

// ============================================================================
// [AsmJit::JitContext]
// ============================================================================

struct JitContext : public Context
{
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a @c JitContext instance.
  ASMJIT_API JitContext();
  //! @brief Destroy the @c JitContext instance.
  ASMJIT_API virtual ~JitContext();

  // --------------------------------------------------------------------------
  // [Memory Manager and Alloc Type]
  // --------------------------------------------------------------------------

  // Note: These members can be ignored by all derived classes. They are here
  // only to privide default implementation. All other implementations (remote
  // code patching or making dynamic loadable libraries/executables) ignore
  // members accessed by these accessors.

  //! @brief Get the @c MemoryManager instance.
  inline MemoryManager* getMemoryManager() const
  { return _memoryManager; }

  //! @brief Set the @c MemoryManager instance.
  inline void setMemoryManager(MemoryManager* memoryManager)
  { _memoryManager = memoryManager; }

  //! @brief Get the type of allocation.
  inline uint32_t getAllocType() const
  { return _allocType; }

  //! @brief Set the type of allocation.
  inline void setAllocType(uint32_t allocType)
  { _allocType = allocType; }

  // --------------------------------------------------------------------------
  // [Memory Marker]
  // --------------------------------------------------------------------------

  //! @brief Get the @c MemoryMarker instance.
  inline MemoryMarker* getMemoryMarker() const
  { return _memoryMarker; }

  //! @brief Set the @c MemoryMarker instance.
  inline void setMemoryMarker(MemoryMarker* memoryMarker)
  { _memoryMarker = memoryMarker; }

  // --------------------------------------------------------------------------
  // [Interface]
  // --------------------------------------------------------------------------

  ASMJIT_API virtual uint32_t generate(void** dest, Assembler* assembler);

  // --------------------------------------------------------------------------
  // [Statics]
  // --------------------------------------------------------------------------

  ASMJIT_API static JitContext* getGlobal();

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Memory manager.
  MemoryManager* _memoryManager;
  //! @brief Memory marker.
  MemoryMarker* _memoryMarker;

  //! @brief Type of allocation.
  uint32_t _allocType;

  ASMJIT_NO_COPY(JitContext)
};

} // AsmJit namespace

// [Guard]
#endif // _ASMJIT_CORE_CONTEXT_H
