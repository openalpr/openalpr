// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  StandardTraits.h
 *
 *  Copyright (c) 2007, Daniel Aarno, Michael E. Smoot .
 *  All rights reverved.
 *
 *  See the file COPYING in the top directory of this distribution for
 *  more information.
 *
 *  THE SOFTWARE IS PROVIDED _AS IS_, WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

// This is an internal tclap file, you should probably not have to
// include this directly

#ifndef TCLAP_STANDARD_TRAITS_H
#define TCLAP_STANDARD_TRAITS_H

#ifdef HAVE_CONFIG_H
#include <config.h> // To check for long long
#endif

// If Microsoft has already typedef'd wchar_t as an unsigned
// short, then compiles will break because it's as if we're
// creating ArgTraits twice for unsigned short. Thus...
#ifdef _MSC_VER
#ifndef _NATIVE_WCHAR_T_DEFINED
#define TCLAP_DONT_DECLARE_WCHAR_T_ARGTRAITS
#endif
#endif

namespace TCLAP
{

// ======================================================================
// Integer types
// ======================================================================

/**
 * longs have value-like semantics.
 */
template<>
struct ArgTraits<long>
{
  typedef ValueLike ValueCategory;
};

/**
 * ints have value-like semantics.
 */
template<>
struct ArgTraits<int>
{
  typedef ValueLike ValueCategory;
};

/**
 * shorts have value-like semantics.
 */
template<>
struct ArgTraits<short>
{
  typedef ValueLike ValueCategory;
};

/**
 * chars have value-like semantics.
 */
template<>
struct ArgTraits<char>
{
  typedef ValueLike ValueCategory;
};

#ifdef HAVE_LONG_LONG
/**
 * long longs have value-like semantics.
 */
template<>
struct ArgTraits<long long>
{
  typedef ValueLike ValueCategory;
};
#endif

// ======================================================================
// Unsigned integer types
// ======================================================================

/**
 * unsigned longs have value-like semantics.
 */
template<>
struct ArgTraits<unsigned long>
{
  typedef ValueLike ValueCategory;
};

/**
 * unsigned ints have value-like semantics.
 */
template<>
struct ArgTraits<unsigned int>
{
  typedef ValueLike ValueCategory;
};

/**
 * unsigned shorts have value-like semantics.
 */
template<>
struct ArgTraits<unsigned short>
{
  typedef ValueLike ValueCategory;
};

/**
 * unsigned chars have value-like semantics.
 */
template<>
struct ArgTraits<unsigned char>
{
  typedef ValueLike ValueCategory;
};

// Microsoft implements size_t awkwardly.
#if defined(_MSC_VER) && defined(_M_X64)
/**
 * size_ts have value-like semantics.
 */
template<>
struct ArgTraits<size_t>
{
  typedef ValueLike ValueCategory;
};
#endif


#ifdef HAVE_LONG_LONG
/**
 * unsigned long longs have value-like semantics.
 */
template<>
struct ArgTraits<unsigned long long>
{
  typedef ValueLike ValueCategory;
};
#endif

// ======================================================================
// Float types
// ======================================================================

/**
 * floats have value-like semantics.
 */
template<>
struct ArgTraits<float>
{
  typedef ValueLike ValueCategory;
};

/**
 * doubles have value-like semantics.
 */
template<>
struct ArgTraits<double>
{
  typedef ValueLike ValueCategory;
};

// ======================================================================
// Other types
// ======================================================================

/**
 * bools have value-like semantics.
 */
template<>
struct ArgTraits<bool>
{
  typedef ValueLike ValueCategory;
};


/**
 * wchar_ts have value-like semantics.
 */
#ifndef TCLAP_DONT_DECLARE_WCHAR_T_ARGTRAITS
template<>
struct ArgTraits<wchar_t>
{
  typedef ValueLike ValueCategory;
};
#endif

/**
 * Strings have string like argument traits.
 */
template<>
struct ArgTraits<std::string>
{
  typedef StringLike ValueCategory;
};

template<typename T>
void SetString(T &dst, const std::string &src)
{
  dst = src;
}

} // namespace

#endif
