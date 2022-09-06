/// @author  Armillus (https://github.com/Armillus)
/// @project Delegate

// Copyright (c) 2022 Armillus.
//
// Permission is hereby  granted, free of charge, to any  person obtaining a copy
// of this software and associated  documentation files (the "Software"), to deal
// in the Software  without restriction, including without  limitation the rights
// to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
// copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
// IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
// FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
// AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
// LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

/* ========================================================================= */
/* Prevent Windows incompatibilities.
/* ------------------------------------------------------------------------- */
/* NOMINMAX is a special macro created to prevent conflicts between 
/* std::max / std::min and old Windows macros max() / min().
/* 
/* While this library doesn't make use of any Windows header, it is defined
/* to make Windows programmers life easier. 
/* ========================================================================= */

#ifndef NOMINMAX
    #define NOMINMAX
#endif // !NOMINMAX


/* ========================================================================= */
/* C++ includes.
/* ========================================================================= */

#include <functional>
#include <string_view>
#include <type_traits>
#include <stdexcept>
#include <algorithm>
#include <cstring>
#include <array>
#include <variant>
#include <tuple>
#include <utility>
#include <format>
#include <memory>
#include <cassert>
#include <source_location>

// +--------------------------------------------------------------------------+
// | All of the code below is a mix of different inspirations such as:        |
// | - Game Engine Gems 3 (Book by Eric Lengyel)                              |
// | - https://github.com/bitwizeshift/Delegate                               |
// | - https://github.com/Neargye/nameof                                      |
// | - My own first version of Delegate                                       |
// +--------------------------------------------------------------------------+


#if defined(__clang__)
    #pragma clang diagnostic push
#endif

/* ========================================================================= */
/* Check if the standard is >= C++ 17.
/* ------------------------------------------------------------------------- */
/* C++ 17 is required because we need: 
/* - std::string_view (compile-time string hashing)
/* - variadic generic lambdas (generic lambdas proxys)
/* - more constexpr possibilities (optimizations)
/* ========================================================================= */

#undef _DELEGATE_CPLUSPLUS_STANDARD

// To understand why MSVC needs a particular behaviour, check this post:
// https://devblogs.microsoft.com/cppblog/msvc-now-correctly-reports-__cplusplus/

#if (defined(_MSVC_LANG))
    #define _DELEGATE_CPLUSPLUS_STANDARD _MSVC_LANG
#else
    #define _DELEGATE_CPLUSPLUS_STANDARD __cplusplus
#endif

static_assert(_DELEGATE_CPLUSPLUS_STANDARD >= 201703L, "Delegate requires at least C++ 17.");


/* ========================================================================= */
/* Check compiler compatibility (Clang >= 5, MSVC >= 15.3, GCC >= 7).
/* ------------------------------------------------------------------------- */
/* We need a compiler supporting C++ 17 and offering a macro to get detailed
/* functions signatures (check next section).
/* ========================================================================= */

#if (defined(__clang__) && __clang_major__ >= 5 || defined(__GNUC__) && __GNUC__ >= 7 || defined(_MSC_VER) && _MSC_VER >= 1910)
    #undef  _DELEGATE_COMPILER_IS_SUPPORTED
    #define _DELEGATE_COMPILER_IS_SUPPORTED 1
#endif

static_assert(
    _DELEGATE_COMPILER_IS_SUPPORTED,
    "Delegate requires a compatible compiler (Clang >= 5, GCC >= 7, MSVC >= 15)."
);


/* ========================================================================= */
/* Set cross-compiler macro to get a detailed function signature.
/* ------------------------------------------------------------------------- */
/* This is needed to properly hash arguments types for generic delegates.
/* ========================================================================= */

#if defined(__clang__) || defined(__GNUC__)
    #define _DELEGATE_FUNCTION_SIGNATURE __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
    #define _DELEGATE_FUNCTION_SIGNATURE __FUNCSIG__
#endif


/* ========================================================================= */
/* Disable unsafe conversion warning on MSVC.
/* ------------------------------------------------------------------------- */
/* This is needed to remove warnings about unsafe 'reinterpret_cast' on 
/* function pointers, since we know what we do and we always convert them
/* back to their original types.
/* ========================================================================= */

#if defined(_MSC_VER)
    #pragma warning(disable: 4191)
#endif


/* ========================================================================= */
/* Forward a list of variadic arguments according to their type
/* ------------------------------------------------------------------------- */
/* This is nothing but a helper used to simplify code reading.
/* You can take a look at this article fore more information:
/* https://www.fluentcpp.com/2019/05/14/3-types-of-macros-that-improve-c-code/
/* ========================================================================= */

#ifndef DELEGATE_FWD
    #define DELEGATE_FWD(...) (std::forward<decltype(__VA_ARGS__)>(__VA_ARGS__))
#endif



#if defined(__clang__) || defined(__GNUC__)
# define DELEGATE_INLINE __attribute__((visibility("hidden"), always_inline))
#elif defined(_MSC_VER)
# define DELEGATE_INLINE __forceinline
#else
# define DELEGATE_INLINE
#endif


#define BOOST_MP11_CONSTEXPR14 constexpr

#if defined( __GNUC__ ) || defined( __clang__ )
# define BOOST_MP11_UNREACHABLE_DEFAULT default: __builtin_unreachable();
#elif defined( _MSC_VER )
# define BOOST_MP11_UNREACHABLE_DEFAULT default: __assume(false);
#else
# define BOOST_MP11_UNREACHABLE_DEFAULT
#endif

namespace boost
{
    namespace mp11
    {
        template<std::size_t N> using mp_size_t = std::integral_constant<std::size_t, N>;

        namespace detail
        {
            template<std::size_t N> using mp_size_t = std::integral_constant<std::size_t, N>;

            template<std::size_t N> struct mp_with_index_impl_
            {
                template<std::size_t Size, std::size_t K, class F, class... InputIndices>
                static BOOST_MP11_CONSTEXPR14 DELEGATE_INLINE auto call(F&& f, std::size_t i, InputIndices&&... indices)
                // template<std::size_t K, class F> static BOOST_MP11_CONSTEXPR14 auto call(std::size_t i, F&& f)
                {
                    if (i < N / 2)
                    {
                        return mp_with_index_impl_<N / 2>::template call<N, K>(std::forward<F>(f), i, DELEGATE_FWD(indices)...);
                    }
                    else
                    {
                        return mp_with_index_impl_<N - N / 2>::template call<N, K + N / 2>(std::forward<F>(f), i - N / 2, DELEGATE_FWD(indices)...);
                    }
                }
            };

            template<std::size_t N, std::size_t K, class F, class... Indices>
            inline BOOST_MP11_CONSTEXPR14 DELEGATE_INLINE auto call(F&& f, Indices&&... indices)
            {
                constexpr bool isComputationDone = std::conjunction_v<std::negation<std::is_integral<Indices>>...>;

                if constexpr (isComputationDone)
                    return std::forward<F>(f)(std::forward<Indices>(indices)...);
                else
                {
                    return []<class T, class... Ts>(F&& f, T&& i, Ts&&... rest) {
                        // assert(i < N);

                        return detail::mp_with_index_impl_<N>::template call<N, K>(std::forward<F>(f), i, DELEGATE_FWD(rest)...);
                    }(std::forward<F>(f), std::forward<Indices>(indices)...);
                }
            }

            template<> struct mp_with_index_impl_<0>
            {
            };

            template<> struct mp_with_index_impl_<1>
            {
                template<std::size_t N, std::size_t K, class F, class... InputIndices>
                static BOOST_MP11_CONSTEXPR14 DELEGATE_INLINE auto call(F&& f, std::size_t, InputIndices&&... indices)
                {
                    return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 0>());
                    // return std::forward<F>(f)(mp_size_t<K + 0>());
                }
            };

            template<> struct mp_with_index_impl_<2>
            {
                template<std::size_t N, std::size_t K, class F, class... InputIndices>
                static BOOST_MP11_CONSTEXPR14 DELEGATE_INLINE auto call(F&& f, std::size_t i, InputIndices&&... indices)
                {
                    switch (i)
                    {
                        BOOST_MP11_UNREACHABLE_DEFAULT
                    case 0: // return std::forward<F>(f)(mp_size_t<K + 0>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 0>());
                    case 1: // return std::forward<F>(f)(mp_size_t<K + 1>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 1>());
                    }
                }
            };

            template<> struct mp_with_index_impl_<3>
            {
                template<std::size_t N, std::size_t K, class F, class... InputIndices>
                static BOOST_MP11_CONSTEXPR14 DELEGATE_INLINE auto call(F&& f, std::size_t i, InputIndices&&... indices)
                {
                    switch (i)
                    {
                        BOOST_MP11_UNREACHABLE_DEFAULT
                    case 0: // return std::forward<F>(f)(mp_size_t<K + 0>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 0>());
                    case 1: // return std::forward<F>(f)(mp_size_t<K + 1>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 1>());
                    case 2: // return std::forward<F>(f)(mp_size_t<K + 2>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 2>());
                    }
                }
            };

            template<> struct mp_with_index_impl_<4>
            {
                template<std::size_t N, std::size_t K, class F, class... InputIndices>
                static BOOST_MP11_CONSTEXPR14 DELEGATE_INLINE auto call(F&& f, std::size_t i, InputIndices&&... indices)
                {
                    switch (i)
                    {
                        BOOST_MP11_UNREACHABLE_DEFAULT
                    case 0:  // return std::forward<F>(f)(mp_size_t<K + 0>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 0>());
                    case 1:  // return std::forward<F>(f)(mp_size_t<K + 1>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 1>());
                    case 2:  // return std::forward<F>(f)(mp_size_t<K + 2>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 2>());
                    case 3:  // return std::forward<F>(f)(mp_size_t<K + 3>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 3>());
                    }
                }
            };

            template<> struct mp_with_index_impl_<5>
            {
                template<std::size_t N, std::size_t K, class F, class... InputIndices>
                static BOOST_MP11_CONSTEXPR14 DELEGATE_INLINE auto call(F&& f, std::size_t i, InputIndices&&... indices)
                {
                    switch (i)
                    {
                        BOOST_MP11_UNREACHABLE_DEFAULT
                    case 0: // return std::forward<F>(f)(mp_size_t<K + 0>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 0>());
                    case 1: // return std::forward<F>(f)(mp_size_t<K + 1>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 1>());
                    case 2: // return std::forward<F>(f)(mp_size_t<K + 2>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 2>());
                    case 3: // return std::forward<F>(f)(mp_size_t<K + 3>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 3>());
                    case 4: // return std::forward<F>(f)(mp_size_t<K + 4>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 4>());
                    }
                }
            };

            template<> struct mp_with_index_impl_<6>
            {
                template<std::size_t N, std::size_t K, class F, class... InputIndices>
                static BOOST_MP11_CONSTEXPR14 DELEGATE_INLINE auto call(F&& f, std::size_t i, InputIndices&&... indices)
                {
                    switch (i)
                    {
                        BOOST_MP11_UNREACHABLE_DEFAULT
                    case 0: // return std::forward<F>(f)(mp_size_t<K + 0>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 0>());
                    case 1: // return std::forward<F>(f)(mp_size_t<K + 1>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 1>());
                    case 2: // return std::forward<F>(f)(mp_size_t<K + 2>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 2>());
                    case 3: // return std::forward<F>(f)(mp_size_t<K + 3>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 3>());
                    case 4: // return std::forward<F>(f)(mp_size_t<K + 4>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 4>());
                    case 5: // return std::forward<F>(f)(mp_size_t<K + 5>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 5>());
                    }
                }
            };

            template<> struct mp_with_index_impl_<7>
            {
                template<std::size_t N, std::size_t K, class F, class... InputIndices>
                static BOOST_MP11_CONSTEXPR14 DELEGATE_INLINE auto call(F&& f, std::size_t i, InputIndices&&... indices)
                {
                    switch (i)
                    {
                        BOOST_MP11_UNREACHABLE_DEFAULT
                    case 0: // return std::forward<F>(f)(mp_size_t<K + 0>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 0>());
                    case 1: // return std::forward<F>(f)(mp_size_t<K + 1>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 1>());
                    case 2: // return std::forward<F>(f)(mp_size_t<K + 2>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 2>());
                    case 3: // return std::forward<F>(f)(mp_size_t<K + 3>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 3>());
                    case 4: // return std::forward<F>(f)(mp_size_t<K + 4>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 4>());
                    case 5: // return std::forward<F>(f)(mp_size_t<K + 5>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 5>());
                    case 6: // return std::forward<F>(f)(mp_size_t<K + 6>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 6>());
                    }
                }
            };

            template<> struct mp_with_index_impl_<8>
            {
                template<std::size_t N, std::size_t K, class F, class... InputIndices>
                static BOOST_MP11_CONSTEXPR14 DELEGATE_INLINE auto call(F&& f, std::size_t i, InputIndices&&... indices)
                {
                    switch (i)
                    {
                        BOOST_MP11_UNREACHABLE_DEFAULT
                    case 0: // return std::forward<F>(f)(mp_size_t<K + 0>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 0>());
                    case 1: // return std::forward<F>(f)(mp_size_t<K + 1>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 1>());
                    case 2: // return std::forward<F>(f)(mp_size_t<K + 2>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 2>());
                    case 3: // return std::forward<F>(f)(mp_size_t<K + 3>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 3>());
                    case 4: // return std::forward<F>(f)(mp_size_t<K + 4>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 4>());
                    case 5: // return std::forward<F>(f)(mp_size_t<K + 5>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 5>());
                    case 6: // return std::forward<F>(f)(mp_size_t<K + 6>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 6>());
                    case 7: // return std::forward<F>(f)(mp_size_t<K + 7>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 7>());
                    }
                }
            };

