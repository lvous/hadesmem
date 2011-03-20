/*
This file is part of HadesMem.
Copyright (C) 2010 Joshua Boyce (aka RaptorFactor, Cypherjb, Cypher, Chazwazza).
<http://www.raptorfactor.com/> <raptorfactor@raptorfactor.com>

HadesMem is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

HadesMem is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with HadesMem.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

// C++ Standard Library
#include <string>
#include <vector>

// Boost
#pragma warning(push, 1)
#include <boost/noncopyable.hpp>
#pragma warning(pop)

namespace Hades
{
  namespace Util
  {
    // Utility class to 'bind' a temporary buffer to a string and allow 
    // easy interoperability with C-style APIs.
    template <class CharT>
    class StringBuffer : private boost::noncopyable
    {
    public:
      // Constructor
      StringBuffer(std::basic_string<CharT>& String, 
        typename std::vector<CharT>::size_type Size) 
        : m_String(&String), 
        m_Buffer(Size + 1) 
      { }

      // Move constructor
      StringBuffer(StringBuffer&& Other)
      {
        *this = std::move(Other);
      }

      // Move assignment operator
      StringBuffer& operator=(StringBuffer&& Other)
      {
        this->m_String = Other.m_String;
        Other.m_String = nullptr;

        this->m_Buffer = std::move(Other.m_Buffer);

        return *this;
      }

      // Destructor
      ~StringBuffer()
      {
        // Commit current buffer
        Commit();
      }

      // Get pointer to internal buffer
      CharT* Get()
      {
        return m_Buffer.data();
      }

      // Get pointer to internal buffer
      CharT* Get() const
      {
        return m_Buffer.data();
      }

      // Implicit conversion operator to allow for easy C-style API 
      // interoperability
      operator CharT* ()
      {
        return Get();
      }

      // Commit current buffer to target string
      void Commit()
      {
        if (!m_Buffer.empty())
        {
          *m_String = m_Buffer.data();
          m_Buffer.clear();
        }
      }

      // Clear current buffer
      void Abort()
      {
        m_Buffer.clear();
      }

    private:
      // Target string
      std::basic_string<CharT>* m_String;

      // Temporary buffer
      std::vector<CharT> m_Buffer;
    };

    // Make string buffer. Simple helper function to automatically detect 
    // character type.
    template <class CharT>
    StringBuffer<CharT> MakeStringBuffer(std::basic_string<CharT>& String, 
      std::size_t Size)
    {
      return StringBuffer<CharT>(String, Size);
    }
  }
}
