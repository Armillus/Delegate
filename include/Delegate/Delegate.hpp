/// @author  Armillus (Axel M. | https://github.com/Armillus)
/// @project Delegate

// Copyright (c) 2021 Axel Millot <millotaxel71@gmail.com>.
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

// C++ includes
#include <functional>
#include <string_view>
#include <type_traits>
#include <stdexcept>

#if defined(__clang__)
    #pragma clang diagnostic push
#endif

// Check if the standard is >= C++ 17
#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
    #undef  _AXL_STANDARD_SUPPORTED
    #define _AXL_STANDARD_SUPPORTED 1
#endif

// Check compiler compatibility (Clang >= 5, MSVC >= 15.3, GCC >= 7)
#if defined(__clang__) && __clang_major__ >= 5 || defined(__GNUC__) && __GNUC__ >= 7 || defined(_MSC_VER) && _MSC_VER >= 1910
    #undef  _AXL_COMPILER_SUPPORTED
    #define _AXL_COMPILER_SUPPORTED 1
#endif

static_assert(_AXL_STANDARD_SUPPORTED && _AXL_COMPILER_SUPPORTED,
              "Delegate requires at least C++ 17 and a compatible compiler.");

#if defined(__clang__) || defined(__GNUC__)
    #define _AXL_FUNCTION_SIGNATURE __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
    #define _AXL_FUNCTION_SIGNATURE __FUNCSIG__
#endif

namespace axl
{
    // Specific code needed for Delegate, do not use it out of this file
    namespace detail
    {
        template<std::size_t N>
        [[nodiscard]] constexpr std::uint32_t hash(std::uint32_t prime, const char (&s)[N], std::size_t len = N - 1) noexcept
        {
            // Simple recursive Horner hash (may fail on Clang)
            return (len <= 1) ? s[0] : (prime * hash(prime, s, len - 1) + s[len - 1]);
        }

        [[nodiscard]] constexpr std::uint32_t hash(std::uint32_t prime, const char* s, std::size_t len) noexcept
        {
            std::uint32_t hash = 0;
            
            // Simple Horner hash
            for (std::uint32_t i = 0; i < len; ++i)
                hash = prime * hash + s[i];
            
            return hash;
        }

        [[nodiscard]] constexpr std::uint32_t hash(std::uint32_t prime, const std::string_view& s) noexcept
        {
            return hash(prime, s.data(), s.size());
        }

        #define HASH_DEFAULT_PRIME_NUMBER 31

        [[nodiscard]] constexpr std::uint32_t hash(const std::string_view& s) noexcept
        {
            return hash(HASH_DEFAULT_PRIME_NUMBER, s);
        }

        #undef HASH_DEFAULT_PRIME_NUMBER

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

        template<typename T>
        constexpr auto typeName() noexcept -> std::string_view
        {
            // Inspired from nameof, check it out here:
            // https://github.com/Neargye/nameof

            constexpr auto size = sizeof(_AXL_FUNCTION_SIGNATURE);
            constexpr auto name = prettifyName({ _AXL_FUNCTION_SIGNATURE, size });
            return name;
        }

        template<typename Ret, typename... Args, typename F = Ret (*)(Args...)>
        constexpr auto hashFunctionSignature() noexcept -> std::uint32_t
        {
            // Declare 'hashedSignature' as constexpr before returning it forces 
            // the compiler to create it at compile time.

            constexpr auto hashedSignature = hash(typeName<F>());
            return hashedSignature;
        }

        // Specific axl traits, mostly used to decay lambdas as function pointers

        template<typename F>
        struct function_type : public function_type<decltype(&F::operator())> {};

        template<typename Ret, typename Class, typename... Args>
        struct function_type<Ret (Class::*)(Args...) const>
        {
            using type    = std::function<Ret (Args...)>;
            using pointer = Ret (*)(Args...);
        };

        template<typename F>
        using function_type_t = typename function_type<F>::type;

        template<typename F>
        using function_type_p = typename function_type<F>::pointer;

        // More on dedicated / retrospective casts for lambdas:
        // https://www.py4u.net/discuss/63381
        // https://stackoverflow.com/questions/13358672/how-to-convert-a-lambda-to-an-stdfunction-using-templates
        // https://gist.github.com/khvorov/cd626ea3685fd5e8bf14

        template<typename F>
        constexpr decltype(auto) retrospective_cast(F&& func)
        {
            // Capturing lambdas can't be converted into function pointers,
            // and even by putting them in a std::function, we still have
            // a custom pointer incompatible with how Delegate works.

            static_assert(std::is_convertible_v<F, function_type_p<F>>,
                          "Delegate: capturing lambdas are not supported, "
                          "refer to the documentation for more information.");
            
            if constexpr (std::is_convertible_v<F, function_type_p<F>>)
            {
                // TODO: more tests to check if the behavior is always correct
                return static_cast<function_type_p<F>>(func);
            }
            else
            {
                // It won't work (because of the internal type of the lamdba),
                // no more than function_type_t<F>(func).
                return std::function { func };  
            }
        }
    } // end of namespace detail