            template<> struct mp_with_index_impl_<9>
            {
                template<std::size_t N, std::size_t K, class F, class... InputIndices>
                static BOOST_MP11_CONSTEXPR14 DELEGATE_INLINE auto call(F&& f, std::size_t i, InputIndices&&... indices)
                {
                    switch (i)
                    {
                        BOOST_MP11_UNREACHABLE_DEFAULT
                    case 0: // return std::forward<F>(f)(mp_size_t<K + 0>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 0>());
                    case 1: // return std::forward<F>(f)(mp_size_t<K + 1>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 1>());
                    case 2: // return std::forward<F>(f)(mp_size_t<K + 2>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 2>());
                    case 3: // return std::forward<F>(f)(mp_size_t<K + 3>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 3>());
                    case 4: // return std::forward<F>(f)(mp_size_t<K + 4>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 4>());
                    case 5: // return std::forward<F>(f)(mp_size_t<K + 5>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 5>());
                    case 6: // return std::forward<F>(f)(mp_size_t<K + 6>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 6>());
                    case 7: // return std::forward<F>(f)(mp_size_t<K + 7>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 7>());
                    case 8: // return std::forward<F>(f)(mp_size_t<K + 8>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 8>());
                    }
                }
            };

            template<> struct mp_with_index_impl_<10>
            {
                template<std::size_t N, std::size_t K, class F, class... InputIndices>
                static BOOST_MP11_CONSTEXPR14 DELEGATE_INLINE auto call(F&& f, std::size_t i, InputIndices&&... indices)
                {
                    switch (i)
                    {
                        BOOST_MP11_UNREACHABLE_DEFAULT
                    case 0: // return std::forward<F>(f)(mp_size_t<K + 0>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 0>());
                    case 1: // return std::forward<F>(f)(mp_size_t<K + 1>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 1>());
                    case 2: // return std::forward<F>(f)(mp_size_t<K + 2>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 2>());
                    case 3: // return std::forward<F>(f)(mp_size_t<K + 3>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 3>());
                    case 4: // return std::forward<F>(f)(mp_size_t<K + 4>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 4>());
                    case 5: // return std::forward<F>(f)(mp_size_t<K + 5>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 5>());
                    case 6: // return std::forward<F>(f)(mp_size_t<K + 6>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 6>());
                    case 7: // return std::forward<F>(f)(mp_size_t<K + 7>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 7>());
                    case 8: // return std::forward<F>(f)(mp_size_t<K + 8>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 8>());
                    case 9: // return std::forward<F>(f)(mp_size_t<K + 9>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 9>());
                    }
                }
            };

            template<> struct mp_with_index_impl_<11>
            {
                template<std::size_t N, std::size_t K, class F, class... InputIndices>
                static BOOST_MP11_CONSTEXPR14 DELEGATE_INLINE auto call(F&& f, std::size_t i, InputIndices&&... indices)
                {
                    switch (i)
                    {
                        BOOST_MP11_UNREACHABLE_DEFAULT
                    case 0: // return std::forward<F>(f)(mp_size_t<K + 0>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 0>());
                    case 1: // return std::forward<F>(f)(mp_size_t<K + 1>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 1>());
                    case 2: // return std::forward<F>(f)(mp_size_t<K + 2>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 2>());
                    case 3: // return std::forward<F>(f)(mp_size_t<K + 3>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 3>());
                    case 4: // return std::forward<F>(f)(mp_size_t<K + 4>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 4>());
                    case 5: // return std::forward<F>(f)(mp_size_t<K + 5>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 5>());
                    case 6: // return std::forward<F>(f)(mp_size_t<K + 6>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 6>());
                    case 7: // return std::forward<F>(f)(mp_size_t<K + 7>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 7>());
                    case 8: // return std::forward<F>(f)(mp_size_t<K + 8>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 8>());
                    case 9: // return std::forward<F>(f)(mp_size_t<K + 9>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 9>());
                    case 10: //return std::forward<F>(f)(mp_size_t<K + 10>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 10>());
                    }
                }
            };

            template<> struct mp_with_index_impl_<12>
            {
                template<std::size_t N, std::size_t K, class F, class... InputIndices>
                static BOOST_MP11_CONSTEXPR14 DELEGATE_INLINE auto call(F&& f, std::size_t i, InputIndices&&... indices)
                {
                    switch (i)
                    {
                        BOOST_MP11_UNREACHABLE_DEFAULT
                    case 0: //return std::forward<F>(f)(mp_size_t<K + 0>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 0>());
                    case 1: //return std::forward<F>(f)(mp_size_t<K + 1>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 1>());
                    case 2: //return std::forward<F>(f)(mp_size_t<K + 2>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 2>());
                    case 3: //return std::forward<F>(f)(mp_size_t<K + 3>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 3>());
                    case 4: //return std::forward<F>(f)(mp_size_t<K + 4>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 4>());
                    case 5: //return std::forward<F>(f)(mp_size_t<K + 5>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 5>());
                    case 6: //return std::forward<F>(f)(mp_size_t<K + 6>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 6>());
                    case 7: //return std::forward<F>(f)(mp_size_t<K + 7>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 7>());
                    case 8: //return std::forward<F>(f)(mp_size_t<K + 8>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 8>());
                    case 9: //return std::forward<F>(f)(mp_size_t<K + 9>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 9>());
                    case 10: // return std::forward<F>(f)(mp_size_t<K + 10>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 10>());
                    case 11: // return std::forward<F>(f)(mp_size_t<K + 11>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 11>());
                    }
                }
            };

            template<> struct mp_with_index_impl_<13>
            {
                template<std::size_t N, std::size_t K, class F, class... InputIndices>
                static BOOST_MP11_CONSTEXPR14 DELEGATE_INLINE auto call(F&& f, std::size_t i, InputIndices&&... indices)
                {
                    switch (i)
                    {
                        BOOST_MP11_UNREACHABLE_DEFAULT
                    case 0: // return std::forward<F>(f)(mp_size_t<K + 0>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 0>());
                    case 1: // return std::forward<F>(f)(mp_size_t<K + 1>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 1>());
                    case 2: // return std::forward<F>(f)(mp_size_t<K + 2>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 2>());
                    case 3: // return std::forward<F>(f)(mp_size_t<K + 3>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 3>());
                    case 4: // return std::forward<F>(f)(mp_size_t<K + 4>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 4>());
                    case 5: // return std::forward<F>(f)(mp_size_t<K + 5>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 5>());
                    case 6: // return std::forward<F>(f)(mp_size_t<K + 6>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 6>());
                    case 7: // return std::forward<F>(f)(mp_size_t<K + 7>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 7>());
                    case 8: // return std::forward<F>(f)(mp_size_t<K + 8>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 8>());
                    case 9: // return std::forward<F>(f)(mp_size_t<K + 9>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 9>());
                    case 10: // return std::forward<F>(f)(mp_size_t<K + 10>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 10>());
                    case 11: // return std::forward<F>(f)(mp_size_t<K + 11>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 11>());
                    case 12: // return std::forward<F>(f)(mp_size_t<K + 12>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 12>());
                    }
                }
            };

            template<> struct mp_with_index_impl_<14>
            {
                template<std::size_t N, std::size_t K, class F, class... InputIndices>
                static BOOST_MP11_CONSTEXPR14 DELEGATE_INLINE auto call(F&& f, std::size_t i, InputIndices&&... indices)
                {
                    switch (i)
                    {
                        BOOST_MP11_UNREACHABLE_DEFAULT
                    case 0: //return std::forward<F>(f)(mp_size_t<K + 0>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 0>());
                    case 1: //return std::forward<F>(f)(mp_size_t<K + 1>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 1>());
                    case 2: //return std::forward<F>(f)(mp_size_t<K + 2>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 2>());
                    case 3: //return std::forward<F>(f)(mp_size_t<K + 3>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 3>());
                    case 4: //return std::forward<F>(f)(mp_size_t<K + 4>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 4>());
                    case 5: //return std::forward<F>(f)(mp_size_t<K + 5>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 5>());
                    case 6: //return std::forward<F>(f)(mp_size_t<K + 6>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 6>());
                    case 7: //return std::forward<F>(f)(mp_size_t<K + 7>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 7>());
                    case 8: //return std::forward<F>(f)(mp_size_t<K + 8>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 8>());
                    case 9: //return std::forward<F>(f)(mp_size_t<K + 9>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 9>());
                    case 10: // return std::forward<F>(f)(mp_size_t<K + 10>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 10>());
                    case 11: // return std::forward<F>(f)(mp_size_t<K + 11>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 11>());
                    case 12: // return std::forward<F>(f)(mp_size_t<K + 12>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 12>());
                    case 13: // return std::forward<F>(f)(mp_size_t<K + 13>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 13>());
                    }
                }
            };

            template<> struct mp_with_index_impl_<15>
            {
                template<std::size_t N, std::size_t K, class F, class... InputIndices>
                static BOOST_MP11_CONSTEXPR14 DELEGATE_INLINE auto call(F&& f, std::size_t i, InputIndices&&... indices)
                {
                    switch (i)
                    {
                        BOOST_MP11_UNREACHABLE_DEFAULT
                    case 0: // return std::forward<F>(f)(mp_size_t<K + 0>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 0>());
                    case 1: // return std::forward<F>(f)(mp_size_t<K + 1>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 1>());
                    case 2: // return std::forward<F>(f)(mp_size_t<K + 2>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 2>());
                    case 3: // return std::forward<F>(f)(mp_size_t<K + 3>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 3>());
                    case 4: // return std::forward<F>(f)(mp_size_t<K + 4>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 4>());
                    case 5: // return std::forward<F>(f)(mp_size_t<K + 5>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 5>());
                    case 6: // return std::forward<F>(f)(mp_size_t<K + 6>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 6>());
                    case 7: // return std::forward<F>(f)(mp_size_t<K + 7>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 7>());
                    case 8: // return std::forward<F>(f)(mp_size_t<K + 8>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 8>());
                    case 9: // return std::forward<F>(f)(mp_size_t<K + 9>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 9>());
                    case 10: // return std::forward<F>(f)(mp_size_t<K + 10>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 10>());
                    case 11: // return std::forward<F>(f)(mp_size_t<K + 11>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 11>());
                    case 12: // return std::forward<F>(f)(mp_size_t<K + 12>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 12>());
                    case 13: // return std::forward<F>(f)(mp_size_t<K + 13>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 13>());
                    case 14: // return std::forward<F>(f)(mp_size_t<K + 14>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 14>());
                    }
                }
            };

            template<> struct mp_with_index_impl_<16>
            {
                template<std::size_t N, std::size_t K, class F, class... InputIndices>
                static BOOST_MP11_CONSTEXPR14 DELEGATE_INLINE auto call(F&& f, std::size_t i, InputIndices&&... indices)
                {
                    switch (i)
                    {
                        BOOST_MP11_UNREACHABLE_DEFAULT
                    case 0: // return std::forward<F>(f)(mp_size_t<K + 0>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 0>());
                    case 1: // return std::forward<F>(f)(mp_size_t<K + 1>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 1>());
                    case 2: // return std::forward<F>(f)(mp_size_t<K + 2>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 2>());
                    case 3: // return std::forward<F>(f)(mp_size_t<K + 3>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 3>());
                    case 4: // return std::forward<F>(f)(mp_size_t<K + 4>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 4>());
                    case 5: // return std::forward<F>(f)(mp_size_t<K + 5>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 5>());
                    case 6: // return std::forward<F>(f)(mp_size_t<K + 6>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 6>());
                    case 7: // return std::forward<F>(f)(mp_size_t<K + 7>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 7>());
                    case 8: // return std::forward<F>(f)(mp_size_t<K + 8>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 8>());
                    case 9: // return std::forward<F>(f)(mp_size_t<K + 9>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 9>());
                    case 10: // return std::forward<F>(f)(mp_size_t<K + 10>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 10>());
                    case 11: // return std::forward<F>(f)(mp_size_t<K + 11>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 11>());
                    case 12: // return std::forward<F>(f)(mp_size_t<K + 12>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 12>());
                    case 13: // return std::forward<F>(f)(mp_size_t<K + 13>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 13>());
                    case 14: // return std::forward<F>(f)(mp_size_t<K + 14>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 14>());
                    case 15: // return std::forward<F>(f)(mp_size_t<K + 15>());
                        return detail::template call<N, K>(std::forward<F>(f), DELEGATE_FWD(indices)..., mp_size_t<K + 15>());
                    }
                }
            };
        } // namespace detail

        template<std::size_t N, class F, class... Indices>
        inline BOOST_MP11_CONSTEXPR14 DELEGATE_INLINE auto mp_with_index(F&& f, Indices&&... indices)
        {
            if constexpr (sizeof...(Indices) == 0u)
                return std::forward<F>(f)();
            else
            {
                // const auto inIndices = std::make_tuple(std::forward<Indices>(indices)...);

                return detail::template call<N, 0>(std::forward<F>(f), std::forward<Indices>(indices)...);
            }
        }

        //template<class N, class F, std::unsigned_integral... Indices>
        //inline BOOST_MP11_CONSTEXPR14 auto mp_with_index(F&& f, Indices&&... indices)
        //{
        //    return mp_with_index<std::size_t { N::value }>(std::forward<F>(f), std::forward<Indices>(indices)...);
        //}

#undef BOOST_MP11_CONSTEXPR14
#undef BOOST_MP11_UNREACHABLE_DEFAULT

    } // namespace mp11
} // namespace boost

namespace axl
{
    namespace detail
    {
        /* ============================================================= */
        /* FixedString
        /* ============================================================= */
        
        template<std::size_t Size>
        class FixedString
        {
        public:
            consteval FixedString() noexcept = default;
            
            consteval FixedString(std::string_view str) noexcept
            {
                std::copy_n(str.begin(), str.size(), _buffer.begin());
            }

            consteval FixedString(const char (&str)[Size + 1]) noexcept
            {
                std::copy(std::begin(str), std::end(str), _buffer.begin());
            }

            /* ------------------------------------------------------------- */
            /* Conversions
            /* ------------------------------------------------------------- */
            consteval operator std::string_view() const noexcept { return *this; }

            /* ------------------------------------------------------------- */
            /* Properties
            /* ------------------------------------------------------------- */

