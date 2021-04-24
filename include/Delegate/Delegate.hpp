#pragma once

// C++ includes
#include <functional>
#include <string_view>

namespace axl
{
    namespace detail
    {
        #define HASH_DEFAULT_PRIME_NUMBER 73

        template<std::size_t N>
        [[nodiscard]] constexpr std::uint32_t hash(std::uint32_t prime, const char (&s)[N], std::size_t len = N - 1) noexcept
        {
            // Simple Horner hash
            return (len <= 1) ? s[0] : (prime * hash(prime, s, len - 1) + s[len - 1]);
        }

        [[nodiscard]] constexpr std::uint32_t hash(std::uint32_t prime, const char* s, std::size_t len) noexcept
        {
            // Simple Horner hash
            return (len <= 1) ? s[0] : (prime * hash(prime, s, len - 1) + s[len - 1]);
        }

        [[nodiscard]] constexpr std::uint32_t hash(std::uint32_t prime, const std::string_view& s) noexcept
        {
            return hash(prime, s.data(), s.size() - 1);
        }

        [[nodiscard]] constexpr std::uint32_t hash(const std::string_view& s) noexcept
        {
            return hash(HASH_DEFAULT_PRIME_NUMBER, s);
        }

        #undef HASH_DEFAULT_PRIME_NUMBER

        template<typename Ret, typename... Args, typename F = Ret (*)(Args...)>
        constexpr auto hashFunctionSignature() noexcept -> std::uint32_t
        {
            // Declare 'hashedSignature' as constexpr before returning it forces 
            // the compiler to create it at compile time.
            
            constexpr auto hashedSignature = hash(typeName<F>());
            return hashedSignature;
        }

        constexpr auto prettifyName(std::string_view s) noexcept -> std::string_view
        {
            std::size_t start = 0;
            std::size_t end   = s.find_last_of('>');
            
            if (end == std::string_view::npos)
                return {};

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

            #if defined(__clang__) || defined(__GNUC__)
                constexpr auto name = prettifyName({ __PRETTY_FUNCTION__, sizeof(__PRETTY_FUNCTION__) });
            #elif defined(_MSC_VER)
                constexpr auto name = prettifyName({ __FUNCSIG__, sizeof(__FUNCSIG__) });
            #else
                #error Unsupported compiler, impossible to detect the function signature.
            #endif

            return name;
        }

        template<typename F>
        struct fun_type : public fun_type<decltype(&F::operator())> {};

        template<typename Ret, typename Class, typename... Args>
        struct fun_type<Ret (Class::*)(Args...) const>
        {
            using type    = std::function<Ret (Args...)>;
            using pointer = Ret (*)(Args...);
        };

        template<typename F>
        using fun_type_t = typename fun_type<F>::type;

        template<typename F>
        using fun_type_p = typename fun_type<F>::pointer;

        template<typename F>
        inline decltype(auto) retrospective_cast(F&& func)
        {
            // Capturing lambdas can't be converted into function pointers,
            // and even by putting them in a std::function, we still have
            // a custom pointer incompatible with how Delegate works.

            static_assert(std::is_convertible_v<F, fun_type_p<F>>,
                          "Delegate: capturing lambdas are not supported, "
                          "refer to the documentation for more information.");
            
            if constexpr (std::is_convertible_v<F, fun_type_p<F>>)
            {
                // TODO: more tests to check if the behavior is always correct
                return static_cast<fun_type_p<F>>(func);
            }
            else
            {
                // It won't work (because of the internal type of the lamdba),
                // no more than fun_type_t<F>(func).
                return std::function { func };  
            }
        }
    } // end of namespace detail

    class BadDelegateArguments final : public std::runtime_error
    {
    public:
        BadDelegateArguments(std::string_view invalidSignature) noexcept
            : std::runtime_error { std::string("Delegate called with the following bad signature: ")
                                   + invalidSignature.data() }
        {}
    };

    #pragma pack(push, 1)

    class Delegate
    {
    public:
        // Mandatory to be stored in a map
        Delegate() noexcept = default;

        template<typename F>
        Delegate(F&& function) noexcept
        {
            // Deduce lambdas types
            assign(detail::retrospective_cast(std::forward<F>(function)));
        }

        template<typename Ret, typename... Args>
        Delegate(std::function<Ret (Args...)> function) noexcept
            : _hashedSignature { detail::hashFunctionSignature<Ret, std::decay_t<Args>...>() }
        { 
            assign(std::move(function));
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

    public:
        template<typename Ret, typename... Args>
        void assign(std::function<Ret (Args...)> function) noexcept
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

                _functor = f;
                _handle = function;
            }
        }

    public:
        template<typename Ret, typename... Args>
        Ret call(Args&&... args) const
        {
            return this->operator()<Ret>(std::forward<Args>(args)...);
        }

        template<typename Ret = void, typename... Args>
        Ret operator ()(Args&&... args) const
        {
            constexpr auto sigHash = detail::hashFunctionSignature<Ret, std::decay_t<Args>...>();

            if (_handle == nullptr)
                throw std::bad_function_call();
            
            if (sigHash != _hashedSignature)
                throw BadDelegateArguments(detail::typeName<Ret (*)(Args...)>());

            // Add rvalues to map correctly to the functor expected arguments
            auto f = reinterpret_cast<Ret (*)(void*, std::add_rvalue_reference_t<Args>...)>(_functor);
            return f(_handle, std::forward<Args>(args)...);
        }

    public:
        auto type()   const { return _hashedSignature; }
        auto target() const { return _handle;          }

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