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

            constexpr bool endsWith(std::string_view suffix) const noexcept { return sv().ends_with(suffix); }
            constexpr bool endsWith(const char* suffix)      const noexcept { return sv().ends_with(suffix); }
            constexpr bool endsWith(const char suffix)       const noexcept { return sv().ends_with(suffix); }

            constexpr bool startsWith(std::string_view prefix) const noexcept { return sv().starts_with(prefix); }
            constexpr bool startsWith(const char* prefix)      const noexcept { return sv().starts_with(prefix); }
            constexpr bool startsWith(const char prefix)       const noexcept { return sv().starts_with(prefix); }

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
        BadDelegateArguments(std::string_view acceptableArguments, std::string_view arguments) noexcept
            : std::runtime_error { makeErrorMessage(acceptableArguments, arguments) }
        {}

    private:
        std::string makeErrorMessage(std::string_view acceptableArguments, std::string_view arguments) noexcept
        {
            using namespace std::string_literals;

            return "Arguments ["s
                    + arguments.data()
                    + "] were given instead of expected ["s
                    + acceptableArguments.data()
                    + "]."s;
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
        template<class... Args>
        constexpr auto operator ()(Args&&... args) const -> Ret
        {
            using ProxyFunction = Ret(*)(const Delegate*, Args&&...);

            constexpr auto arguments     = detail::typeName<Ret(*)(Args...)>();
            constexpr auto argumentsHash = traits::function_hash<Ret(*)(Args...)>;

            const auto function = _wrapper(arguments, argumentsHash, true);
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

            constexpr auto arguments     = detail::typeName<Ret(*)(Args...)>();
            constexpr auto argumentsHash = traits::function_hash<Ret(*)(Args...)>;

            const auto function = _wrapper(arguments, argumentsHash, false);
            const auto proxy    = reinterpret_cast<ProxyFunction>(function);

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
        constexpr void bind() noexcept
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
        [[noreturn]]
        static constexpr auto throwBadCall(const std::string_view, const std::size_t, const bool) -> AnyTarget
        {
            throw BadDelegateCall();
        }

        template<auto Target>
        static constexpr auto executeFunction(
            const std::string_view arguments,
            const std::size_t argumentsHash,
            const bool throwOnMismatch
        ) -> AnyTarget
        {
            using ProxyFunction = traits::delegate_proxy_t<decltype(Target), Delegate>;

            constexpr ProxyFunction proxy = [](const Delegate*, auto&&... args) -> Ret {
                return invoke(Target, DELEGATE_FWD(args)...);
            };

            return getMatchingFunctor<decltype(Target), proxy>(arguments, argumentsHash, throwOnMismatch);
        }

        template<class T, auto T::*Target>
        static constexpr auto executeMemberFunction(
            const std::string_view arguments,
            const std::size_t argumentsHash,
            const bool throwOnMismatch
        ) -> AnyTarget
        {
            using ProxyFunction = traits::delegate_proxy_t<decltype(Target), Delegate>;

            constexpr ProxyFunction proxy = [](const Delegate* self, auto&&... args) -> Ret
            {
                const auto& instance = [](const Delegate* self) constexpr -> T&
                {
                    if constexpr (std::is_const_v<T>)
                        return *static_cast<T*>(self->_constInstance);

                    return *static_cast<T*>(self->_instance);
                }(self);

                return invoke(Target, instance, std::forward<decltype(args)>(args)...);
            };

            return getMatchingFunctor<decltype(Target), proxy>(arguments, argumentsHash, throwOnMismatch);
        }

        template<class F>
        static constexpr auto executeCallableView(
            const std::string_view arguments,
            const std::size_t argumentsHash,
            const bool throwOnMismatch
        ) -> AnyTarget
        {
            using ProxyFunction = traits::delegate_proxy_t<F, Delegate>;

            constexpr ProxyFunction proxy = [](const Delegate* self, auto&&... args) -> Ret
            {
                const auto target = [](const Delegate* self) constexpr -> F*
                {
                    if constexpr (std::is_const_v<F>)
                        return static_cast<F*>(self->_constInstance);
                        
                    return static_cast<F*>(self->_instance);
                }(self);

                return invoke(*target, DELEGATE_FWD(args)...);
            };

            return getMatchingFunctor<F, proxy>(arguments, argumentsHash, throwOnMismatch);
        }

        template<class F>
        static constexpr auto executeEmptyCallable(
            const std::string_view arguments,
            const std::size_t argumentsHash,
            const bool throwOnMismatch
        ) -> AnyTarget
        {
            using ProxyFunction = traits::delegate_proxy_t<F, Delegate>;

            constexpr ProxyFunction proxy = [](const Delegate*, auto&&... args) -> Ret {
                return invoke(F{}, DELEGATE_FWD(args)...);
            };

            return getMatchingFunctor<F, proxy>(arguments, argumentsHash, throwOnMismatch);
        }

        template<class F>
        static constexpr auto executeStatefulCallable(
            const std::string_view arguments,
            const std::size_t argumentsHash,
            const bool throwOnMismatch
        ) -> AnyTarget
        {
            using ProxyFunction = traits::delegate_proxy_t<F, Delegate>;

            constexpr ProxyFunction proxy = [](const Delegate* self, auto&&... args) -> Ret
            {
                const auto& target = *std::launder(reinterpret_cast<const F*>(_storage));

                return invoke(target, DELEGATE_FWD(args)...);
            };

            return getMatchingFunctor<F, proxy>(arguments, argumentsHash, throwOnMismatch);
        }

        template<class R, class... Args>
        static constexpr auto executeStatelessCallable(
            const std::string_view arguments,
            const std::size_t argumentsHash,
            const bool throwOnMismatch
        ) -> AnyTarget
        {
            constexpr auto proxy = +[](const Delegate* self, Args&&... args) -> Ret
            {
                const auto target = reinterpret_cast<R(*)(Args...)>(self->_function);

                return invoke(target, DELEGATE_FWD(args)...);
            };

            return getMatchingFunctor<R(*)(Args...), proxy>(arguments, argumentsHash, throwOnMismatch);
        }

        template<class Target, auto Functor>
        static constexpr auto getMatchingFunctor(
            const std::string_view arguments,
            const std::size_t argumentsHash,
            const bool throwOnMismatch
        ) -> AnyTarget
        {
            using Signature = traits::function_signature<Target>;

            constexpr auto acceptableArguments     = detail::typeName<Signature>();
            constexpr auto acceptableArgumentsHash = traits::function_hash<Target>;

            if (acceptableArgumentsHash != argumentsHash)
            {
                if (throwOnMismatch)
                    throw BadDelegateArguments(acceptableArguments, arguments);

                return nullptr;
            }

            return reinterpret_cast<AnyTarget>(Functor);
        }

        template<class F, class... Args>
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

            alignas(Alignment) std::byte _storage[BufferSize] { };
        };

        using Wrapper = AnyTarget(*)(const std::string_view, const std::size_t, const bool);

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