            [[nodiscard]] consteval bool empty()  const noexcept { return Size == 0;      }
            [[nodiscard]] consteval auto size()   const noexcept { return Size;           }
            [[nodiscard]] consteval auto length() const noexcept { return Size;           }
            [[nodiscard]] consteval auto data()   const noexcept { return _buffer.data(); }

            /* ------------------------------------------------------------- */
            /* String matching
            /* ------------------------------------------------------------- */

            [[nodiscard]] consteval bool endsWith(std::string_view suffix) const noexcept { return sv().ends_with(suffix); }
            [[nodiscard]] consteval bool endsWith(const char* suffix)      const noexcept { return sv().ends_with(suffix); }
            [[nodiscard]] consteval bool endsWith(const char suffix)       const noexcept { return sv().ends_with(suffix); }

            [[nodiscard]] consteval bool startsWith(std::string_view prefix) const noexcept { return sv().starts_with(prefix); }
            [[nodiscard]] consteval bool startsWith(const char* prefix)      const noexcept { return sv().starts_with(prefix); }
            [[nodiscard]] consteval bool startsWith(const char prefix)       const noexcept { return sv().starts_with(prefix); }

            //template<std::size_t Len>
            //[[nodiscard]] constexpr auto find(const same_with_other_size<Len>& str, size_type pos = 0) const noexcept
            //{
            //    if constexpr (Len > Size)
            //        return std::string_view::npos;

            //    return sv().find(str.sv(), pos);
            //}

            //[[nodiscard]] constexpr auto find(std::string_view sv, std::size_t pos = 0)      const noexcept { return sv().find(sv, pos);   }
            //[[nodiscard]] constexpr auto find(const char* s, std::size_t pos, std::size_t n) const          { return sv().find(s, pos, n); }
            //[[nodiscard]] constexpr auto find(const char* s, std::size_t pos = 0)            const          { return sv().find(s, pos);    }
            //[[nodiscard]] constexpr auto find(char c, std::size_t pos = 0)                   const noexcept { return sv().find(c, pos);    }

        private:
            /* ------------------------------------------------------------- */
            /* Internal helpers
            /* ------------------------------------------------------------- */

            consteval std::string_view sv() const noexcept { return { _buffer.data(), Size + 1 }; }

        private:
            std::array<char, Size + 1> _buffer {};
        };

        /* ------------------------------------------------------------- */
        /* Deduction guide to build FixedStrings without the usual '\0'
        /* at the end of the string.
        /* ------------------------------------------------------------- */

        template<std::size_t Size>
        FixedString(const char(&)[Size]) -> FixedString<Size - 1>;

        /* ============================================================= */
        /* Compile-time string hashing
        /* ============================================================= */

        template<std::size_t N>
        [[nodiscard]]
        constexpr DELEGATE_INLINE std::uint32_t hash(std::uint32_t prime, const char (&s)[N], std::size_t len = N - 1) noexcept
        {
            // Simple recursive Horner hash (may fail on Clang if 's' is too long)
            return (len <= 1) ? s[0] : (prime * hash(prime, s, len - 1) + s[len - 1]);
        }

        [[nodiscard]]
        constexpr DELEGATE_INLINE std::uint32_t hash(std::uint32_t prime, const char* s, std::size_t len) noexcept
        {
            std::uint32_t hash = 0;
            
            // Simple Horner hash
            for (std::uint32_t i = 0; i < len; ++i)
                hash = prime * hash + s[i];
            
            return hash;
        }

        [[nodiscard]]
        constexpr DELEGATE_INLINE std::uint32_t hash(std::uint32_t prime, const std::string_view s) noexcept
        {
            return hash(prime, s.data(), s.size());
        }

        [[nodiscard]]
        constexpr DELEGATE_INLINE std::uint32_t hash(const std::string_view s) noexcept
        {
            constexpr std::size_t defaultPrimeNumber = 31;

            return hash(defaultPrimeNumber, s);
        }

        /* ============================================================= */
        /* Function reflexion
        /* ============================================================= */

        constexpr auto prettifyName(std::string_view s) noexcept -> std::string_view
        {
            std::size_t start = 0;
            std::size_t end   = s.find_last_of('>');
            
            if (end == std::string_view::npos)
                return {};  // Unsupported type name

            for (std::size_t i = end - 1, tokens = 0; i > 0; --i)
            {
                // Roll back to the original template opening token ('<')
                if (s[i] == '>')
                    tokens++;
                else if (s[i] == '<')
                {
                    if (tokens)
                        tokens--;
                    else
                    {
                        start = i + 1;
                        break;
                    }
                }
            }

            return s.substr(start, end - start);
        }

        template<class T>
        constexpr DELEGATE_INLINE auto typeName() noexcept -> std::string_view
        {
            // Inspired from nameof, check it out here:
            // https://github.com/Neargye/nameof

            constexpr auto name = prettifyName(_DELEGATE_FUNCTION_SIGNATURE);

            return name;
        }

        template<class R, class... Args>
        constexpr DELEGATE_INLINE auto hashSignature() noexcept -> std::uint32_t
        {
            using F = R(*)(Args...);
            constexpr auto hashedSignature = hash(typeName<F>());

            return hashedSignature;
        }
    } // !namespace detail

    template<class T>
    struct ReflectedArg
    {
        template<class U>
        requires std::is_same_v<std::decay_t<T>, std::decay_t<U>>
        constexpr DELEGATE_INLINE ReflectedArg(U&& parameter) noexcept
            : arg { parameter }
        {
            if constexpr (std::is_const_v<U>)
                isConst = true;

            //if constexpr (std::is_volatile_v<U>)
            //    isVolatile = true;
        }

        template<class U>
        requires std::is_same_v<std::decay_t<T>, std::decay_t<U>>
        constexpr DELEGATE_INLINE auto forward() const noexcept -> U
        {
            if constexpr (!std::is_const_v<U>)
            {
                if (isConst) throw std::exception();
                
            }
            //if constexpr (!std::is_volatile_v<U>)
            //{
            //    if (isVolatile) throw std::exception();
            //}

            else if constexpr (std::is_lvalue_reference_v<U>)
            {
                return const_cast<U>(arg);
            }
            else
            {
                return std::move(arg);
            }
        }

        const /*volatile*/ T& arg;
        bool isConst = false;
        // bool isVolatile = false;
    };

    namespace traits
    {
        /* ============================================================= */
        /* is_object
        /* ============================================================= */

        template<class F>
        struct is_object : std::bool_constant<(
            std::is_object_v<std::remove_pointer_t<std::remove_reference_t<F>>>
        )>
        {};

        template<class F>
        inline constexpr bool is_object_v = is_object<F>::value;

        /* ============================================================= */
        /* is_function
        /* ============================================================= */

        template<class F>
        struct is_function : std::bool_constant<(
            std::is_function_v<std::remove_pointer_t<std::remove_reference_t<F>>>
        )>
        {};

        template<class F>
        inline constexpr bool is_function_v = is_function<F>::value;

        /* ============================================================= */
        /* is_function_pointer
        /* ============================================================= */

        template<class F>
        struct is_function_pointer : std::bool_constant<(
            std::is_pointer_v<F> &&
            is_function_v<F>
        )>
        {};

        template<class F>
        inline constexpr bool is_function_pointer_v = is_function_pointer<F>::value;

        /* ============================================================= */
        /* is_equality_comparable
        /* ============================================================= */

        template<class T, class U>
        using equality_comparison_t = decltype(std::declval<T&>() == std::declval<U&>());

        template<class T, class U, typename = std::void_t<>>
        struct is_equality_comparable
            : std::false_type
        {};

        template<class T, class U>
        struct is_equality_comparable<T, U, std::void_t<equality_comparison_t<T, U>>>
            : std::is_same<equality_comparison_t<T, U>, bool>
        {};

        template<class T, class U = T>
        inline constexpr bool is_equality_comparable_v = is_equality_comparable<T, U>::value;

        /* ============================================================= */
        /* function_type_impl
        /* ============================================================= */

        template<class F>
        struct function_type_impl;

        /* ------------------------------------------------------------- */
        /* Function pointers
        /* ------------------------------------------------------------- */

        template<class R, class... Args>
        struct function_type_impl<R(*)(Args...)>
        {
            static constexpr auto hash = detail::hashSignature<R, std::add_rvalue_reference<Args>...>();

            using decayed_args = std::tuple<std::decay_t<Args>...>;

            using args    = std::tuple<Args...>;
            using type    = R(Args...);
            using pointer = R(*)(Args...);

            template<class Delegate>
            using proxy = R(*)(const Delegate*, ReflectedArg<std::decay_t<Args>>&&...);
        };

        template<class R, class... Args>
        struct function_type_impl<R(*)(Args..., ...)>
        {
            static constexpr auto hash = detail::hashSignature<R, std::add_rvalue_reference<Args>...>();
            
            using decayed_args = std::tuple<std::decay_t<Args>...>;

            using args    = std::tuple<Args...>;
            using type    = R(Args..., ...);
            using pointer = R(*)(Args..., ...);
                
            template<class Delegate>
            using proxy = R(*)(const Delegate*, ReflectedArg<std::decay_t<Args>>&&..., ...);
        };

        template<class R, class... Args>
        struct function_type_impl<R(*)(Args...) noexcept>
        {
            static constexpr auto hash = detail::hashSignature<R, std::add_rvalue_reference<Args>...>();
            
            using decayed_args = std::tuple<std::decay_t<Args>...>;

            using args    = std::tuple<Args...>;
            using type    = R(Args...);
            using pointer = R(*)(Args...);
                
            template<class Delegate>
            using proxy = R(*)(const Delegate*, ReflectedArg<std::decay_t<Args>>&&...);
        };

        template<class R, class... Args>
        struct function_type_impl<R(*)(Args..., ...) noexcept>
        {
            static constexpr auto hash = detail::hashSignature<R, std::add_rvalue_reference<Args>...>();

            using decayed_args = std::tuple<std::decay_t<Args>...>;

            using args    = std::tuple<Args...>;
            using type    = R(Args..., ...);
            using pointer = R(*)(Args..., ...);
                
            template<class Delegate>
            using proxy = R(*)(const Delegate*, ReflectedArg<std::decay_t<Args>>&&..., ...);
        };

        /* ------------------------------------------------------------- */
        /* Member function pointers
        /* ------------------------------------------------------------- */

        template<class R, class C, class... Args>
        struct function_type_impl<R(C::*)(Args...)>
        {
            static constexpr auto hash = detail::hashSignature<R, std::add_rvalue_reference<Args>...>();

            using decayed_args = std::tuple<std::decay_t<Args>...>;

            using args    = std::tuple<Args...>;
            using type    = R(Args...);
            using pointer = R(*)(Args...);
                
            template<class Delegate>
            using proxy = R(*)(const Delegate*, ReflectedArg<std::decay_t<Args>>&&...);
        };

        template<class R, class C, class... Args>
        struct function_type_impl<R(C::*)(Args..., ...)>
        {
            static constexpr auto hash = detail::hashSignature<R, std::add_rvalue_reference<Args>...>();

            using decayed_args = std::tuple<std::decay_t<Args>...>;

            using args    = std::tuple<Args...>;
            using type    = R(Args..., ...);
            using pointer = R(*)(Args..., ...);
                
            template<class Delegate>
            using proxy = R(*)(const Delegate*, ReflectedArg<std::decay_t<Args>>&&..., ...);
        };

        template<class R, class C, class... Args>
        struct function_type_impl<R(C::*)(Args...) noexcept>
        {
            static constexpr auto hash = detail::hashSignature<R, std::add_rvalue_reference<Args>...>();

            using decayed_args = std::tuple<std::decay_t<Args>...>;

            using args    = std::tuple<Args...>;
            using type    = R(Args...);
            using pointer = R(*)(Args...);
                
            template<class Delegate>
            using proxy = R(*)(const Delegate*, ReflectedArg<std::decay_t<Args>>&&...);
        };

        template<class R, class C, class... Args>
        struct function_type_impl<R(C::*)(Args..., ...) noexcept>
        {
            static constexpr auto hash = detail::hashSignature<R, std::add_rvalue_reference<Args>...>();

            using decayed_args = std::tuple<std::decay_t<Args>...>;

            using args    = std::tuple<Args...>;
            using type    = R(Args..., ...);
            using pointer = R(*)(Args..., ...);
                
            template<class Delegate>
            using proxy = R(*)(const Delegate*, ReflectedArg<std::decay_t<Args>>&&..., ...);
        };

        /* ------------------------------------------------------------- */
        /* Const member function pointers
        /* ------------------------------------------------------------- */

        template<class R, class C, class... Args>
        struct function_type_impl<R(C::*)(Args...) const>
        {
            static constexpr auto hash = detail::hashSignature<R, std::add_rvalue_reference<Args>...>();

            using decayed_args = std::tuple<std::decay_t<Args>...>;

            using args    = std::tuple<Args...>;
            using type    = R(Args...);
            using pointer = R(*)(Args...);
                
            template<class Delegate>
            using proxy = R(*)(const Delegate*, ReflectedArg<std::decay_t<Args>>&&...);
        };

        template<class R, class C, class... Args>
        struct function_type_impl<R(C::*)(Args..., ...) const>
        {
            static constexpr auto hash = detail::hashSignature<R, std::add_rvalue_reference<Args>...>();

            using decayed_args = std::tuple<std::decay_t<Args>...>;

            using args    = std::tuple<Args...>;
            using type    = R(Args..., ...);
            using pointer = R(*)(Args..., ...);
                
            template<class Delegate>
            using proxy = R(*)(const Delegate*, ReflectedArg<std::decay_t<Args>>&&..., ...);
        };

        template<class R, class C, class... Args>
        struct function_type_impl<R(C::*)(Args...) const noexcept>
        {
            static constexpr auto hash = detail::hashSignature<R, std::add_rvalue_reference<Args>...>();

            using decayed_args = std::tuple<std::decay_t<Args>...>;

            using args    = std::tuple<Args...>;
            using type    = R(Args...);
            using pointer = R(*)(Args...);
                
