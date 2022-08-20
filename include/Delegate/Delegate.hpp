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
            constexpr FixedString() noexcept = default;
            
            constexpr FixedString(std::string_view str) noexcept
            {
                std::copy_n(str.begin(), str.size(), _buffer.begin());
            }

            constexpr FixedString(const char (&str)[Size + 1]) noexcept
            {
                std::copy(std::begin(str), std::end(str), _buffer.begin());
            }

            /* ------------------------------------------------------------- */
            /* Conversions
            /* ------------------------------------------------------------- */
            constexpr operator std::string_view() const noexcept { return { _buffer, Size }; }

            /* ------------------------------------------------------------- */
            /* Properties
            /* ------------------------------------------------------------- */

            [[nodiscard]] constexpr bool empty()  const noexcept { return Size == 0; }
            [[nodiscard]] constexpr auto size()   const noexcept { return Size;      }
            [[nodiscard]] constexpr auto lentgh() const noexcept { return Size;      }

            /* ------------------------------------------------------------- */
            /* String matching
            /* ------------------------------------------------------------- */

            [[nodiscard]] constexpr bool endsWith(std::string_view suffix) const noexcept { return sv().ends_with(suffix); }
            [[nodiscard]] constexpr bool endsWith(const char* suffix)      const noexcept { return sv().ends_with(suffix); }
            [[nodiscard]] constexpr bool endsWith(const char suffix)       const noexcept { return sv().ends_with(suffix); }

            [[nodiscard]] constexpr bool startsWith(std::string_view prefix) const noexcept { return sv().starts_with(prefix); }
            [[nodiscard]] constexpr bool startsWith(const char* prefix)      const noexcept { return sv().starts_with(prefix); }
            [[nodiscard]] constexpr bool startsWith(const char prefix)       const noexcept { return sv().starts_with(prefix); }

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

            constexpr std::string_view sv() const noexcept { return *this; }

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
        constexpr std::uint32_t hash(std::uint32_t prime, const char (&s)[N], std::size_t len = N - 1) noexcept
        {
            // Simple recursive Horner hash (may fail on Clang if 's' is too long)
            return (len <= 1) ? s[0] : (prime * hash(prime, s, len - 1) + s[len - 1]);
        }

        [[nodiscard]]
        constexpr std::uint32_t hash(std::uint32_t prime, const char* s, std::size_t len) noexcept
        {
            std::uint32_t hash = 0;
            
            // Simple Horner hash
            for (std::uint32_t i = 0; i < len; ++i)
                hash = prime * hash + s[i];
            
            return hash;
        }

        [[nodiscard]]
        constexpr std::uint32_t hash(std::uint32_t prime, const std::string_view s) noexcept
        {
            return hash(prime, s.data(), s.size());
        }

        [[nodiscard]]
        constexpr std::uint32_t hash(const std::string_view s) noexcept
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
        constexpr auto typeName() noexcept -> std::string_view
        {
            // Inspired from nameof, check it out here:
            // https://github.com/Neargye/nameof

            constexpr auto name = prettifyName(_DELEGATE_FUNCTION_SIGNATURE);

            return name;
        }

        template<class R, class... Args>
        constexpr auto hashSignature() noexcept -> std::uint32_t
        {
            using F = R(*)(Args...);
            constexpr auto hashedSignature = hash(typeName<F>());

            return hashedSignature;
        }
    } // !namespace detail

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

            using type    = R(Args...);
            using pointer = R(*)(Args...);

            template<class Delegate>
            using proxy = R(*)(const Delegate*, Args&&...);
        };

        template<class R, class... Args>
        struct function_type_impl<R(*)(Args..., ...)>
        {
            static constexpr auto hash = detail::hashSignature<R, std::add_rvalue_reference<Args>...>();

            using type    = R(Args..., ...);
            using pointer = R(*)(Args..., ...);
                
            template<class Delegate>
            using proxy = R(*)(const Delegate*, Args&&..., ...);
        };

        template<class R, class... Args>
        struct function_type_impl<R(*)(Args...) noexcept>
        {
            static constexpr auto hash = detail::hashSignature<R, std::add_rvalue_reference<Args>...>();

            using type    = R(Args...);
            using pointer = R(*)(Args...);
                
            template<class Delegate>
            using proxy = R(*)(const Delegate*, Args&&...);
        };

        template<class R, class... Args>
        struct function_type_impl<R(*)(Args..., ...) noexcept>
        {
            static constexpr auto hash = detail::hashSignature<R, std::add_rvalue_reference<Args>...>();

            using type    = R(Args..., ...);
            using pointer = R(*)(Args..., ...);
                
            template<class Delegate>
            using proxy = R(*)(const Delegate*, Args&&..., ...);
        };

        /* ------------------------------------------------------------- */
        /* Member function pointers
        /* ------------------------------------------------------------- */

        template<class R, class C, class... Args>
        struct function_type_impl<R(C::*)(Args...)>
        {
            static constexpr auto hash = detail::hashSignature<R, std::add_rvalue_reference<Args>...>();

            using type    = R(Args...);
            using pointer = R(*)(Args...);
                
            template<class Delegate>
            using proxy = R(*)(const Delegate*, Args&&...);
        };

        template<class R, class C, class... Args>
        struct function_type_impl<R(C::*)(Args..., ...)>
        {
            static constexpr auto hash = detail::hashSignature<R, std::add_rvalue_reference<Args>...>();

            using type    = R(Args..., ...);
            using pointer = R(*)(Args..., ...);
                
            template<class Delegate>
            using proxy = R(*)(const Delegate*, Args&&..., ...);
        };

        template<class R, class C, class... Args>
        struct function_type_impl<R(C::*)(Args...) noexcept>
        {
            static constexpr auto hash = detail::hashSignature<R, std::add_rvalue_reference<Args>...>();

            using type    = R(Args...);
            using pointer = R(*)(Args...);
                
            template<class Delegate>
            using proxy = R(*)(const Delegate*, Args&&...);
        };

        template<class R, class C, class... Args>
        struct function_type_impl<R(C::*)(Args..., ...) noexcept>
        {
            static constexpr auto hash = detail::hashSignature<R, std::add_rvalue_reference<Args>...>();

            using type    = R(Args..., ...);
            using pointer = R(*)(Args..., ...);
                
            template<class Delegate>
            using proxy = R(*)(const Delegate*, Args&&..., ...);
        };

        /* ------------------------------------------------------------- */
        /* Const member function pointers
        /* ------------------------------------------------------------- */

        template<class R, class C, class... Args>
        struct function_type_impl<R(C::*)(Args...) const>
        {
            static constexpr auto hash = detail::hashSignature<R, std::add_rvalue_reference<Args>...>();

            using type    = R(Args...);
            using pointer = R(*)(Args...);
                
            template<class Delegate>
            using proxy = R(*)(const Delegate*, Args&&...);
        };

        template<class R, class C, class... Args>
        struct function_type_impl<R(C::*)(Args..., ...) const>
        {
            static constexpr auto hash = detail::hashSignature<R, std::add_rvalue_reference<Args>...>();

            using type    = R(Args..., ...);
            using pointer = R(*)(Args..., ...);
                
            template<class Delegate>
            using proxy = R(*)(const Delegate*, Args&&..., ...);
        };

        template<class R, class C, class... Args>
        struct function_type_impl<R(C::*)(Args...) const noexcept>
        {
            static constexpr auto hash = detail::hashSignature<R, std::add_rvalue_reference<Args>...>();

            using type    = R(Args...);
            using pointer = R(*)(Args...);
                
            template<class Delegate>
            using proxy = R(*)(const Delegate*, Args&&...);
        };

        template<class R, class C, class... Args>
        struct function_type_impl<R(C::*)(Args..., ...) const noexcept>
        {
            static constexpr auto hash = detail::hashSignature<R, std::add_rvalue_reference<Args>...>();

            using type    = R(Args..., ...);
            using pointer = R(*)(Args..., ...);
                
            template<class Delegate>
            using proxy = R(*)(const Delegate*, Args&&..., ...);
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
        constexpr decltype(auto) retrospective_cast(F&& callable) noexcept
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

            constexpr virtual auto signature() const noexcept -> FunctionSignature = 0;
            constexpr virtual auto decayedSignature() const noexcept -> FunctionSignature = 0;
            constexpr virtual auto nthArgument(std::size_t) const noexcept -> FunctionArgument = 0;
            constexpr virtual auto numberOfArguments() const noexcept -> std::size_t = 0;
            constexpr virtual bool isCompatibleWith(const IMetaFunction&) const noexcept = 0;
            
            template<class F>
            static constexpr auto fromFunctionType() noexcept
            {
                using FunctionPointer = traits::function_pointer_t<F>;

                return []<typename R, typename... Args>(R (*)(Args...)) constexpr
                {
                    constexpr MetaFunction<R, Args...> metaFunction {};
                
                    return metaFunction;
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

            template<class Arg>
            using ParameterType = std::variant<
                std::integral_constant<InvokedParameterType, InvokedParameterType::Original>,
                std::integral_constant<InvokedParameterType, InvokedParameterType::CV>,
                std::integral_constant<InvokedParameterType, InvokedParameterType::Const>,
                std::integral_constant<InvokedParameterType, InvokedParameterType::Volatile>,

                std::integral_constant<InvokedParameterType, InvokedParameterType::Unreferenced>,
                std::integral_constant<InvokedParameterType, InvokedParameterType::UnreferencedCV>,
                std::integral_constant<InvokedParameterType, InvokedParameterType::UnreferencedConst>,
                std::integral_constant<InvokedParameterType, InvokedParameterType::UnreferencedVolatile>,

                std::integral_constant<InvokedParameterType, InvokedParameterType::LValue>,
                std::integral_constant<InvokedParameterType, InvokedParameterType::LValueCV>,
                std::integral_constant<InvokedParameterType, InvokedParameterType::LValueConst>,
                std::integral_constant<InvokedParameterType, InvokedParameterType::LValueVolatile>,

                std::integral_constant<InvokedParameterType, InvokedParameterType::RValue>,
                std::integral_constant<InvokedParameterType, InvokedParameterType::RValueCV>,
                std::integral_constant<InvokedParameterType, InvokedParameterType::RValueConst>,
                std::integral_constant<InvokedParameterType, InvokedParameterType::RValueVolatile>
            >;

        template<class R, class... Args>
        class MetaFunction : public IMetaFunction
        {
            template<std::size_t N>
            using NthTypeOf = std::tuple_element_t<N, std::tuple<Args...>>;

        public:
            constexpr ~MetaFunction() noexcept = default;

            constexpr auto numberOfArguments() const noexcept -> std::size_t override
            {
                return sizeof...(Args);
            }

            constexpr auto signature() const noexcept -> FunctionSignature override
            {
                constexpr auto signature = detail::typeName<R(*)(Args...)>();

                return signature;
            }

            constexpr auto decayedSignature() const noexcept -> FunctionSignature override
            {
                constexpr auto signature = detail::typeName<R(*)(Args...)>();

                return signature;
            }
            
            constexpr auto nthArgument(std::size_t i) const noexcept -> FunctionArgument override
            {
                return signature().nthArgument(i);
/*                switch (i)
                {
                case 0:  return FunctionArgument::fromConcreteArg<NthTypeOf<0>>();
                case 1:  return FunctionArgument::fromConcreteArg<NthTypeOf<1>>();
                case 2:  return FunctionArgument::fromConcreteArg<NthTypeOf<2>>();
                case 3:  return FunctionArgument::fromConcreteArg<NthTypeOf<3>>();
                case 4:  return FunctionArgument::fromConcreteArg<NthTypeOf<4>>();
                case 5:  return FunctionArgument::fromConcreteArg<NthTypeOf<5>>();
                case 6:  return FunctionArgument::fromConcreteArg<NthTypeOf<6>>();
                case 7:  return FunctionArgument::fromConcreteArg<NthTypeOf<7>>();
                case 8:  return FunctionArgument::fromConcreteArg<NthTypeOf<8>>();
                case 9:  return FunctionArgument::fromConcreteArg<NthTypeOf<9>>();
                case 10: return FunctionArgument::fromConcreteArg<NthTypeOf<10>>();
                case 11: return FunctionArgument::fromConcreteArg<NthTypeOf<11>>();
                case 12: return FunctionArgument::fromConcreteArg<NthTypeOf<12>>();
                case 13: return FunctionArgument::fromConcreteArg<NthTypeOf<13>>();
                case 14: return FunctionArgument::fromConcreteArg<NthTypeOf<14>>();
                case 15: return FunctionArgument::fromConcreteArg<NthTypeOf<15>>();
                case 16: return FunctionArgument::fromConcreteArg<NthTypeOf<16>>();
                case 17: return FunctionArgument::fromConcreteArg<NthTypeOf<17>>();
                case 18: return FunctionArgument::fromConcreteArg<NthTypeOf<18>>();
                case 19: return FunctionArgument::fromConcreteArg<NthTypeOf<19>>();
                case 20: return FunctionArgument::fromConcreteArg<NthTypeOf<20>>();
                default: return FunctionArgument::fromConcreteArg<NthTypeOf<0>>();
                }   */ 

                //static_assert(false, "Functions with more than 20 arguments are not supported yet.");
            }

            enum class CompatibilityError
            {
                None,
                DifferentParameters,
                NotLValue,
                IsLValue,
                IsConst,
                IsVolatile,
                NotCopiable,
                NotMovable,
            };

            constexpr bool isCompatibleWith(const IMetaFunction& rhs) const noexcept override
            {
                if (numberOfArguments() != rhs.numberOfArguments())
                    return false;

                constexpr FunctionSignature ownSignature = axl::detail::typeName<R(*)(Args...)>();

                if (ownSignature == rhs.signature())
                    return true;

                constexpr auto Indices = std::make_index_sequence<sizeof...(Args)>();

                std::size_t i = 0;
                return (... && isArgumentCompatibleWith<Args>(rhs.nthArgument(i++)));


                //return []<std::size_t... I>(const IMetaFunction& rhs, std::index_sequence<I...>) {
                //    return (... && isArgumentCompatibleWith<NthTypeOf<I>>(rhs.nthArgument(I)));
                //}(rhs, Indices);
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

            template<class... Args>
            static constexpr bool all(Args... args) noexcept { return (... && args); }
            
        public:
            static constexpr decltype(auto) getInvokedSignatureType(const IMetaFunction& rhs) noexcept
            {
                constexpr auto Indices = std::make_index_sequence<sizeof...(Args)>();

                return []<std::size_t... I> (const IMetaFunction& rhs, std::index_sequence<I...>) {
                    return [](auto... args) {
                        return toFunctionPointer(getInvokedParameterType<Args>(args)...);
                    }(rhs.nthArgument(I)...);
                }(rhs, Indices);

                //std::size_t i = 0;

                //return toFunctionPointer(getInvokedParameterType<Args>(rhs.nthArgument(i++))...);


                // 1. Split the signature into a list of std::string_view
                // 2. Returns something like this:
                //      to_function_pointer(transform<Args, Indices>(list)...)
            }

        private:
            template<class Arg>
            static constexpr auto getInvokedParameterType(FunctionArgument target) noexcept -> ParameterType<Arg>
            {
                if constexpr (std::is_lvalue_reference_v<Arg>)
                {
                    // The original argument is a lvalue, so the target needs to be a lvalue too,
                    // otherwise it will result in an Undefined Behavior.

                    // The argument is a lvalue, as expected
                    return addCVQualifiersIfRequired<Arg>(target);
                }
        
                if constexpr (std::is_rvalue_reference_v<Arg>)
                {
                    // The original argument is a Arg&&, so the target needs to be either an Arg&& or an Arg,
                    // since lvalues are not transformable into rvalues.

                    if (target.isRValue)
                    {
                        // The argument is a rvalue, as expected
                        return addCVQualifiersIfRequired<Arg>(target);
                    }

                    static_assert(
                        std::is_move_constructible_v<Arg>,
                        "[Delegate] A function taking T&& as a parameter and invoked with T requires from T that it is movable."
                    );

                    // If the function is invoked with a non-reference, it entails that a move will
                    // be made at some point.
                    return addCVQualifiersIfRequired<Arg, std::remove_reference_t<Arg>>(target);
                }

                if (target.isLValue)
                {
                    static_assert(
                        std::is_copy_constructible_v<Arg>,
                        "[Delegate] A function taking T as a parameter and invoked with T& requires from T that it is copyable."
                    );

                    // If the function is invoked with a lvalue reference, it entails that a copy will
                    // be made at some point.
                    return addCVQualifiersIfRequired<Arg, std::add_lvalue_reference_t<Arg>>(target);
                }

                if (target.isRValue)
                {
                    static_assert(
                        std::is_move_constructible_v<Arg>,
                        "[Delegate] A function taking T as a parameter and invoked with T&& requires from T that it is movable."
                    );

                    // If the function is invoked with a rvalue reference, it entails that a move will
                    // be made at some point.
                    return addCVQualifiersIfRequired<Arg, std::add_rvalue_reference_t<Arg>>(target);
                }

                // The argument is not a reference, as expected
                return addCVQualifiersIfRequired<Arg>(target);
            }

            template<class Arg, class InvokedArg = Arg>
            // requires std::same_as<std::decay_t<Arg>, std::decay_t<InvokedArg>>
            static constexpr auto addCVQualifiersIfRequired(FunctionArgument target) noexcept -> ParameterType<Arg>
            {
                if (target.isConst && target.isVolatile)
                {
                    constexpr auto type = getInvokedParameterType<InvokedArg, true, true>();

                    return ParameterType<Arg>(std::in_place_index<static_cast<std::size_t>(type)>);
                }
                if (target.isConst)
                {
                    constexpr auto type = getInvokedParameterType<InvokedArg, true, false>();

                    return ParameterType<Arg>(std::in_place_index<static_cast<std::size_t>(type)>);
                }
                if (target.isVolatile)
                {
                    constexpr auto type = getInvokedParameterType<InvokedArg, false, true>();

                    return ParameterType<Arg>(std::in_place_index<static_cast<std::size_t>(type)>);
                }

                constexpr auto type = getInvokedParameterType<InvokedArg, false, false>();

                return ParameterType<Arg>(std::in_place_index<static_cast<std::size_t>(type)>);
            }

            template<class InvokedArg, bool IsConst, bool IsVolatile>
            static constexpr auto getInvokedParameterType() noexcept -> InvokedParameterType
            {
                if constexpr (std::is_rvalue_reference_v<InvokedArg>)
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
                else if constexpr (std::is_lvalue_reference_v<InvokedArg>)
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

            // TODO: Try to serialize the qualifiers to get a FixedString and use it here in a lambda to get the right
            // type
            static constexpr decltype(auto) toFunctionPointer(ParameterType<Args>... args) noexcept
            {
                /*using OpaqueTuple = std::tuple<decltype(std::visit([](auto v) -> decltype(auto) { return *v; }, args))...>;

                return []<class... Parameters>(std::tuple<Parameters...>*) {
                    return static_cast<R(*)(Parameters...)>(nullptr);
                }(reinterpret_cast<OpaqueTuple*>(nullptr));*/

                // const auto opaqueTuple = std::make_tuple(std::visit([](auto v) -> auto { return v; }, args)...);
                using OpaqueTuple = std::tuple<decltype(std::visit([](auto v) -> decltype(auto) { return v; }, args))... > ;

                return []<class... Parameters>(std::tuple<Parameters...>*) {
                    return static_cast<R(*)(std::variant_alternative_t<static_cast<std::size_t>(Parameters::value), InvokedParameter<Args>>...)>(nullptr);
                }(reinterpret_cast<OpaqueTuple*>(nullptr));
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
    inline constexpr auto bind() noexcept -> Function<Target> { return {}; }

    template<auto Target, class T>
    inline constexpr auto bind(T* instance) noexcept -> MemberFunction<T, Target> { return { instance }; }


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
        constexpr Delegate() noexcept = default;

        template<class R2, class... Args>
        explicit Delegate(std::function<R2(Args...)>&& target) noexcept
        {
            bind(DELEGATE_FWD(target));
        }

        template<auto Target>
        constexpr Delegate(Function<Target>) noexcept
            : _wrapper { &executeFunction<Target> }
        {}

        template<class T, auto T::*Target>
        constexpr Delegate(MemberFunction<const T, Target> target) noexcept
            : _constInstance { target.instance                         }
            , _wrapper       { &executeMemberFunction<const T, Target> }
        {}

        template<class T, auto T::*Target>
        constexpr Delegate(MemberFunction<T, Target> target) noexcept
            : _instance { target.instance                   }
            , _wrapper  { &executeMemberFunction<T, Target> }
        {}

        template<class F,
                 typename = std::enable_if_t<(
                    !traits::is_function_v<F>
                 )>>
        constexpr Delegate(F* target) noexcept
            : _instance { target                  }
            , _wrapper  { &executeCallableView<F> }
        {}
            
        template<class F,
                 typename = std::enable_if_t<(
                    !traits::is_function_v<F>
                 )>>
        constexpr Delegate(const F* target) noexcept
            : _constInstance { target                        }
            , _wrapper       { &executeCallableView<const F> }
        {}

        template<class F,
                 std::enable_if_t<(
                    std::is_empty_v<F> &&
                    std::is_default_constructible_v<F>
                 ), int> = 0>
        constexpr Delegate(F&&) noexcept
            : _wrapper { &executeEmptyCallable<F> }
        {}

        template<class F,
                 std::enable_if_t<(
                     traits::is_likely_captureless_lambda_v<F>
                 ), int> = 0>
        constexpr Delegate(F&& target) noexcept
            : Delegate { detail::retrospective_cast(std::forward<F>(target)) }
        {}

        template<class F,
                 std::enable_if_t<(
                     traits::is_likely_capturing_lambda_v<F> &&
                     is_storable_v<std::decay_t<F>>
                 ), int> = 0>
        constexpr Delegate(F&& target) noexcept
            : _wrapper { &executeStatefulCallable<F> }
        {
            new (_storage) F(std::forward<F>(target));
        }

        template<class R, class... Args>
        constexpr Delegate(R(*target)(Args...)) noexcept
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
        constexpr void operator=(Function<Target>) noexcept
        {
            bind<Target>();
        }
            
        template<class T, auto T::*Target>
        constexpr void operator=(MemberFunction<const T, Target> target) noexcept
        {
            bind<Target>(target.instance);
        }

        template<class T, auto T::* Target>
        constexpr void operator=(MemberFunction<T, Target> target) noexcept
        {
            bind<Target>(target.instance);
        }

        template<class F,
                 typename = std::enable_if_t<(
                    !traits::is_function_v<F>
                 )>>
        constexpr void operator=(F* target) noexcept
        {
            bind(target);
        }
            
        template<class F,
                 typename = std::enable_if_t<(
                    !traits::is_function_v<F>
                 )>>
        constexpr void operator=(const F* target) noexcept
        {
            bind(target);
        }

        template<class F,
                 std::enable_if_t<(
                    std::is_empty_v<F> &&
                    std::is_default_constructible_v<F>
                 ), int> = 0>
        constexpr void operator=(F&& target) noexcept
        {
            bind<F>();
        }

        template<class F,
                 std::enable_if_t<(
                     traits::is_likely_captureless_lambda_v<F>
                 ), int> = 0>
        constexpr void operator=(F&& target) noexcept
        {
            bind(detail::retrospective_cast(std::forward<F>(target)));
        }

        template<class F,
                 std::enable_if_t<(
                    traits::is_likely_capturing_lambda_v<F> &&
                    is_storable_v<std::decay_t<F>>
                 ), int> = 0>
        constexpr void operator=(F&& target) noexcept
        {
            bind(std::forward<F>(target));
        }

        template<class R, class... Args>
        constexpr void operator=(R(*target)(Args...)) noexcept
        {
            bind(target);
        }

    public:
        using MetaFunctionBuilder = const IMetaFunction& (*)();

        template<class... Args>
        constexpr auto operator ()(Args&&... args) const -> Ret
        {
            using ProxyFunction = Ret(*)(const Delegate*, Args&&...);

            constexpr MetaFunction<Ret, Args...> invokedFunction {};

            std::cout << std::format("Real Proxy => [{}].", detail::typeName<ProxyFunction>()) << std::endl;

            std::cout << "Real Proxy Signature => " << invokedFunction.signature().representation() << std::endl;

            const auto function = _wrapper(invokedFunction, true);
            const auto proxy    = reinterpret_cast<ProxyFunction>(function);

            return proxy(this, std::forward<Args>(args)...);
        }

        constexpr void reset() noexcept { _wrapper = &throwBadCall; }

        explicit constexpr operator bool() const noexcept { return hasTarget(); }

    public:
        template<class R>
        [[nodiscard]]
        constexpr bool hasReturnType() const noexcept
        {
            constexpr bool hasReturnType = std::is_same_v<R, Ret>;

            return hasReturnType;
        }

        template<class... Args>
        [[nodiscard]]
        constexpr bool isInvokable() const noexcept
        {
            using ProxyFunction = Ret(*)(const Delegate*, Args&&...);

            constexpr MetaFunction<Ret, Args...> invokedFunction{};

            const auto function = _wrapper(invokedFunction, false);
            const auto proxy = reinterpret_cast<ProxyFunction>(function);

            return (proxy != nullptr);
        }

        [[nodiscard]]
        constexpr bool hasTarget() const noexcept
        {
            return _wrapper != &throwBadCall;
        }

        template<auto Target>
        [[nodiscard]]
        constexpr bool hasTarget() const noexcept
        {
            return _wrapper == &executeFunction<Target>;
        }

        template<auto Target, class T,
                 typename = std::enable_if_t<(
                     std::is_member_function_pointer_v<decltype(Target)>
                 )>>
        [[nodiscard]]
        constexpr bool hasTarget(const T* instance) const noexcept
        {
            return _wrapper       == &executeMemberFunction<const T, Target>
                && _constInstance == instance;
        }

        template<auto Target, class T,
                 typename = std::enable_if_t<(
                     std::is_member_function_pointer_v<decltype(Target)>
                 )>>
        [[nodiscard]]
        constexpr bool hasTarget(T* instance) const noexcept
        {
            return _wrapper  == &executeMemberFunction<T, Target>
                && _instance == instance;
        }

        template<class F,
                 typename = std::enable_if_t<(
                     !traits::is_function_v<F>
                 )>>
        [[nodiscard]]
        constexpr bool hasTarget(const F* instance) const noexcept
        {
            return _wrapper       == &executeCallableView<const F>
                && _constInstance == instance;
        }

        template<class F,
                 typename = std::enable_if_t<(
                     !traits::is_function_v<F>
                 )>>
        [[nodiscard]]
        constexpr bool hasTarget(F* instance) const noexcept
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
        constexpr bool hasTarget() const noexcept
        {
            return _wrapper == &executeEmptyCallable<F>;
        }

        template<class F,
                 typename = std::enable_if_t<(
                     traits::is_likely_capturing_lambda_v<F> &&
                     is_storable_v<std::decay_t<F>>
                 )>>
        [[nodiscard]]
        constexpr bool hasTarget(const F& target) const noexcept
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
        constexpr bool hasTarget(R2(*target)(Args...)) const noexcept
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
        constexpr void bind() noexcept
        {
            _wrapper = &executeFunction<Target>;
        }
            
        template<auto Target, class T>
        constexpr void bind(const T* instance) noexcept
        {
            _constInstance = instance;
            _wrapper       = &executeMemberFunction<const T, Target>;
        }

        template<auto Target, class T>
        constexpr void bind(T* instance) noexcept
        {
            _instance = instance;
            _wrapper  = &executeMemberFunction<T, Target>;
        }

        template<class F,
                 typename = std::enable_if_t<(
                     !traits::is_function_v<F>
                 )>>
        constexpr void bind(F* target) noexcept
        {
            _instance = target;
            _wrapper  = &executeCallableView<F>;
        }
            
        template<class F,
                 typename = std::enable_if_t<(
                     !traits::is_function_v<F>
                 )>>
        constexpr void bind(const F* target) noexcept
        {
            _constInstance = target;
            _wrapper       = &executeCallableView<const F>;
        }

        template<class F,
                 std::enable_if_t<(
                     std::is_empty_v<F> &&
                     std::is_default_constructible_v<F>
                 ), int> = 0>
        constexpr void bind(F&&) noexcept
        {
            _wrapper = &executeEmptyCallable<F>;
        }

        template<class F,
                 std::enable_if_t<(
                     traits::is_likely_captureless_lambda_v<F>
                 ), int> = 0>
        constexpr void bind(F&& target) noexcept
        {
            bind(detail::retrospective_cast(std::forward<F>(target)));
        }

        template<class F,
                 std::enable_if_t<(
                     traits::is_likely_capturing_lambda_v<F> &&
                     is_storable_v<std::decay_t<F>>
                 ), int> = 0>
        constexpr void bind(F&& target) noexcept
        {
            _wrapper = &executeStatefulCallable<F>;

            new (_storage) F(std::forward<F>(target));
        }

        template<class R, class... Args>
        constexpr void bind(R(*target)(Args...)) noexcept
        {
            _function = reinterpret_cast<AnyTarget>(target);
            _wrapper  = &executeStatelessCallable<R, Args...>;
        }

    private:
        using Signature = detail::FixedString<512U>;

        template<detail::FixedString S>
        struct Meta {};

        [[noreturn]]
        static constexpr auto throwBadCall(const IMetaFunction&, const bool) -> AnyTarget
        {
            throw BadDelegateCall();
        }

        template<auto Target>
        static constexpr auto executeFunction(
            const IMetaFunction& invokedFunction,
            const bool throwOnMismatch
        ) -> AnyTarget
        {
            constexpr auto proxy = []<class... Invoked>(const Delegate * self, Invoked&&... args) constexpr -> Ret {
                return invoke(Target, DELEGATE_FWD(args)...);
            };

            return getProxyFunction<decltype(Target), decltype(proxy)>(invokedFunction, throwOnMismatch);
        }

        template<class T, auto T::*Target>
        static constexpr auto executeMemberFunction(
            const IMetaFunction& invokedFunction,
            const bool throwOnMismatch
        ) -> AnyTarget
        {
            constexpr auto proxy = []<class... Invoked>(const Delegate * self, Invoked&&... args) constexpr -> Ret
            {
                const auto& instance = [](const Delegate* self) constexpr -> T&
                {
                    if constexpr (std::is_const_v<T>)
                        return *static_cast<T*>(self->_constInstance);

                    return *static_cast<T*>(self->_instance);
                }(self);

                return invoke(Target, instance, std::forward<decltype(args)>(args)...);
            };

            return getProxyFunction<decltype(Target), decltype(proxy)>(invokedFunction, throwOnMismatch);
        }

        template<class F>
        static constexpr auto executeCallableView(
            const IMetaFunction& invokedFunction,
            const bool throwOnMismatch
        ) -> AnyTarget
        {
            constexpr auto proxy = []<class... Invoked>(const Delegate * self, Invoked&&... args) constexpr -> Ret
            {
                const auto target = [](const Delegate* self) constexpr -> F*
                {
                    if constexpr (std::is_const_v<F>)
                        return static_cast<F*>(self->_constInstance);
                        
                    return static_cast<F*>(self->_instance);
                }(self);

                return invoke(*target, DELEGATE_FWD(args)...);
            };

            return getProxyFunction<F, decltype(proxy)>(invokedFunction, throwOnMismatch);
        }

        template<class F>
        static constexpr auto executeEmptyCallable(
            const IMetaFunction& invokedFunction,
            const bool throwOnMismatch
        ) -> AnyTarget
        {
            constexpr auto proxy = []<class... Invoked>(const Delegate * self, Invoked&&... args) constexpr -> Ret {
                return invoke(F{}, DELEGATE_FWD(args)...);
            };

            return getProxyFunction<F, decltype(proxy)>(invokedFunction, throwOnMismatch);
        }

        template<class F>
        static constexpr auto executeStatefulCallable(
            const IMetaFunction& invokedFunction,
            const bool throwOnMismatch
        ) -> AnyTarget
        {
            constexpr auto proxy = []<class... Invoked>(const Delegate * self, Invoked&&... args) constexpr -> Ret
            {
                const auto& target = *std::launder(reinterpret_cast<const F*>(_storage));

                return invoke(target, DELEGATE_FWD(args)...);
            };

            return getProxyFunction<F, decltype(proxy)>(invokedFunction, throwOnMismatch);
        }

        template<class R, class... Args>
        static constexpr auto executeStatelessCallable(
            const IMetaFunction& invokedFunction,
            const bool throwOnMismatch
        ) -> AnyTarget
        {
            constexpr auto proxy = []<class... Invoked>(const Delegate* self, Invoked&&... args) constexpr -> Ret
            {
                const auto target = reinterpret_cast<R(*)(Args...)>(self->_function);

                return invoke(target, DELEGATE_FWD(args)...);
            };

            return getProxyFunction<R(*)(Args...), decltype(proxy)>(invokedFunction, throwOnMismatch);
        }

        template<class Target, class GenericProxy>
        static constexpr auto getProxyFunction(
            const IMetaFunction& invoked,
            const bool throwOnMismatch
        )
        {
            constexpr auto target = IMetaFunction::fromFunctionType<Target>();
                
            if (!target.isCompatibleWith(invoked))
            {
                if (throwOnMismatch)
                    throw BadDelegateArguments(target.signature(), invoked.signature());

                return reinterpret_cast<AnyTarget>(nullptr);
            }

            using InvokedSignature = decltype(target.getInvokedSignatureType(invoked));
            using FunctionProxy    = traits::delegate_proxy_t<InvokedSignature, Delegate>;

            constexpr FunctionProxy proxy = GenericProxy();

            constexpr auto test = IMetaFunction::fromFunctionType<FunctionProxy>();
            std::cout << std::format("Target => [{}].", target.signature().representation()) << std::endl;
            std::cout << std::format("FunctionProxy => [{}].", test.signature().representation()) << std::endl;

            return reinterpret_cast<AnyTarget>(proxy);
        }

        template<class F, class... Args>
        static constexpr auto invoke(F&& target, Args&&... args) -> Ret
        {
            static_assert(
                std::is_invocable_v<F, Args...>,
                "[Delegate] You are trying to invoke a function with non compatible arguments."
            );

            if constexpr (std::is_void_v<Ret>)
                std::invoke(DELEGATE_FWD(target), DELEGATE_FWD(args)...);
            else
                return std::invoke(DELEGATE_FWD(target), DELEGATE_FWD(args)...);
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
        constexpr Delegate() noexcept = default;

        template<class R2, class... UArgs,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, std::function<R2(UArgs...)>, Args...>
                 )>>
        explicit Delegate(std::function<R2(UArgs...)>&& target) noexcept
        {
            bind(DELEGATE_FWD(target));
        }

        template<auto Target,
                 typename = std::enable_if_t<(
                    std::is_invocable_r_v<Ret, decltype(Target), Args...>
                 )>>
        constexpr Delegate(Function<Target>) noexcept
            : _proxy { &executeFunction<Target> }
        {}

        template<class T, auto T::*Target,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, decltype(Target), const T&, Args...>
                 )>>
        constexpr Delegate(MemberFunction<const T, Target> target) noexcept
            : _constInstance { target.instance                         }
            , _proxy         { &executeMemberFunction<const T, Target> }
        {}

        template<class T, auto T::* Target,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, decltype(Target), T&, Args...>
                 )>>
        constexpr Delegate(MemberFunction<T, Target> target) noexcept
            : _instance { target.instance                   }
            , _proxy    { &executeMemberFunction<T, Target> }
        {}

        template<class F,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, F, Args...> &&
                     !traits::is_function_v<F>
                 )>>
        constexpr Delegate(F* target) noexcept
            : _instance { target                  }
            , _proxy    { &executeCallableView<F> }
        {}
            
        template<class F,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, const F, Args...> &&
                     !traits::is_function_v<F>
                 )>>
        constexpr Delegate(const F* target) noexcept
            : _constInstance { target                        }
            , _proxy         { &executeCallableView<const F> }
        {}

        template<class F,
                 std::enable_if_t<(
                     std::is_empty_v<F> &&
                     std::is_default_constructible_v<F> &&
                     std::is_invocable_r_v<Ret, F, Args...>
                 ), int> = 0>
        constexpr Delegate(F&&) noexcept
            : _proxy { &executeEmptyCallable<F> }
        {}

        template<class F,
                 std::enable_if_t<(
                     traits::is_likely_captureless_lambda_v<F>
                 ), int> = 0>
        constexpr Delegate(F&& target) noexcept
            : Delegate { detail::retrospective_cast(std::forward<F>(target)) }
        {}

        template<class F,
                 std::enable_if_t<(
                     traits::is_likely_capturing_lambda_v<F> &&
                     is_storable_v<std::decay_t<F>> &&
                     std::is_invocable_r_v<Ret, const F&, Args...>
                 ), int> = 0>
        constexpr Delegate(F&& target) noexcept
            : _proxy { &executeStatefulCallable<F> }
        {
            new (_storage) F(std::forward<F>(target));
        }

        template<class R, class... UArgs,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, R(*)(UArgs...), Args...>
                 )>>
        constexpr Delegate(R(*target)(UArgs...)) noexcept
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
        constexpr void operator=(Function<Target>) noexcept
        {
            bind<Target>();
        }
            
        template<class T, auto T::*Target,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, decltype(Target), const T&, Args...>
                 )>>
        constexpr void operator=(MemberFunction<const T, Target> target) noexcept
        {
            bind<Target>(target.instance);
        }

        template<class T, auto T::*Target,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, decltype(Target), T&, Args...>
                 )>>
        constexpr void operator=(MemberFunction<T, Target> target) noexcept
        {
            bind<Target>(target.instance);
        }

        template<class F,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, F, Args...> &&
                     !traits::is_function_v<F>
                 )>>
        constexpr void operator=(F* target) noexcept
        {
            bind(target);
        }
            
        template<class F,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, const F, Args...> &&
                     !traits::is_function_v<F>
                 )>>
        constexpr void operator=(const F* target) noexcept
        {
            bind(target);
        }

        template<class F,
                 std::enable_if_t<(
                     std::is_empty_v<F>&&
                     std::is_default_constructible_v<F> &&
                     std::is_invocable_r_v<Ret, F, Args...>
                 ), int> = 0>
        constexpr void operator=(F&&) noexcept
        {
            bind<F>();
        }

        template<class F,
                 std::enable_if_t<(
                     traits::is_likely_captureless_lambda_v<F>
                 ), int> = 0>
        constexpr void operator=(F&& target) noexcept
        {
            bind(std::forward<F>(target));
        }

        template<class F,
                 std::enable_if_t<(
                     traits::is_likely_capturing_lambda_v<F> &&
                     is_storable_v<std::decay_t<F>> &&
                     std::is_invocable_r_v<Ret, const F&, Args...>
                 ), int> = 0>
        constexpr void operator=(F&& target) noexcept
        {
            bind(std::forward<F>(target));
        }

        template<class R, class... UArgs,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, R(*)(UArgs...), Args...>
                 )>>
        constexpr void operator=(R(*target)(UArgs...)) noexcept
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

        constexpr void reset() noexcept { _proxy = &throwBadCall; }

        explicit constexpr operator bool() const noexcept { return hasTarget(); }

    public:
        template<class R>
        [[nodiscard]]
        constexpr bool hasReturnType() const noexcept
        {
            constexpr bool hasReturnType = std::is_same_v<R, Ret>;

            return hasReturnType;
        }

        template<class... UArgs>
        [[nodiscard]]
        constexpr bool isInvokable() const noexcept
        {
            constexpr bool isInvokable = std::is_invocable_r_v<Ret, Ret(*)(Args...), UArgs...>;

            return isInvokable;
        }

        [[nodiscard]]
        constexpr bool hasTarget() const noexcept
        {
            return _proxy != &throwBadCall;
        }

        template<auto Target,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, decltype(Target), Args...>
                 )>>
        [[nodiscard]]
        constexpr bool hasTarget() const noexcept
        {
            return _proxy == &executeFunction<Target>;
        }

        template<auto Target, class T,
                 typename = std::enable_if_t<(
                     std::is_member_function_pointer_v<decltype(Target)> &&
                     std::is_invocable_r_v<Ret, decltype(Target), Args...>
                 )>>
        [[nodiscard]]
        constexpr bool hasTarget(const T* instance) const noexcept
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
        constexpr bool hasTarget(T* instance) const noexcept
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
        constexpr bool hasTarget(const F* instance) const noexcept
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
        constexpr bool hasTarget(F* instance) const noexcept
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
        constexpr bool hasTarget() const noexcept
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
        constexpr bool hasTarget(const F& target) const noexcept
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
        constexpr bool hasTarget(R2(*target)(UArgs...)) const noexcept
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
        constexpr void bind() noexcept
        {
            _proxy = &executeFunction<Target>;
        }

            
        template<auto Target, class T,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, decltype(Target), const T&, Args...>
                 )>>
        constexpr void bind(const T* instance) noexcept
        {
            _constInstance = instance;
            _proxy         = &executeMemberFunction<const T, Target>;
        }

        template<auto Target, class T,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, decltype(Target), T&, Args...>
                 )>>
        constexpr void bind(T* instance) noexcept
        {
            _instance = instance;
            _proxy    = &executeMemberFunction<T, Target>;
        }

        template<class F,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, F, Args...> &&
                     !traits::is_function_v<F>
                 )>>
        constexpr void bind(F* target) noexcept
        {
            _instance = target;
            _proxy    = &executeCallableView<F>;
        }
            
        template<class F,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, const F, Args...> &&
                     !traits::is_function_v<F>
                 )>>
        constexpr void bind(const F* target) noexcept
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
        constexpr void bind() noexcept
        {
            _proxy = &executeEmptyCallable<F>;
        }

        template<class F,
                 std::enable_if_t<(
                     traits::is_likely_captureless_lambda_v<F> &&
                     std::is_invocable_r_v<Ret, F, Args...>
                 ), int> = 0>
        constexpr void bind(F&& target) noexcept
        {
            bind(detail::retrospective_cast(std::forward<F>(target)));
        }

        template<class F,
                 std::enable_if_t<(
                     traits::is_likely_capturing_lambda_v<F> &&
                     is_storable_v<std::decay_t<F>> &&
                     std::is_invocable_r_v<Ret, const F&, Args...>
                 ), int> = 0>
        constexpr void bind(F&& target) noexcept
        {
            _proxy = &executeStatefulCallable<F>;

            new (_storage) F(std::forward<F>(target));
        }

        template<class R, class... UArgs,
                 typename = std::enable_if_t<(
                     std::is_invocable_r_v<Ret, R(*)(UArgs...), Args...>
                 )>>
        constexpr void bind(R(*target)(UArgs...)) noexcept
        {
            _function = reinterpret_cast<void (*)()>(target);
            _proxy    = &executeStatelessCallable<R, UArgs...>;
        }

    private:
        [[noreturn]]
        static constexpr auto throwBadCall(const Delegate*, Args&&...) -> Ret
        {
            throw BadDelegateCall();
        }

        template<auto Target>
        static constexpr auto executeFunction(const Delegate*, Args&&... args) -> Ret
        {
            return invoke(Target, std::forward<Args>(args)...);
        }

        template<class T, auto T::*Target>
        static constexpr auto executeMemberFunction(const Delegate* self, Args&&... args) -> Ret
        {
            const auto& instance = [](const Delegate* self) constexpr noexcept -> T&
            {
                if constexpr (std::is_const_v<T>)
                    return *static_cast<T*>(self->_constInstance);

                return *static_cast<T*>(self->_instance);
            }(self);

            return invoke(Target, instance, std::forward<Args>(args)...);
        }

        template<class F>
        static constexpr auto executeCallableView(const Delegate* self, Args&&... args) -> Ret
        {
            const auto target = [](const Delegate* self) constexpr noexcept -> F*
            {
                if constexpr (std::is_const_v<F>)
                    return static_cast<F*>(self->_constInstance);
                    
                return static_cast<F*>(self->_instance);
            }(self);

            return invoke(*target, std::forward<Args>(args)...);
        }

        template<class F>
        static constexpr auto executeEmptyCallable(const Delegate*, Args&&... args) -> Ret
        {
            return invoke(F{}, std::forward<Args>(args)...);
        }

        template<class F>
        static constexpr auto executeStatefulCallable(const Delegate* self, Args&&... args) -> Ret
        {
            const auto& target = *std::launder(reinterpret_cast<const F*>(self->_storage));

            return invoke(target, std::forward<Args>(args)...);
        }

        template<class R2, class... UArgs>
        static constexpr auto executeStatelessCallable(const Delegate* self, Args&&...args) -> Ret
        {
            const auto target = reinterpret_cast<R2(*)(UArgs...)>(self->_function);

            return invoke(target, std::forward<Args>(args)...);
        }

        template<class F>
        static constexpr auto invoke(F&& target, Args&&... args) -> Ret
        {
            if constexpr (std::is_void_v<Ret>)
                std::invoke(DELEGATE_FWD(target), DELEGATE_FWD(args)...);
            else
                return std::invoke(DELEGATE_FWD(target), DELEGATE_FWD(args)...);
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