    class BadDelegateArguments final : public std::runtime_error
    {
    public:
        BadDelegateArguments(std::string_view&& invalidSignature) noexcept
            : std::runtime_error { std::string("Delegate called with the following bad signature: ")
                                   + invalidSignature.data() }
        {}
    };

    // Reduce the size of Delegate of 4 bytes by setting the memory alignment to 1
    #pragma pack(push, 1)

    class Delegate
    {
    public:
        // Mandatory to be stored in a map
        Delegate() noexcept = default;

        ~Delegate() noexcept = default;

        template<typename F>
        Delegate(F&& function) noexcept
        {
            // Deduce lambdas types
            assign(detail::retrospective_cast(std::forward<F>(function)));
        }

        template<typename Ret, typename... Args>
        Delegate(std::function<Ret (Args...)>&& function) noexcept
            : _hashedSignature { detail::hashFunctionSignature<Ret, std::decay_t<Args>...>() }
        { 
            assign(std::forward<decltype(function)>(function));
        }

        template<typename Ret, typename... Args>
        Delegate(Ret (*function)(Args...)) noexcept
            : _hashedSignature { detail::hashFunctionSignature<Ret, std::decay_t<Args>...>() }
        {
            assign(function);
        }

        Delegate(Delegate&& other) noexcept
            : _functor         { std::exchange(other._functor, nullptr) }
            , _handle          { std::exchange(other._handle, nullptr)  }
            , _hashedSignature { other._hashedSignature                 }
        {}

        Delegate& operator=(Delegate&& other) noexcept
        {
            _functor = std::exchange(other._functor, nullptr);
            _handle  = std::exchange(other._handle, nullptr);
            
            _hashedSignature = other._hashedSignature;
            return *this;
        }

        // Makes Delegate non copyable
        
        Delegate(Delegate&) noexcept = delete;
        Delegate& operator=(Delegate&) noexcept = delete;

    public:
        template<typename F>
        void operator=(F&& function) noexcept
        {
            if constexpr (std::is_pointer_v<F>)
                assign(function);
            else
                assign(detail::retrospective_cast(std::forward<F>(function)));
        }

        template<typename Ret, typename... Args>
        void operator=(std::function<Ret (Args...)>&& function) noexcept
        {
            assign(std::forward<decltype(function)>(function));
        }

        template<typename Ret = void, typename... Args>
        Ret operator ()(Args&&... args) const
        {
            if (_handle == nullptr)
                throw std::bad_function_call();

            constexpr auto sigHash = detail::hashFunctionSignature<Ret, std::decay_t<Args>...>();

            if (sigHash != _hashedSignature)
                throw BadDelegateArguments(detail::typeName<Ret (*)(Args...)>());

            // Add rvalues to map correctly to the functor expected arguments
            auto f = reinterpret_cast<Ret (*)(void*, std::add_rvalue_reference_t<Args>...)>(_functor);
            return f(_handle, std::forward<Args>(args)...);
        }

    public:
        template<typename Ret, typename... Args>
        Ret call(Args&&... args) const
        {
            return this->operator()<Ret>(std::forward<Args>(args)...);
        }

    public:
        auto type()   const { return _hashedSignature; }
        auto target() const { return _handle;          }

    private:
        template<typename Ret, typename... Args>
        void assign(std::function<Ret (Args...)>&& function) noexcept
        {
            if (auto target = function.target<Ret (*)(Args...)>(); target)
                assign(*target);
        }

        template<typename Ret, typename... Args>
        void assign(Ret (*function)(Args...)) noexcept
        {
            if (function)
            {
                constexpr auto sigHash = detail::hashFunctionSignature<Ret, std::decay_t<Args>...>();

                if (_hashedSignature == 0)
                    _hashedSignature = sigHash;
                else if (sigHash != _hashedSignature)
                    return;

                // The functor is mandatory to keep the real signature of the handle.

                // In this particular case, f should take (void*, auto&&... args) as parameters.
                // However, to be able to decay the lambda as a function pointer, Args... must 
                // be deduced at compile time.
                // Thus, we add rvalueness to avoid memory errors since args can change their 
                // type (e.g. T& to T).

                constexpr auto f = +[](void *handle, std::add_rvalue_reference_t<Args>... args) -> Ret {
                    return reinterpret_cast<Ret(*)(Args...)>(handle)(std::forward<Args>(args)...);
                };

                _functor = reinterpret_cast<void*>(f);
                _handle  = reinterpret_cast<void*>(function);
            }
        }

    private:
        void*         _functor         = nullptr;
        void*         _handle          = nullptr;
        std::uint32_t _hashedSignature = 0;
    };
    
    #pragma pack(pop)

    inline bool operator==(const Delegate& lhs, const Delegate& rhs)
    {
        return lhs.type() == rhs.type();
    }

    inline bool operator!=(const Delegate& lhs, const Delegate& rhs)
    {
        return lhs.type() != rhs.type();
    }
} // end of axl namespace

#undef _AXL_FUNCTION_SIGNATURE
#undef _AXL_COMPILER_SUPPORTED
#undef _AXL_STANDARD_SUPPORTED 

#if defined(__clang__)
    #pragma clang diagnostic pop
#endif