            template<class Delegate>
            using proxy = R(*)(const Delegate*, ReflectedArg<std::decay_t<Args>>&&...);
        };

        template<class R, class C, class... Args>
        struct function_type_impl<R(C::*)(Args..., ...) const noexcept>
        {
            static constexpr auto hash = detail::hashSignature<R, std::add_rvalue_reference<Args>...>();

            using decayed_args = std::tuple<std::decay_t<Args>...>;

            using args    = std::tuple<Args...>;
            using type    = R(Args..., ...);
            using pointer = R(*)(Args..., ...);
                
            template<class Delegate>
            using proxy = R(*)(const Delegate*, ReflectedArg<std::decay_t<Args>>&&..., ...);
        };

        /* ============================================================= */
        /* function_type
        /* ============================================================= */

        template<class F, class Enable = void>
        struct function_type : function_type_impl<F> {};

        template<class F>
        struct function_type<F, typename std::enable_if_t<!is_function_pointer_v<F>>>
            : function_type_impl<decltype(&F::operator())>
        {};

        template<class F>
        using function_signature = typename function_type<F>::type;

        template<class F>
        using function_args = typename function_type<F>::args;

        template<class F>
        using function_decayed_args = typename function_type<F>::decayed_args;

        template<class F>
        using function_pointer_t = typename function_type<F>::pointer;

        // https://stackoverflow.com/questions/54126204/what-is-the-need-of-template-lambda-introduced-in-c20-when-c14-already-has-g
        // https://stackoverflow.com/questions/20063184/template-alias-for-template-member-class
        // https://stackoverflow.com/questions/610245/where-and-why-do-i-have-to-put-the-template-and-typename-keywords

        template<class F, class Delegate>
        using delegate_proxy_t = typename function_type<F>::template proxy<Delegate>;

        template<class F>
        inline constexpr auto function_hash = function_type<F>::hash;

        /* ============================================================= */
        /* is_callable_impl
        /* ============================================================= */

        // Inspired from:
        // https://stackoverflow.com/questions/15393938/find-out-whether-a-c-object-is-callable
        // https://github.com/boostorg/type_traits/blob/develop/include/boost/type_traits/detail/is_likely_lambda.hpp

        template<class F, bool IsObject = is_object_v<F>>
        struct is_callable_impl : is_function<F> {};

        template<class F>
        struct is_callable_impl<F, true>
        {
        private:
            struct Fallback { void operator()() {}; };
            struct Derived : F, Fallback
            {
                constexpr Derived() noexcept = delete;
                ~Derived() noexcept = default;
                constexpr Derived(const Derived&) noexcept = delete;
                constexpr Derived(Derived&&) noexcept = delete;
                constexpr void operator=(const Derived&) noexcept = delete;
                constexpr void operator=(Derived&&) noexcept = delete;
            };

            template<class U, U> struct Check;

            template<class>
            static std::true_type test(...);

            template<class C>
            static std::false_type test(Check<void (Fallback::*)(), &C::operator()>*);

            using IsCallable = decltype(test<Derived>(nullptr));
            
        public:
            static constexpr bool value = std::is_same_v<IsCallable, std::true_type>;
        };

        /* ============================================================= */
        /* is_callable
        /* ============================================================= */

        template<class F>
        struct is_callable : is_callable_impl<F> {};

        template<class F>
        inline constexpr bool is_callable_v = is_callable<F>::value;

        /* ============================================================= */
        /* is_likely_lambda
        /* ============================================================= */

        template<class F>
        struct is_likely_lambda : std::bool_constant<(
            is_object_v<F> &&
            is_callable_v<F> &&
            !std::has_unique_object_representations_v<F> &&
            !std::is_assignable_v<F, F> &&
            std::is_constructible_v<std::decay_t<F>, F> &&
            std::is_trivially_destructible_v<std::decay_t<F>> &&
            std::is_trivially_copyable_v<std::decay_t<F>>
        )>
        {};

        template<class F>
        inline constexpr bool is_likely_lambda_v = is_likely_lambda<F>::value;

        /* ============================================================= */
        /* is_likely_captureless_lambda
        /* ============================================================= */

        template<class F, class Enable = void>
        struct is_likely_captureless_lambda : std::bool_constant<(
            std::is_empty_v<std::decay_t<F>> &&
            is_likely_lambda_v<F> &&
            std::is_convertible_v<F, function_pointer_t<F>>
        )>
        {};

        template<class F>
        struct is_likely_captureless_lambda<F, typename std::enable_if_t<!is_callable_v<F>>>
            : std::false_type
        {};

        template<class F>
        inline constexpr bool is_likely_captureless_lambda_v = is_likely_captureless_lambda<F>::value;

        /* ============================================================= */
        /* is_likely_capturing_lambda
        /* ============================================================= */

        template<class F, class Enable = void>
        struct is_likely_capturing_lambda : std::bool_constant<(
            !std::is_empty_v<std::decay_t<F>> &&
            is_likely_lambda_v<F> &&
            !std::is_convertible_v<F, function_pointer_t<F>>
        )>
        {};

        template<class F>
        struct is_likely_capturing_lambda<F, typename std::enable_if_t<!is_callable_v<F>>>
            : std::false_type
        {};

        template<class F>
        inline constexpr bool is_likely_capturing_lambda_v = is_likely_capturing_lambda<F>::value;
    } // !namespace traits

    namespace detail
    {
        // More on dedicated / retrospective casts for lambdas:
        // https://www.py4u.net/discuss/63381
        // https://stackoverflow.com/questions/13358672/how-to-convert-a-lambda-to-an-stdfunction-using-templates
        // https://gist.github.com/khvorov/cd626ea3685fd5e8bf14

        template<class F>
        constexpr DELEGATE_INLINE decltype(auto) retrospective_cast(F&& callable) noexcept
        {
            if constexpr (traits::is_likely_captureless_lambda_v<F>)
            {
                return static_cast<traits::function_pointer_t<F>>(callable);
            }
            else
            {
                // Capturing lambdas can't be converted into classic function pointers
                return std::forward<F>(callable);
            }
        }
    } // !namespace detail

    enum ArgumentState : std::size_t
    {
        IsOriginal = 0x00,
        IsLValue = 0x01,
        IsRValue = 0x02,
        IsVolatile = 0x04,
        IsConst = 0x08,
    };

    template<ArgumentState State>
    struct Argument
    {
        static constexpr bool is_original = static_cast<bool>(State & ArgumentState::IsOriginal);
        static constexpr bool is_lvalue   = static_cast<bool>(State & ArgumentState::IsLValue);
        static constexpr bool is_rvalue   = static_cast<bool>(State & ArgumentState::IsRValue);
        static constexpr bool is_const    = static_cast<bool>(State & ArgumentState::IsConst);
        static constexpr bool is_volatile = static_cast<bool>(State & ArgumentState::IsVolatile);
    };

    using FixedArgument = std::variant<
        Argument<IsOriginal>,
        Argument<static_cast<ArgumentState>(IsOriginal | IsConst)>,
        Argument<static_cast<ArgumentState>(IsOriginal | IsVolatile)>,
        Argument<static_cast<ArgumentState>(IsOriginal | IsConst | IsVolatile)>,

        Argument<IsLValue>,
        Argument<static_cast<ArgumentState>(IsLValue | IsConst)>,
        Argument<static_cast<ArgumentState>(IsLValue | IsVolatile)>,
        Argument<static_cast<ArgumentState>(IsLValue | IsConst | IsVolatile)>,

        Argument<IsRValue>,
        Argument<static_cast<ArgumentState>(IsRValue | IsConst)>,
        Argument<static_cast<ArgumentState>(IsRValue | IsVolatile)>,
        Argument<static_cast<ArgumentState>(IsRValue | IsConst | IsVolatile)>
    >;

        struct FunctionArgument
        {
            template<class Arg>
            static constexpr auto fromConcreteArg() noexcept -> FunctionArgument
            {
                return {
                    std::is_const_v<Arg>,
                    std::is_volatile_v<Arg>,
                    std::is_rvalue_reference_v<Arg>,
                    std::is_lvalue_reference_v<Arg>
                };
            }

            constexpr FunctionArgument() noexcept = default;

            constexpr FunctionArgument(std::string_view representation) noexcept
                : isConst    { representation.find("const ") != std::string_view::npos           }
                , isVolatile { representation.find("volatile ") != std::string_view::npos        }
                , isRValue   { representation.find("&&") != std::string_view::npos               }
                , isLValue   { !isRValue && representation.find("&") != std::string_view::npos   }
            {
            
            }

            constexpr FunctionArgument(bool isConst, bool isVolatile, bool isLValue, bool isRValue) noexcept
                : isConst    { isConst    }
                , isVolatile { isVolatile }
                , isRValue   { isRValue   }
                , isLValue   { isLValue   }
            {}

            operator FixedArgument() const noexcept
            {
                if (isLValue)
                {
                    if (isConst)
                    {
                        if (isVolatile)
                            return FixedArgument(std::in_place_type<Argument<static_cast<ArgumentState>(IsLValue | IsConst | IsVolatile)>>);
                        
                        return FixedArgument(std::in_place_type<Argument<static_cast<ArgumentState>(IsLValue | IsConst)>>);
                    }
                    return FixedArgument(std::in_place_type<Argument<static_cast<ArgumentState>(IsLValue)>>);
                }
                if (isRValue)
                {
                    if (isConst)
                    {
                        if (isVolatile)
                            return FixedArgument(std::in_place_type<Argument<static_cast<ArgumentState>(IsRValue | IsConst | IsVolatile)>>);

                        return FixedArgument(std::in_place_type<Argument<static_cast<ArgumentState>(IsRValue | IsConst)>>);
                    }
                    return FixedArgument(std::in_place_type<Argument<static_cast<ArgumentState>(IsRValue)>>);
                }
                if (isConst)
                {
                    if (isVolatile)
                        return FixedArgument(std::in_place_type<Argument<static_cast<ArgumentState>(IsOriginal | IsConst | IsVolatile)>>);

                    return FixedArgument(std::in_place_type<Argument<static_cast<ArgumentState>(IsOriginal | IsConst)>>);
                }
                return FixedArgument(std::in_place_type<Argument<static_cast<ArgumentState>(IsOriginal)>>);
            }

            bool isConst = false;
            bool isVolatile = false;
            bool isRValue = false;
            bool isLValue = false;
        };

        class FunctionSignature
        {
        public:
            constexpr FunctionSignature(std::string_view representation) noexcept
                : _representation { representation }
            {}

            constexpr auto numberOfArguments() const noexcept -> std::size_t
            {
                std::size_t separators = 0;
        
                for (std::size_t offset = 0; offset != std::string_view::npos; )
                {
                    if (offset = _representation.find(',', offset); offset != std::string_view::npos)
                        separators += 1;
                }

                return separators;
            }

            constexpr auto nthArgument(std::size_t index) const noexcept -> FunctionArgument
            {
                std::size_t offset = _representation.find_last_of('(');

                for (std::size_t separators = 0; offset != std::string_view::npos;)
                {
                    offset++;

                    if (separators++ == index)
                    {
                        const std::size_t end = _representation.find_first_of(",)", offset);

                        return _representation.substr(offset, end - offset);
                    }

                    offset = _representation.find(',', offset);
                }

                return {};
            }

            template<std::size_t index>
            constexpr auto nthArgument() const noexcept -> FunctionArgument
            {
                std::size_t offset = _representation.find_last_of('(');

                for (std::size_t separators = 0; offset != std::string_view::npos;)
                {
                    offset++;

                    if (separators++ == index)
                    {
                        const std::size_t end = _representation.find_first_of(",)", offset);

                        return _representation.substr(offset, end - offset);
                    }

                    offset = _representation.find(',', offset);
                }

                return {};
            }

            constexpr auto _nthArgument(std::size_t index) const noexcept -> std::string_view
            {
                std::size_t offset = _representation.find_last_of('(');

                for (std::size_t separators = 0; offset != std::string_view::npos;)
                {
                    offset++;

                    if (separators++ == index)
                    {
                        const std::size_t end = _representation.find_first_of(",)", offset);

                        return _representation.substr(offset, end - offset);
                    }

                    offset = _representation.find(',', offset);
                }

                return {};
            }

            constexpr bool operator==(const FunctionSignature& rhs) const noexcept
            {
                return _representation == rhs._representation;
            }

            constexpr auto representation() const noexcept -> std::string_view
            {
                return _representation;
            }

        private:
            std::string_view _representation;
        };

        struct IMetaFunction
        {
            constexpr virtual ~IMetaFunction() noexcept = default;

            constexpr virtual auto reflection() const noexcept -> detail::FixedString<1024> = 0;
            constexpr virtual auto signature() const noexcept -> FunctionSignature = 0;
            constexpr virtual auto decayedSignature() const noexcept -> FunctionSignature = 0;
            constexpr virtual auto decayedSignatureHash() const noexcept -> std::size_t = 0;
            constexpr virtual auto nthArgument(std::size_t) const noexcept -> FunctionArgument = 0;
            constexpr virtual auto numberOfArguments() const noexcept -> std::size_t = 0;
            constexpr virtual bool isCompatibleWith(const IMetaFunction&) const noexcept = 0;
            
            template<class F>
            static consteval auto fromFunctionType() noexcept
            {
                using FunctionPointer = traits::function_pointer_t<F>;

                return []<typename R, typename... Args>(R (*)(Args...)) constexpr
                {
                    constexpr MetaFunction<R, Args...> metaFunction {};
                
                    return metaFunction;
                }(reinterpret_cast<FunctionPointer>(nullptr));
            }

            template<class F>
            static consteval auto pointerFromFunctionType() noexcept -> IMetaFunction*
            {
                using FunctionPointer = traits::function_pointer_t<F>;

                return[]<typename R, typename... Args>(R(*)(Args...)) constexpr
                {
                    static constexpr MetaFunction<R, Args...> metaFunction{};

                    return &metaFunction;
                }(reinterpret_cast<FunctionPointer>(nullptr));
            }
        };

            template<class Parameter>
            using InvokedParameter = std::variant<
                Parameter,
                std::add_cv_t<Parameter>,
                std::add_const_t<Parameter>,
                std::add_volatile_t<Parameter>,

                std::remove_reference_t<Parameter>,
                std::remove_reference_t<std::add_cv_t<Parameter>>,
                std::remove_reference_t<std::add_const_t<Parameter>>,
                std::remove_reference_t<std::add_volatile_t<Parameter>>,

                std::add_lvalue_reference_t<Parameter>,
                std::add_lvalue_reference_t<std::add_cv_t<Parameter>>,
                std::add_lvalue_reference_t<std::add_const_t<Parameter>>,
                std::add_lvalue_reference_t<std::add_volatile_t<Parameter>>,

                std::add_rvalue_reference_t<Parameter>,
                std::add_rvalue_reference_t<std::add_cv_t<Parameter>>,
                std::add_rvalue_reference_t<std::add_const_t<Parameter>>,
                std::add_rvalue_reference_t<std::add_volatile_t<Parameter>>
            >;

            enum class InvokedParameterType : std::size_t
            {
                Original,
                CV,
                Const,
                Volatile,

                Unreferenced,
                UnreferencedCV,
                UnreferencedConst,
                UnreferencedVolatile,

                LValue,
                LValueCV,
                LValueConst,
                LValueVolatile,

                RValue,
                RValueCV,
                RValueConst,
                RValueVolatile,
            };

        class ReflectedMetaFunction // : public IMetaFunction
        {
        public:
            constexpr ReflectedMetaFunction(detail::FixedString<1024> s) : _s { s } {}

        private:
            detail::FixedString<1024> _s;
        };

        template<class R, class... Args>
        class MetaFunction : public IMetaFunction
        {
            template<std::size_t N>
            using NthTypeOf = std::tuple_element_t<N, std::tuple<Args...>>;

        public:
            constexpr ~MetaFunction() noexcept = default;

            constexpr DELEGATE_INLINE auto numberOfArguments() const noexcept -> std::size_t override
            {
                return sizeof...(Args);
            }

            constexpr DELEGATE_INLINE auto reflection() const noexcept -> detail::FixedString<1024> override
            {
                constexpr detail::FixedString<1024> reflection(detail::typeName<R(*)(Args...)>());

                return reflection;
            }

            constexpr DELEGATE_INLINE auto signature() const noexcept -> FunctionSignature override
            {
                constexpr auto signature = detail::typeName<R(*)(Args...)>();

                return signature;
            }

            constexpr DELEGATE_INLINE auto decayedSignature() const noexcept -> FunctionSignature override
            {
                constexpr auto signature = detail::typeName<R(*)(std::decay_t<Args>...)>();

                return signature;
            }

            constexpr DELEGATE_INLINE auto decayedSignatureHash() const noexcept -> std::size_t override
            {
                constexpr auto hash = traits::function_hash<R(*)(std::decay_t<Args>...)>;

                return hash;
            }
            
            constexpr auto nthArgument(std::size_t index) const noexcept -> FunctionArgument override
            {
                constexpr auto getNthArg = []<std::size_t I>() constexpr {
                    if constexpr (I < sizeof...(Args))
                    {
                        constexpr auto arg = FunctionArgument::fromConcreteArg<NthTypeOf<I>>();

                        return arg;
                    }
                    else return FunctionArgument();
                };

                switch (index)
                {
                case 0: return getNthArg.template operator()<0>();
                case 1: return getNthArg.template operator()<1>();
                case 2: return getNthArg.template operator()<2>();
                case 3: return getNthArg.template operator()<3>();
                case 4: return getNthArg.template operator()<4>();
                case 5: return getNthArg.template operator()<5>();
                default: return FunctionArgument();
                }
            }

            constexpr bool isCompatibleWith(const IMetaFunction& rhs) const noexcept override
            {
                constexpr FunctionSignature self = detail::typeName<R(*)(Args...)>();

                constexpr auto numberOfArguments = sizeof...(Args);

                if (numberOfArguments != rhs.numberOfArguments())
                    return false;

                if (self == rhs.signature())
                    return true;

                constexpr auto Indices = std::make_index_sequence<sizeof...(Args)>();

                //std::size_t i = 0;
                //return (... && isArgumentCompatibleWith<Args>(rhs.nthArgument(i++)));

                return [&rhs]<std::size_t... I>(std::index_sequence<I...>) {
                    return (... && isArgumentCompatibleWith<NthTypeOf<I>>(rhs.nthArgument(I)));
                }(Indices);
            }

        private:
            template<class OriginalArg>
            static constexpr bool isArgumentCompatibleWith(FunctionArgument invoked) noexcept
            {
                constexpr auto test = detail::typeName<void(*)(OriginalArg)>();

                if constexpr (std::is_lvalue_reference_v<OriginalArg>)
                {
                    if constexpr (std::is_const_v<std::remove_reference_t<OriginalArg>>)
                        return !invoked.isVolatile;

                    if constexpr (std::is_volatile_v<OriginalArg>)
                        return invoked.isLValue && !invoked.isConst;

                    return invoked.isLValue && !invoked.isConst && !invoked.isVolatile;
                }

                if constexpr (std::is_rvalue_reference_v<OriginalArg>)
                {
                    if constexpr (std::is_const_v<std::remove_reference_t<OriginalArg>>)
                        return !invoked.isLValue && !invoked.isVolatile;

                    if constexpr (std::is_volatile_v<OriginalArg>)
                        return !invoked.isLValue && !invoked.isConst;

                    return !invoked.isLValue && !invoked.isConst && !invoked.isVolatile;
                }

                if constexpr (!std::is_move_constructible_v<OriginalArg>)
                    return !invoked.isRValue;

                if constexpr (!std::is_copy_constructible_v<OriginalArg>)
                    return invoked.isLValue || invoked.isRValue;

                return true;
            }
            
        public:
            template<class Proxy, class Delegate>
            static constexpr DELEGATE_INLINE auto getProxyForFunction(const IMetaFunction& rhs) noexcept
            {
                // constexpr auto self = IMetaFunction::fromFunctionType<R(*)(Args...)>();
                constexpr auto hash = traits::function_hash<R(*)(std::decay_t<Args>...)>;
                
                //if (const bool isRhsInvokable = self.isCompatibleWith(rhs); !isRhsInvokable)
                if (hash != rhs.decayedSignatureHash())
                    return reinterpret_cast<void(*)()>(nullptr);

                constexpr auto Indices = std::make_index_sequence<sizeof...(Args)>();

                //using ProxyFunction = traits::delegate_proxy_t<R(*)(Args...), Delegate>;

                //constexpr ProxyFunction f = Proxy();

                //return reinterpret_cast<void(*)()>(f);

                return []<std::size_t... I> (const IMetaFunction& rhs, std::index_sequence<I...>) {
                    using Self = MetaFunction<R, Args...>;
                    //size_t i = 0;

                    //using FunctionProxy = traits::delegate_proxy_t<R(*)(Args...), Delegate>;

                    //constexpr FunctionProxy proxy = Proxy();

                    //return reinterpret_cast<void(*)()>(proxy);

                    return toFunctionPointer<Proxy, Delegate>(getInvokedParameterType<Args>(rhs.nthArgument(I))...);
                }(rhs, Indices);
            }

        private:
            template<class Arg/*, class InvokedArg*/>
            static constexpr auto getInvokedParameterType(FunctionArgument invokedArg) noexcept -> std::size_t
            {
                if constexpr (std::is_lvalue_reference_v<Arg>)
                {
                    // The original argument is a lvalue, so the target needs to be a lvalue too,
                    // otherwise it will result in an Undefined Behavior.

                    // The argument is a lvalue, as expected
                    return addCVQualifiersIfRequired</*InvokedArg, */Arg>(invokedArg);
                }
        
                else if constexpr (std::is_rvalue_reference_v<Arg>)
                {
                    // The original argument is a Arg&&, so the target needs to be either an Arg&& or an Arg,
                    // since lvalues are not transformable into rvalues.

                    if (invokedArg.isRValue)
                    // if constexpr (InvokedArg::is_rvalue)
                    {
                        // The argument is a rvalue, as expected
                        return addCVQualifiersIfRequired</*InvokedArg, */Arg>(invokedArg);
                    }

                    static_assert(
                        std::is_move_constructible_v<Arg>,
                        "[Delegate] A function taking T&& as a parameter and invoked with T requires from T that it is movable."
                    );

                    // If the function is invoked with a non-reference, it entails that a move will
                    // be made at some point.
                    return addCVQualifiersIfRequired</*InvokedArg, */Arg, std::remove_reference_t<Arg>>(invokedArg);
                }

                else if (invokedArg.isLValue)
                // else if constexpr (InvokedArg::is_lvalue)
                {
                    static_assert(
                        std::is_copy_constructible_v<Arg>,
                        "[Delegate] A function taking T as a parameter and invoked with T& requires from T that it is copyable."
                    );

                    // If the function is invoked with a lvalue reference, it entails that a copy will
                    // be made at some point.
                    return addCVQualifiersIfRequired</*InvokedArg, */Arg, std::add_lvalue_reference_t<Arg>>(invokedArg);
                }

                if (invokedArg.isRValue)
                // else if constexpr (InvokedArg::is_rvalue)
                {
                    static_assert(
                        std::is_move_constructible_v<Arg>,
                        "[Delegate] A function taking T as a parameter and invoked with T&& requires from T that it is movable."
                    );

                    // If the function is invoked with a rvalue reference, it entails that a move will
                    // be made at some point.
                    return addCVQualifiersIfRequired</*InvokedArg, */Arg, std::add_rvalue_reference_t<Arg>>(invokedArg);
                }
                else
                {
                    // The argument is not a reference, as expected
                    return addCVQualifiersIfRequired</*InvokedArg, */Arg>(invokedArg);
                }
            }

            template</*class Invoked, */class Arg, class InvokedArg = Arg>
            // requires std::same_as<std::decay_t<Arg>, std::decay_t<InvokedArg>>
            static constexpr auto addCVQualifiersIfRequired(FunctionArgument invokedArg) noexcept -> std::size_t
            {
                if (invokedArg.isConst && invokedArg.isVolatile)
                // if constexpr (Invoked::is_const && Invoked::is_volatile)
                {
                    constexpr auto type = static_cast<std::size_t>(getExactParameterType<InvokedArg, true, true>());

                    return type;
                }
                else if (invokedArg.isConst)
                // if constexpr (Invoked::is_const)
                {
                    constexpr auto type = static_cast<std::size_t>(getExactParameterType<InvokedArg, true, false>());

                    return type;
                }
                if (invokedArg.isVolatile)
                // if constexpr (Invoked::is_volatile)
                {
                    constexpr auto type = static_cast<std::size_t>(getExactParameterType<InvokedArg, false, true>());

                    return type;
                }
                else
                {
                    constexpr auto type = static_cast<std::size_t>(getExactParameterType<InvokedArg, false, false>());

                    return type;
                }
            }

            template<class Arg, bool IsConst, bool IsVolatile>
            static constexpr DELEGATE_INLINE auto getExactParameterType() noexcept -> InvokedParameterType
            {
                if constexpr (std::is_rvalue_reference_v<Arg>)
                {
                    if constexpr (IsConst && IsVolatile)
                        return InvokedParameterType::RValueCV;
                    else if constexpr (IsConst)
                        return InvokedParameterType::RValueConst;
                    else if constexpr (IsVolatile)
                        return InvokedParameterType::RValueVolatile;
                    else
                        return InvokedParameterType::RValue;
                }
                else if constexpr (std::is_lvalue_reference_v<Arg>)
                {
                    if constexpr (IsConst && IsVolatile)
                        return InvokedParameterType::LValueCV;
                    else if constexpr (IsConst)
                        return InvokedParameterType::LValueConst;
                    else if constexpr (IsVolatile)
                        return InvokedParameterType::LValueVolatile;
                    else
                        return InvokedParameterType::LValue;
                }
                else
                {
                    if constexpr (IsConst && IsVolatile)
                        return InvokedParameterType::CV;
                    else if constexpr (IsConst)
                        return InvokedParameterType::Const;
                    else if constexpr (IsVolatile)
                        return InvokedParameterType::Volatile;
                    else
                        return InvokedParameterType::Original;
                }
            }

            using AnyTarget = void (*)(void);

            template<class Arg, std::size_t I>
            using Type = typename std::variant_alternative_t<I, InvokedParameter<Arg>>;

            template<std::size_t I, class Tuple>
            using NthParam = std::tuple_element_t<I, Tuple>;

            template<class Proxy, class Delegate>
            static constexpr DELEGATE_INLINE auto toFunctionPointer(auto&&... indices) noexcept -> AnyTarget
            {
                constexpr auto N = std::variant_size_v<InvokedParameter<int>>;

                //using ProxyFunction = traits::delegate_proxy_t<R(*)(Args...), Delegate>;

                //constexpr ProxyFunction f = Proxy();

                //return reinterpret_cast<void(*)()>(f);

                using MakeProxyFunction = decltype([]<class... Constants>(Constants&&...) [[msvc::forceinline]]
                {
                    //using Original = R(*)(Args&&...);
                    //using Invoked = R(*)(Type<Args, Constants::value>&&...);

                    //constexpr Original o = nullptr;
                    //constexpr Invoked i = nullptr;

                    using Test = std::tuple<Type<Args, Constants::value>...>;

                    constexpr auto Indices = std::make_index_sequence<sizeof...(Constants)>();

                    return []<std::size_t... I>(std::index_sequence<I...>) [[msvc::forceinline]] {
                        constexpr auto getProxy = []<class F, class... Params>() [[msvc::forceinline]] {
                            if constexpr (!std::is_invocable_v<R(*)(Args...), Params...>)
                                return reinterpret_cast<void(*)()>(nullptr);
                            else
                            {
                                using FunctionProxy = traits::delegate_proxy_t<F, Delegate>;
    
                                constexpr FunctionProxy proxy = Proxy();

                                return reinterpret_cast<void(*)()>(proxy);
                            }
                        };

                        return getProxy.template operator()<R(*)(NthParam<I, Test>...), NthParam<I, Test>...>();
                    }(Indices);
                });
                
                return boost::mp11::mp_with_index<N>(MakeProxyFunction{}, DELEGATE_FWD(indices)...);
            }
        };


    /* ===================================================================== */
    /* BadDelegateCall
    /* --------------------------------------------------------------------- */
    /* Exception thrown when a Delegate is called while it doesn't hold
    /* any target.
    /* ===================================================================== */

    class BadDelegateCall : public std::runtime_error
    {
    public:
        BadDelegateCall() noexcept
            : std::runtime_error { "A delegate without any bound function has been called." }
        {}
    };


    /* ===================================================================== */
    /* BadDelegateArguments
    /* --------------------------------------------------------------------- */
    /* Exception thrown when a Delegate is called whit a wrong set of
    /* arguments. Note that this can be thrown only by dynamic delegates.
    /* ===================================================================== */

    class BadDelegateArguments : public std::runtime_error
    {

    public:
        BadDelegateArguments(FunctionSignature target, FunctionSignature invoked) noexcept
            : std::runtime_error { makeErrorMessage(target, invoked) }
        {}

    private:
        std::string makeErrorMessage(FunctionSignature target, FunctionSignature invoked) noexcept
        {
            using namespace std::string_literals;

            return std::format("Function [{}] cannot be invoked with [{}].", target.representation(), invoked.representation());
        }
    };

    /* ===================================================================== */
    /* Helpers to bind functions directly in Delegate constructors.
    /* ===================================================================== */

    struct Callable {};

    template<auto Target>
    struct Function : Callable {};

    template<class T, auto T::*Target>
    struct MemberFunction : Callable { T* instance; };

    template<auto Target>
    inline constexpr DELEGATE_INLINE auto bind() noexcept -> Function<Target> { return {}; }

    template<auto Target, class T>
    inline constexpr DELEGATE_INLINE auto bind(T* instance) noexcept -> MemberFunction<T, Target> { return { instance }; }


    /* ===================================================================== */
    /* Initial Delegate declaration.
    /* ===================================================================== */

    template<class Signature         = void(...),
             std::size_t StorageSize = sizeof(void*),
             std::size_t Alignment   = alignof(void*)>
    class Delegate;


    /* ===================================================================== */
    /*  Partial specialization for dynamic signatures.
    /* --------------------------------------------------------------------- */
    /*  Examples:
    /*  - Delegate<void (...)>
    /*  - Delegate<int (...)>
    /*  - Delegate<float (...)>
    /* ===================================================================== */

    template<class Ret, std::size_t StorageSize, std::size_t Alignment>
    class Delegate<Ret(...), StorageSize, Alignment>
    {
        using AnyTarget = void (*)(void);

        static constexpr auto PointerSize = (std::max)(sizeof(AnyTarget), sizeof(void*)); 
        static constexpr auto BufferSize  = (std::max)(StorageSize, PointerSize);

        template<class F>
        struct is_storable : std::bool_constant<sizeof(F) <= BufferSize && alignof(F) <= Alignment> {};

        template<class F>
        static constexpr bool is_storable_v = is_storable<F>::value;

    public:
        constexpr DELEGATE_INLINE Delegate() noexcept = default;

        template<class R2, class... Args>
        explicit Delegate(std::function<R2(Args...)>&& target) noexcept
        {
            bind(DELEGATE_FWD(target));
        }

        template<auto Target>
        constexpr DELEGATE_INLINE Delegate(Function<Target>) noexcept
            : _wrapper { &executeFunction<Target> }
        {}

        template<class T, auto T::*Target>
        constexpr DELEGATE_INLINE Delegate(MemberFunction<const T, Target> target) noexcept
            : _constInstance { target.instance                         }
            , _wrapper       { &executeMemberFunction<const T, Target> }
        {}

        template<class T, auto T::*Target>
        constexpr DELEGATE_INLINE Delegate(MemberFunction<T, Target> target) noexcept
            : _instance { target.instance                   }
            , _wrapper  { &executeMemberFunction<T, Target> }
        {}

        template<class F,
                 typename = std::enable_if_t<(
                    !traits::is_function_v<F>
                 )>>
        constexpr DELEGATE_INLINE Delegate(F* target) noexcept
            : _instance { target                  }
            , _wrapper  { &executeCallableView<F> }
        {}
            
        template<class F,
                 typename = std::enable_if_t<(
                    !traits::is_function_v<F>
                 )>>
        constexpr DELEGATE_INLINE Delegate(const F* target) noexcept
            : _constInstance { target                        }
            , _wrapper       { &executeCallableView<const F> }
        {}

        template<class F,
                 std::enable_if_t<(
                    std::is_empty_v<F> &&
                    std::is_default_constructible_v<F>
                 ), int> = 0>
        constexpr DELEGATE_INLINE Delegate(F&&) noexcept
            : _wrapper { &executeEmptyCallable<F> }
        {}

        template<class F,
                 std::enable_if_t<(
                     traits::is_likely_captureless_lambda_v<F>
                 ), int> = 0>
        constexpr DELEGATE_INLINE Delegate(F&& target) noexcept
            : Delegate { detail::retrospective_cast(std::forward<F>(target)) }
        {}

        template<class F,
                 std::enable_if_t<(
                     traits::is_likely_capturing_lambda_v<F> &&
                     is_storable_v<std::decay_t<F>>
                 ), int> = 0>
        constexpr DELEGATE_INLINE Delegate(F&& target) noexcept
            : _wrapper { &executeStatefulCallable<F> }
        {
            new (_storage) F(std::forward<F>(target));
        }

        template<class R, class... Args>
        constexpr DELEGATE_INLINE Delegate(R (*target)(Args...)) noexcept
            : _function { reinterpret_cast<AnyTarget>(target)   }
            , _wrapper  { &executeStatelessCallable<R, Args...> }
        {}

    public:
        template<class R2, class... Args>
        void operator=(std::function<R2(Args...)>&& target) noexcept
        {
            bind(std::forward<decltype(target)>(target));
        }

        template<auto Target>
        constexpr DELEGATE_INLINE void operator=(Function<Target>) noexcept
        {
            bind<Target>();
        }
            
        template<class T, auto T::*Target>
        constexpr DELEGATE_INLINE void operator=(MemberFunction<const T, Target> target) noexcept
        {
            bind<Target>(target.instance);
        }

        template<class T, auto T::* Target>
        constexpr DELEGATE_INLINE void operator=(MemberFunction<T, Target> target) noexcept
        {
            bind<Target>(target.instance);
        }

        template<class F,
                 typename = std::enable_if_t<(
                    !traits::is_function_v<F>
                 )>>
        constexpr DELEGATE_INLINE void operator=(F* target) noexcept
        {
            bind(target);
        }
            
        template<class F,
                 typename = std::enable_if_t<(
                    !traits::is_function_v<F>
                 )>>
        constexpr DELEGATE_INLINE void operator=(const F* target) noexcept
        {
            bind(target);
        }

        template<class F,
                 std::enable_if_t<(
                    std::is_empty_v<F> &&
                    std::is_default_constructible_v<F>
                 ), int> = 0>
        constexpr DELEGATE_INLINE void operator=(F&& target) noexcept
        {
            bind<F>();
        }

        template<class F,
                 std::enable_if_t<(
                     traits::is_likely_captureless_lambda_v<F>
                 ), int> = 0>
        constexpr DELEGATE_INLINE void operator=(F&& target) noexcept
        {
            bind(detail::retrospective_cast(std::forward<F>(target)));
        }

        template<class F,
                 std::enable_if_t<(
                    traits::is_likely_capturing_lambda_v<F> &&
                    is_storable_v<std::decay_t<F>>
                 ), int> = 0>
        constexpr DELEGATE_INLINE void operator=(F&& target) noexcept
        {
            bind(std::forward<F>(target));
        }

        template<class R, class... Args>
        constexpr DELEGATE_INLINE void operator=(R(*target)(Args...)) noexcept
        {
            bind(target);
        }

    public:
        template<class Arg>
        using ProxyArg = std::add_const_t<std::add_volatile_t<std::add_lvalue_reference_t<Arg>>>;

        template<class... Args>
        constexpr DELEGATE_INLINE auto operator ()(Args&&... args) const -> Ret
        {
            using InvokedTarget = Ret(*)(Args...);
            using ProxyFunction = traits::delegate_proxy_t<InvokedTarget, Delegate>;

            constexpr MetaFunction<Ret, Args...> invokedFunction {};

            //std::cout << std::format("Real Proxy => [{}].", detail::typeName<ProxyFunction>()) << std::endl;

            //std::cout << "Real Proxy Signature => " << invokedFunction.signature().representation() << std::endl;

            const auto function = _wrapper(invokedFunction, true);
            const auto proxy    = reinterpret_cast<ProxyFunction>(function);

            return proxy(this, ReflectedArg<std::decay_t<Args>>(DELEGATE_FWD(args))...);
        }

        constexpr DELEGATE_INLINE void reset() noexcept { _wrapper = &throwBadCall; }

        explicit constexpr DELEGATE_INLINE operator bool() const noexcept { return hasTarget(); }

    public:
        template<class R>
        [[nodiscard]]
        constexpr DELEGATE_INLINE bool hasReturnType() const noexcept
        {
            constexpr bool hasReturnType = std::is_same_v<R, Ret>;

            return hasReturnType;
        }

        template<class... Args>
        [[nodiscard]]
        constexpr DELEGATE_INLINE bool isInvokable() const noexcept
        {
            using ProxyFunction = Ret(*)(const Delegate*, Args&&...);

            constexpr MetaFunction<Ret, Args...> invokedFunction{};

            const auto function = _wrapper(invokedFunction, false);
            const auto proxy = reinterpret_cast<ProxyFunction>(function);

            return (proxy != nullptr);
        }

        [[nodiscard]]
        constexpr DELEGATE_INLINE bool hasTarget() const noexcept
        {
            return _wrapper != &throwBadCall;
        }

        template<auto Target>
        [[nodiscard]]
        constexpr DELEGATE_INLINE bool hasTarget() const noexcept
        {
            return _wrapper == &executeFunction<Target>;
        }

        template<auto Target, class T,
                 typename = std::enable_if_t<(
                     std::is_member_function_pointer_v<decltype(Target)>
                 )>>
        [[nodiscard]]
        constexpr DELEGATE_INLINE bool hasTarget(const T* instance) const noexcept
        {
            return _wrapper       == &executeMemberFunction<const T, Target>
                && _constInstance == instance;
        }

        template<auto Target, class T,
                 typename = std::enable_if_t<(
                     std::is_member_function_pointer_v<decltype(Target)>
                 )>>
        [[nodiscard]]
        constexpr DELEGATE_INLINE bool hasTarget(T* instance) const noexcept
        {
            return _wrapper  == &executeMemberFunction<T, Target>
                && _instance == instance;
        }

        template<class F,
                 typename = std::enable_if_t<(
                     !traits::is_function_v<F>
                 )>>
        [[nodiscard]]
        constexpr DELEGATE_INLINE bool hasTarget(const F* instance) const noexcept
        {
            return _wrapper       == &executeCallableView<const F>
                && _constInstance == instance;
        }

        template<class F,
                 typename = std::enable_if_t<(
                     !traits::is_function_v<F>
                 )>>
        [[nodiscard]]
        constexpr DELEGATE_INLINE bool hasTarget(F* instance) const noexcept
        {
            return _wrapper  == &executeCallableView<F>
                && _instance == instance;
        }

        template<class F,
                 typename = std::enable_if_t<(
                     std::is_empty_v<F> &&
                     std::is_default_constructible_v<F>
                 )>>
        [[nodiscard]]
        constexpr DELEGATE_INLINE bool hasTarget() const noexcept
        {
            return _wrapper == &executeEmptyCallable<F>;
        }

        template<class F,
                 typename = std::enable_if_t<(
                     traits::is_likely_capturing_lambda_v<F> &&
                     is_storable_v<std::decay_t<F>>
                 )>>
        [[nodiscard]]
        constexpr DELEGATE_INLINE bool hasTarget(const F& target) const noexcept
        {
            if (_wrapper != &executeStatefulCallable<F>)
                return false;

            const auto& callable = *std::launder(reinterpret_cast<const F*>(_storage));

            if constexpr (traits::is_equality_comparable_v<F>)
                return callable == target;
            else
            {
                static_assert(std::has_unique_object_representations_v<F>);
                return std::memcmp(&callable, &target, sizeof(F)) == 0;
            }
        }

        template<class R2, class...Args>
        [[nodiscard]]
        constexpr DELEGATE_INLINE bool hasTarget(R2(*target)(Args...)) const noexcept
        {
            const auto function = reinterpret_cast<decltype(target)>(_function);

            return _wrapper == &executeStatelessCallable<R2, Args...>
                && function == target;
        }

    public:
        template<class R2, class... Args>
        void bind(std::function<R2(Args...)>&& target) noexcept
        {
            using Target = R2(*)(Args...);

            _function = reinterpret_cast<void(*)()>(target.template target<Target>());
            _wrapper  = &executeStatelessCallable<R2, Args...>;

            if (!target)
            {
                // The target is empty, so we cannot extract anything from it
                _wrapper = &throwBadCall;
            }
            else if (_function == nullptr)
            {
                // The target cannot be converted to a function pointer
                // Thus, we try to save it into a lambda if our storage is large enough

                static_assert(sizeof(target) <= sizeof(_storage));

                *this = [f = DELEGATE_FWD(target)](Args&&... args) -> R2 {
                    return f(std::forward<Args>(args)...);
                };
            }
        }

        template<auto Target>
        constexpr DELEGATE_INLINE void bind() noexcept
        {
            _wrapper = &executeFunction<Target>;
        }
            
        template<auto Target, class T>
        constexpr DELEGATE_INLINE void bind(const T* instance) noexcept
        {
            _constInstance = instance;
            _wrapper       = &executeMemberFunction<const T, Target>;
        }

        template<auto Target, class T>
        constexpr DELEGATE_INLINE void bind(T* instance) noexcept
        {
            _instance = instance;
            _wrapper  = &executeMemberFunction<T, Target>;
        }

        template<class F,
                 typename = std::enable_if_t<(
                     !traits::is_function_v<F>
                 )>>
        constexpr DELEGATE_INLINE void bind(F* target) noexcept
        {
            _instance = target;
            _wrapper  = &executeCallableView<F>;
        }
            
        template<class F,
                 typename = std::enable_if_t<(
                     !traits::is_function_v<F>
                 )>>
        constexpr DELEGATE_INLINE void bind(const F* target) noexcept
        {
            _constInstance = target;
            _wrapper       = &executeCallableView<const F>;
        }

        template<class F,
                 std::enable_if_t<(
                     std::is_empty_v<F> &&
                     std::is_default_constructible_v<F>
                 ), int> = 0>
        constexpr DELEGATE_INLINE void bind(F&&) noexcept
        {
            _wrapper = &executeEmptyCallable<F>;
        }

        template<class F,
                 std::enable_if_t<(
                     traits::is_likely_captureless_lambda_v<F>
                 ), int> = 0>
        constexpr DELEGATE_INLINE void bind(F&& target) noexcept
        {
            bind(detail::retrospective_cast(std::forward<F>(target)));
        }

        template<class F,
                 std::enable_if_t<(
                     traits::is_likely_capturing_lambda_v<F> &&
                     is_storable_v<std::decay_t<F>>
                 ), int> = 0>
        constexpr DELEGATE_INLINE void bind(F&& target) noexcept
        {
            _wrapper = &executeStatefulCallable<F>;

            new (_storage) F(std::forward<F>(target));
        }

        template<class R, class... Args>
        constexpr DELEGATE_INLINE void bind(R(*target)(Args...)) noexcept
        {
            _function = reinterpret_cast<AnyTarget>(target);
            _wrapper  = &executeStatelessCallable<R, Args...>;
        }

    private:
        [[noreturn]]
        static constexpr DELEGATE_INLINE auto throwBadCall(const IMetaFunction&, const bool) -> AnyTarget
        {
            throw BadDelegateCall();
        }

        template<auto Target>
        static constexpr DELEGATE_INLINE auto executeFunction(
            const IMetaFunction& invokedFunction,
            const bool throwOnMismatch
        ) -> AnyTarget
        {
            constexpr auto proxy = []<class... Invoked>(const Delegate * self, Invoked&&... args) constexpr [[msvc::forceinline]] -> Ret {
                if constexpr (std::is_void_v<Ret>)
                    std::invoke(Target, std::forward<Invoked>(args)...);
                else
                    return std::invoke(Target, std::forward<Invoked>(args)...);
            };

            return getProxyFunction<decltype(Target), decltype(proxy)>(invokedFunction, throwOnMismatch);
        }

        template<class T, auto T::*Target>
        static constexpr DELEGATE_INLINE auto executeMemberFunction(
            const IMetaFunction& invokedFunction,
            const bool throwOnMismatch
        ) -> AnyTarget
        {
            constexpr auto proxy = []<class... Invoked>(const Delegate * self, Invoked&&... args) constexpr [[msvc::forceinline]] -> Ret
            {
                const auto& instance = [](const Delegate* self) constexpr -> T&
                {
                    if constexpr (std::is_const_v<T>)
                        return *static_cast<T*>(self->_constInstance);

                    return *static_cast<T*>(self->_instance);
                }(self);

                if constexpr (std::is_void_v<Ret>)
                    std::invoke(Target, instance, std::forward<Invoked>(args)...);
                else
                    return std::invoke(Target, instance, std::forward<Invoked>(args)...);
            };

            return getProxyFunction<decltype(Target), decltype(proxy)>(invokedFunction, throwOnMismatch);
        }

        template<class F>
        static constexpr DELEGATE_INLINE auto executeCallableView(
            const IMetaFunction& invokedFunction,
            const bool throwOnMismatch
        ) -> AnyTarget
        {
            constexpr auto proxy = []<class... Invoked>(const Delegate * self, Invoked&&... args) constexpr [[msvc::forceinline]] -> Ret
            {
                const auto target = [](const Delegate* self) constexpr -> F*
                {
                    if constexpr (std::is_const_v<F>)
                        return static_cast<F*>(self->_constInstance);
                        
                    return static_cast<F*>(self->_instance);
                }(self);

                if constexpr (std::is_void_v<Ret>)
                    std::invoke(*target, std::forward<Invoked>(args)...);
                else
                    return std::invoke(*target, std::forward<Invoked>(args)...);
            };

            return getProxyFunction<F, decltype(proxy)>(invokedFunction, throwOnMismatch);
        }

        template<class F>
        static constexpr DELEGATE_INLINE auto executeEmptyCallable(
            const IMetaFunction& invokedFunction,
            const bool throwOnMismatch
        ) -> AnyTarget
        {
            constexpr auto proxy = []<class... Invoked>(const Delegate *, ReflectedArg<Invoked>&&... args) constexpr -> Ret {
                using OriginalArgs = traits::function_args<F>;
                using DecayedArgs  = traits::function_decayed_args<F>;

                static_assert(std::is_same_v<std::tuple<Invoked...>, DecayedArgs>);

                constexpr auto Indices = std::make_index_sequence<sizeof...(Invoked)>();

                return [...args = DELEGATE_FWD(args)]<std::size_t... I>(std::index_sequence<I...>) constexpr
                {
                    if constexpr (std::is_void_v<Ret>)
                        std::invoke(F{}, args.forward<std::tuple_element_t<I, OriginalArgs>>()...);
                    else
                        return std::invoke(F{}, args.forward<std::tuple_element_t<I, OriginalArgs>>()...);
                }(Indices);
            };

            return getProxyFunction<F, decltype(proxy)>(invokedFunction, throwOnMismatch);
        }

        template<class F>
        static constexpr DELEGATE_INLINE auto executeStatefulCallable(
            const IMetaFunction& invokedFunction,
            const bool throwOnMismatch
        ) -> AnyTarget
        {
            constexpr auto proxy = []<class... Invoked>(const Delegate * self, Invoked&&... args) constexpr [[msvc::forceinline]] -> Ret
            {
                const auto& target = *std::launder(reinterpret_cast<const F*>(_storage));

                if constexpr (std::is_void_v<Ret>)
                    std::invoke(target, std::forward<Invoked>(args)...);
                else
                    return std::invoke(target, std::forward<Invoked>(args)...);
            };

            return getProxyFunction<F, decltype(proxy)>(invokedFunction, throwOnMismatch);
        }

        template<class R, class... Args>
        static constexpr DELEGATE_INLINE auto executeStatelessCallable(
            const IMetaFunction& invokedFunction,
            const bool throwOnMismatch
        ) -> AnyTarget
        {
            constexpr auto proxy = []<class... Invoked>(const Delegate* self, Invoked&&... args) constexpr [[msvc::forceinline]] -> Ret
            {
                const auto target = reinterpret_cast<R(*)(Args...)>(self->_function);

                if constexpr (std::is_void_v<Ret>)
                    std::invoke(target, std::forward<Invoked>(args)...);
                else
                    return std::invoke(target, std::forward<Invoked>(args)...);
            };

            return getProxyFunction<R(*)(Args...), decltype(proxy)>(invokedFunction, throwOnMismatch);
        }

        template<class Target, class GenericProxy>
        static constexpr DELEGATE_INLINE auto getProxyFunction(
            const IMetaFunction& invoked,
            const bool throwOnMismatch
        ) -> AnyTarget
        {
            constexpr auto target = IMetaFunction::fromFunctionType<Target>();
            constexpr auto decayedSignatureHash = target.decayedSignatureHash();

            using FunctionProxy = traits::delegate_proxy_t<Target, Delegate>;

            constexpr FunctionProxy proxy = GenericProxy();

            //if (const auto proxy = target.getProxyForFunction<GenericProxy, Delegate>(invoked))
            //     return proxy;

            if (decayedSignatureHash == invoked.decayedSignatureHash())
                return reinterpret_cast<AnyTarget>(proxy);
            else if (throwOnMismatch)
                throw BadDelegateArguments(target.signature(), invoked.signature());

            return reinterpret_cast<AnyTarget>(nullptr);
        }

    protected:
        union
        {
            void*       _instance;
            const void* _constInstance;
            AnyTarget   _function;

            alignas(Alignment) std::byte _storage[BufferSize] { };
        };

        using Wrapper = AnyTarget(*)(const IMetaFunction&, const bool);

        Wrapper _wrapper { &throwBadCall };
    };


    /* ===================================================================== */
    /*  Partial specialization for fixed signatures.
    /* --------------------------------------------------------------------- */
    /* Examples:
    /*  - Delegate<void (void)>
    /*  - Delegate<int (int, float)>
    /*  - Delegate<std::string (const char*)>
    /* ===================================================================== */

    template<std::size_t StorageSize, std::size_t Alignment, class Ret, class... Args>
    class Delegate<Ret(Args...), StorageSize, Alignment>
    {
        using AnyTarget = void (*)(void);

        static constexpr auto PointerSize = (std::max)(sizeof(AnyTarget), sizeof(void*)); 
        static constexpr auto BufferSize  = (std::max)(StorageSize, PointerSize);

        template<class F>
        struct is_storable : std::bool_constant<sizeof(F) <= BufferSize && alignof(F) <= Alignment> {};

        template<class F>
        static constexpr bool is_storable_v = is_storable<F>::value;

    public:
        constexpr DELEGATE_INLINE Delegate() noexcept = default;

        template<class R2, class... UArgs,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, std::function<R2(UArgs...)>, Args...>
                 )>>
        explicit DELEGATE_INLINE Delegate(std::function<R2(UArgs...)>&& target) noexcept
        {
            bind(DELEGATE_FWD(target));
        }

        template<auto Target,
                 typename = std::enable_if_t<(
                    std::is_invocable_r_v<Ret, decltype(Target), Args...>
                 )>>
        constexpr DELEGATE_INLINE Delegate(Function<Target>) noexcept
            : _proxy { &executeFunction<Target> }
        {}

        template<class T, auto T::*Target,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, decltype(Target), const T&, Args...>
                 )>>
        constexpr DELEGATE_INLINE Delegate(MemberFunction<const T, Target> target) noexcept
            : _constInstance { target.instance                         }
            , _proxy         { &executeMemberFunction<const T, Target> }
        {}

        template<class T, auto T::* Target,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, decltype(Target), T&, Args...>
                 )>>
        constexpr DELEGATE_INLINE Delegate(MemberFunction<T, Target> target) noexcept
            : _instance { target.instance                   }
            , _proxy    { &executeMemberFunction<T, Target> }
        {}

        template<class F,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, F, Args...> &&
                     !traits::is_function_v<F>
                 )>>
        constexpr DELEGATE_INLINE Delegate(F* target) noexcept
            : _instance { target                  }
            , _proxy    { &executeCallableView<F> }
        {}
            
        template<class F,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, const F, Args...> &&
                     !traits::is_function_v<F>
                 )>>
        constexpr DELEGATE_INLINE Delegate(const F* target) noexcept
            : _constInstance { target                        }
            , _proxy         { &executeCallableView<const F> }
        {}

        template<class F,
                 std::enable_if_t<(
                     std::is_empty_v<F> &&
                     std::is_default_constructible_v<F> &&
                     std::is_invocable_r_v<Ret, F, Args...>
                 ), int> = 0>
        constexpr DELEGATE_INLINE Delegate(F&&) noexcept
            : _proxy { &executeEmptyCallable<F> }
        {}

        template<class F,
                 std::enable_if_t<(
                     traits::is_likely_captureless_lambda_v<F>
                 ), int> = 0>
        constexpr DELEGATE_INLINE Delegate(F&& target) noexcept
            : Delegate { detail::retrospective_cast(std::forward<F>(target)) }
        {}

        template<class F,
                 std::enable_if_t<(
                     traits::is_likely_capturing_lambda_v<F> &&
                     is_storable_v<std::decay_t<F>> &&
                     std::is_invocable_r_v<Ret, const F&, Args...>
                 ), int> = 0>
        constexpr DELEGATE_INLINE Delegate(F&& target) noexcept
            : _proxy { &executeStatefulCallable<F> }
        {
            new (_storage) F(std::forward<F>(target));
        }

        template<class R, class... UArgs,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, R(*)(UArgs...), Args...>
                 )>>
        constexpr DELEGATE_INLINE Delegate(R(*target)(UArgs...)) noexcept
            : _function { reinterpret_cast<void (*)()>(target)        }
            , _proxy    { &executeStatelessCallable<R, UArgs...> }
        {}

    public:
        template<class R2, class... UArgs,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, std::function<R2(UArgs...)>, Args...>
                 )>>
        void operator=(std::function<R2(UArgs...)>&& target) noexcept
        {
            bind(DELEGATE_FWD(target));
        }

        template<auto Target,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, decltype(Target), Args...>
                 )>>
        constexpr DELEGATE_INLINE void operator=(Function<Target>) noexcept
        {
            bind<Target>();
        }
            
        template<class T, auto T::*Target,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, decltype(Target), const T&, Args...>
                 )>>
        constexpr DELEGATE_INLINE void operator=(MemberFunction<const T, Target> target) noexcept
        {
            bind<Target>(target.instance);
        }

        template<class T, auto T::*Target,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, decltype(Target), T&, Args...>
                 )>>
        constexpr DELEGATE_INLINE void operator=(MemberFunction<T, Target> target) noexcept
        {
            bind<Target>(target.instance);
        }

        template<class F,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, F, Args...> &&
                     !traits::is_function_v<F>
                 )>>
        constexpr DELEGATE_INLINE void operator=(F* target) noexcept
        {
            bind(target);
        }
            
        template<class F,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, const F, Args...> &&
                     !traits::is_function_v<F>
                 )>>
        constexpr DELEGATE_INLINE void operator=(const F* target) noexcept
        {
            bind(target);
        }

        template<class F,
                 std::enable_if_t<(
                     std::is_empty_v<F>&&
                     std::is_default_constructible_v<F> &&
                     std::is_invocable_r_v<Ret, F, Args...>
                 ), int> = 0>
        constexpr DELEGATE_INLINE void operator=(F&&) noexcept
        {
            bind<F>();
        }

        template<class F,
                 std::enable_if_t<(
                     traits::is_likely_captureless_lambda_v<F>
                 ), int> = 0>
        constexpr DELEGATE_INLINE void operator=(F&& target) noexcept
        {
            bind(std::forward<F>(target));
        }

        template<class F,
                 std::enable_if_t<(
                     traits::is_likely_capturing_lambda_v<F> &&
                     is_storable_v<std::decay_t<F>> &&
                     std::is_invocable_r_v<Ret, const F&, Args...>
                 ), int> = 0>
        constexpr DELEGATE_INLINE void operator=(F&& target) noexcept
        {
            bind(std::forward<F>(target));
        }

        template<class R, class... UArgs,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, R(*)(UArgs...), Args...>
                 )>>
        constexpr DELEGATE_INLINE void operator=(R(*target)(UArgs...)) noexcept
        {
            bind(target);
        }

    public:
        template<class... UArgs,
                 typename = std::enable_if_t<std::is_invocable_r_v<Ret, Ret(*)(Args...), UArgs...>>>
        constexpr auto operator ()(UArgs&&... args) const -> Ret
        {
            return _proxy(this, std::forward<UArgs>(args)...);
        }

        constexpr DELEGATE_INLINE void reset() noexcept { _proxy = &throwBadCall; }

        explicit constexpr DELEGATE_INLINE operator bool() const noexcept { return hasTarget(); }

    public:
        template<class R>
        [[nodiscard]]
        constexpr DELEGATE_INLINE bool hasReturnType() const noexcept
        {
            constexpr bool hasReturnType = std::is_same_v<R, Ret>;

            return hasReturnType;
        }

        template<class... UArgs>
        [[nodiscard]]
        constexpr DELEGATE_INLINE bool isInvokable() const noexcept
        {
            constexpr bool isInvokable = std::is_invocable_r_v<Ret, Ret(*)(Args...), UArgs...>;

            return isInvokable;
        }

        [[nodiscard]]
        constexpr DELEGATE_INLINE bool hasTarget() const noexcept
        {
            return _proxy != &throwBadCall;
        }

        template<auto Target,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, decltype(Target), Args...>
                 )>>
        [[nodiscard]]
        constexpr DELEGATE_INLINE bool hasTarget() const noexcept
        {
            return _proxy == &executeFunction<Target>;
        }

        template<auto Target, class T,
                 typename = std::enable_if_t<(
                     std::is_member_function_pointer_v<decltype(Target)> &&
                     std::is_invocable_r_v<Ret, decltype(Target), Args...>
                 )>>
        [[nodiscard]]
        constexpr DELEGATE_INLINE bool hasTarget(const T* instance) const noexcept
        {
            return _proxy         == &executeMemberFunction<const T, Target>
                && _constInstance == instance;
        }

        template<auto Target, class T,
                 typename = std::enable_if_t<(
                     std::is_member_function_pointer_v<decltype(Target)> &&
                     std::is_invocable_r_v<Ret, decltype(Target), Args...>
                 )>>
        [[nodiscard]]
        constexpr DELEGATE_INLINE bool hasTarget(T* instance) const noexcept
        {
            return _proxy    == &executeMemberFunction<T, Target>
                && _instance == instance;
        }

        template<class F,
                 typename = std::enable_if_t<(
                     !traits::is_function_v<F> &&
                     std::is_invocable_r_v<Ret, const F, Args...>
                 )>>
        [[nodiscard]]
        constexpr DELEGATE_INLINE bool hasTarget(const F* instance) const noexcept
        {
            return _proxy         == &executeCallableView<const F>
                && _constInstance == instance;
        }

        template<class F,
                 typename = std::enable_if_t<(
                     !traits::is_function_v<F> &&
                     std::is_invocable_r_v<Ret, F, Args...>
                 )>>
        [[nodiscard]]
        constexpr DELEGATE_INLINE bool hasTarget(F* instance) const noexcept
        {
            return _proxy    == &executeCallableView<F>
                && _instance == instance;
        }

        template<class F,
                 typename = std::enable_if_t<(
                     std::is_empty_v<F> &&
                     std::is_default_constructible_v<F> &&
                     std::is_invocable_r_v<Ret, F, Args...>
                 )>>
        [[nodiscard]]
        constexpr DELEGATE_INLINE bool hasTarget() const noexcept
        {
            return _proxy == &executeEmptyCallable<F>;
        }

        template<class F,
                 typename = std::enable_if_t<(
                     traits::is_likely_capturing_lambda_v<F> &&
                     is_storable_v<std::decay_t<F>> &&
                     std::is_invocable_r_v<Ret, const F&, Args...>
                 )>>
        [[nodiscard]]
        constexpr DELEGATE_INLINE bool hasTarget(const F& target) const noexcept
        {
            if (_proxy != &executeStatefulCallable<F>)
                return false;

            const auto& callable = *std::launder(reinterpret_cast<const F*>(_storage));

            if constexpr (traits::is_equality_comparable_v<F>)
                return callable == target;
            else
            {
                static_assert(std::has_unique_object_representations_v<F>);
                return std::memcmp(&callable, &target, sizeof(F)) == 0;
            }
        }

        template<class R2, class...UArgs,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, R2(*)(UArgs...), Args...>
                 )>>
        [[nodiscard]]
        constexpr DELEGATE_INLINE bool hasTarget(R2(*target)(UArgs...)) const noexcept
        {
            const auto function = reinterpret_cast<decltype(target)>(_function);

            return _proxy == &executeStatelessCallable<R2, UArgs...>
                && function == target;
        }

    public:
        template<class R2, class... UArgs,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, std::function<R2(UArgs...)>, Args...>
                 )>>
        void bind(std::function<R2(UArgs...)>&& target) noexcept
        {
            using Target = R2(*)(UArgs...);

            _function = reinterpret_cast<void(*)()>(target.template target<Target>());
            _proxy    = &executeStatelessCallable<R2, UArgs...>;

            if (!target)
            {
                // The target is empty, so we cannot extract anything from it
                _proxy = &throwBadCall;
            }
            else if (_function == nullptr)
            {
                // The target cannot be converted to a function pointer
                // Thus, we try to save it into a lambda if our storage is large enough

                static_assert(sizeof(target) <= sizeof(_storage));

                *this = [f = DELEGATE_FWD(target)](UArgs&&... args) -> R2 {
                    return f(std::forward<UArgs>(args)...);
                };
            }
        }

        template<auto Target,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, decltype(Target), Args...>
                 )>>
        constexpr DELEGATE_INLINE void bind() noexcept
        {
            _proxy = &executeFunction<Target>;
        }

            
        template<auto Target, class T,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, decltype(Target), const T&, Args...>
                 )>>
        constexpr DELEGATE_INLINE void bind(const T* instance) noexcept
        {
            _constInstance = instance;
            _proxy         = &executeMemberFunction<const T, Target>;
        }

        template<auto Target, class T,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, decltype(Target), T&, Args...>
                 )>>
        constexpr DELEGATE_INLINE void bind(T* instance) noexcept
        {
            _instance = instance;
            _proxy    = &executeMemberFunction<T, Target>;
        }

        template<class F,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, F, Args...> &&
                     !traits::is_function_v<F>
                 )>>
        constexpr DELEGATE_INLINE void bind(F* target) noexcept
        {
            _instance = target;
            _proxy    = &executeCallableView<F>;
        }
            
        template<class F,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, const F, Args...> &&
                     !traits::is_function_v<F>
                 )>>
        constexpr DELEGATE_INLINE void bind(const F* target) noexcept
        {
            _constInstance = target;
            _proxy        = &executeCallableView<const F>;
        }

        template<class F,
                 typename = std::enable_if_t<(
                     std::is_empty_v<F>&&
                     std::is_default_constructible_v<F> &&
                     std::is_invocable_r_v<Ret, F, Args...>
                 )>>
        constexpr DELEGATE_INLINE void bind() noexcept
        {
            _proxy = &executeEmptyCallable<F>;
        }

        template<class F,
                 std::enable_if_t<(
                     traits::is_likely_captureless_lambda_v<F> &&
                     std::is_invocable_r_v<Ret, F, Args...>
                 ), int> = 0>
        constexpr DELEGATE_INLINE void bind(F&& target) noexcept
        {
            bind(detail::retrospective_cast(std::forward<F>(target)));
        }

        template<class F,
                 std::enable_if_t<(
                     traits::is_likely_capturing_lambda_v<F> &&
                     is_storable_v<std::decay_t<F>> &&
                     std::is_invocable_r_v<Ret, const F&, Args...>
                 ), int> = 0>
        constexpr DELEGATE_INLINE void bind(F&& target) noexcept
        {
            _proxy = &executeStatefulCallable<F>;

            new (_storage) F(std::forward<F>(target));
        }

        template<class R, class... UArgs,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, R(*)(UArgs...), Args...>
                 )>>
        constexpr DELEGATE_INLINE void bind(R(*target)(UArgs...)) noexcept
        {
            _function = reinterpret_cast<void (*)()>(target);
            _proxy    = &executeStatelessCallable<R, UArgs...>;
        }

    private:
        [[noreturn]]
        static constexpr DELEGATE_INLINE auto throwBadCall(const Delegate*, Args&&...) -> Ret
        {
            throw BadDelegateCall();
        }

        template<auto Target>
        static constexpr DELEGATE_INLINE auto executeFunction(const Delegate*, Args&&... args) -> Ret
        {
            if constexpr (std::is_void_v<Ret>)
                std::invoke(Target, DELEGATE_FWD(args)...);
            else
                return std::invoke(Target, DELEGATE_FWD(args)...);
        }

        template<class T, auto T::*Target>
        static constexpr DELEGATE_INLINE auto executeMemberFunction(const Delegate* self, Args&&... args) -> Ret
        {
            const auto& instance = [](const Delegate* self) constexpr noexcept -> T&
            {
                if constexpr (std::is_const_v<T>)
                    return *static_cast<T*>(self->_constInstance);

                return *static_cast<T*>(self->_instance);
            }(self);

            if constexpr (std::is_void_v<Ret>)
                std::invoke(Target, instance, std::forward<Args>(args)...);
            else
                return std::invoke(Target, instance, std::forward<Args>(args)...);
        }

        template<class F>
        static constexpr DELEGATE_INLINE auto executeCallableView(const Delegate* self, Args&&... args) -> Ret
        {
            const auto target = [](const Delegate* self) constexpr noexcept -> F*
            {
                if constexpr (std::is_const_v<F>)
                    return static_cast<F*>(self->_constInstance);
                    
                return static_cast<F*>(self->_instance);
            }(self);

            if constexpr (std::is_void_v<Ret>)
                std::invoke(*target, std::forward<Args>(args)...);
            else
                return std::invoke(*target, std::forward<Args>(args)...);
        }

        template<class F>
        static constexpr DELEGATE_INLINE auto executeEmptyCallable(const Delegate*, Args&&... args) -> Ret
        {
            if constexpr (std::is_void_v<Ret>)
                std::invoke(F{}, std::forward<Args>(args)...);
            else
                return std::invoke(F{}, std::forward<Args>(args)...);
        }

        template<class F>
        static constexpr DELEGATE_INLINE auto executeStatefulCallable(const Delegate* self, Args&&... args) -> Ret
        {
            const auto& target = *std::launder(reinterpret_cast<const F*>(self->_storage));

            if constexpr (std::is_void_v<Ret>)
                std::invoke(target, std::forward<Args>(args)...);
            else
                return std::invoke(target, std::forward<Args>(args)...);
        }

        template<class R2, class... UArgs>
        static constexpr DELEGATE_INLINE auto executeStatelessCallable(const Delegate* self, Args&&... args) -> Ret
        {
            const auto target = reinterpret_cast<R2(*)(UArgs...)>(self->_function);

            if constexpr (std::is_void_v<Ret>)
                std::invoke(target, std::forward<Args>(args)...);
            else
                return std::invoke(target, std::forward<Args>(args)...);
        }

    protected:
        union
        {
            void*       _instance;
            const void* _constInstance;
            AnyTarget   _function;

            alignas(Alignment) std::byte _storage[BufferSize] {};
        };

        using ProxyFunction = Ret(*)(const Delegate*, Args&&...);

        ProxyFunction _proxy { &throwBadCall };
    };

    // Deduction guides

    template<auto Target, class Signature = traits::function_signature<decltype(Target)>>
    Delegate(Function<Target>) -> Delegate<Signature, sizeof(decltype(Target))>;

    template<class T, auto T::*Target, class Signature = traits::function_signature<decltype(Target)>>
    Delegate(MemberFunction<T, Target>) -> Delegate<Signature, sizeof(decltype(Target))>;

    template<class R, class... Args>
    Delegate(R(*)(Args...)) -> Delegate<R(Args...), sizeof(R(*)(Args...))>;

    template<class F, typename = std::enable_if_t<!std::is_base_of_v<Callable, F>>>
    Delegate(F) -> Delegate<traits::function_signature<F>, sizeof(F)>;
} // end of axl namespace


/* ===================================================================== */
/* Undef Delegate macros to clean the global space
/* ===================================================================== */

#undef DELEGATE_FWD
#undef _DELEGATE_CPLUSPLUS_STANDARD
#undef _DELEGATE_COMPILER_IS_SUPPORTED
#undef _DELEGATE_FUNCTION_SIGNATURE 

#if defined(__clang__)
    #pragma clang diagnostic pop
#endif


/* ===================================================================== */
/* Reset MSVC warnings to default state
/* ===================================================================== */

#if defined(_MSC_VER)
    #pragma warning(default:)
#endif
