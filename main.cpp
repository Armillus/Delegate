#include <iostream>
// #include <windows.h>

#include "include/Delegate/Delegate.hpp"

#include <array>
#include <string_view>

using L = decltype([]() { return 3; });

constexpr void test(L test)
{
    constexpr auto bob = test();
}


int main() try
{
    int a = 5, b = 3, c = 8;

    using FPTR = void(*)(L);

    FPTR fptr = test;

    fptr(L());
    //static_assert(std::is_invocable_r_v<int, int(*)(int), int>);
    //static_assert(std::is_invocable_r_v<int, int(*)(int), const int>);
    //static_assert(std::is_invocable_r_v<int, int(*)(int), volatile int>);
    //static_assert(std::is_invocable_r_v<int, int(*)(int), int&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(int), const int&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(int), volatile int&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(int), int&&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(int), const int&&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(int), volatile int&&>);

    //static_assert(std::is_invocable_r_v<int, int(*)(int&), int>);
    //static_assert(std::is_invocable_r_v<int, int(*)(int&), int&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(int&), volatile int&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(int&), const int&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(int&), int&&>);

    //static_assert(std::is_invocable_r_v<int, int(*)(int&&), int>);
    //static_assert(std::is_invocable_r_v<int, int(*)(int&&), const int>);
    //static_assert(std::is_invocable_r_v<int, int(*)(int&&), volatile int>);
    //static_assert(std::is_invocable_r_v<int, int(*)(int&&), int&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(int&&), const int&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(int&&), volatile int&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(int&&), int&&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(int&&), const int&&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(int&&), volatile int&&>);



    //static_assert(std::is_invocable_r_v<int, int(*)(const int), int>);
    //static_assert(std::is_invocable_r_v<int, int(*)(const int), const int>);
    //static_assert(std::is_invocable_r_v<int, int(*)(const int), volatile int>);
    //static_assert(std::is_invocable_r_v<int, int(*)(const int), int&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(const int), const int&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(const int), volatile int&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(const int), int&&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(const int), const int&&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(const int), volatile int&&>);

    //static_assert(std::is_invocable_r_v<int, int(*)(const int&), int>);
    //static_assert(std::is_invocable_r_v<int, int(*)(const int&), int&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(const int&), volatile int&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(const int&), const int&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(const int&), int&&>);

    //static_assert(std::is_invocable_r_v<int, int(*)(const int&&), int>);
    //static_assert(std::is_invocable_r_v<int, int(*)(const int&&), const int>);
    //static_assert(std::is_invocable_r_v<int, int(*)(const int&&), volatile int>);
    //static_assert(std::is_invocable_r_v<int, int(*)(const int&&), int&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(const int&&), const int&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(const int&&), volatile int&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(const int&&), int&&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(const int&&), const int&&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(const int&&), volatile int&&>);



    //static_assert(std::is_invocable_r_v<int, int(*)(volatile int), int>);
    //static_assert(std::is_invocable_r_v<int, int(*)(volatile int), const int>);
    //static_assert(std::is_invocable_r_v<int, int(*)(volatile int), volatile int>);
    //static_assert(std::is_invocable_r_v<int, int(*)(volatile int), int&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(volatile int), const int&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(volatile int), volatile int&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(volatile int), int&&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(volatile int), const int&&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(volatile int), volatile int&&>);

    //static_assert(std::is_invocable_r_v<int, int(*)(volatile int&), int>);
    //static_assert(std::is_invocable_r_v<int, int(*)(volatile int&), volatile int>);
    //static_assert(std::is_invocable_r_v<int, int(*)(volatile int&), int&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(volatile int&), volatile int&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(volatile int&), const int&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(volatile int&), int&&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(volatile int&), volatile int&&>);

    //static_assert(std::is_invocable_r_v<int, int(*)(volatile int&&), int>);
    //static_assert(std::is_invocable_r_v<int, int(*)(volatile int&&), const int>);
    //static_assert(std::is_invocable_r_v<int, int(*)(volatile int&&), volatile int>);
    //static_assert(std::is_invocable_r_v<int, int(*)(volatile int&&), int&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(volatile int&&), const int&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(volatile int&&), volatile int&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(volatile int&&), int&&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(volatile int&&), const int&&>);
    //static_assert(std::is_invocable_r_v<int, int(*)(volatile int&&), volatile int&&>);

    static_assert(std::is_invocable_v<void(*)(int&, bool, int), int&, const bool, int>);

    static_assert(axl::traits::is_function_pointer_v<decltype(&std::strlen)>);

    constexpr axl::Delegate del { axl::bind<&std::strlen>() };

    axl::Delegate d { [&]() { return a + b + c + 3; } };
    constexpr axl::Delegate e { [](int num) { return num * num; } };

    std::unordered_map<int, axl::Delegate<int()>> test;

    axl::Delegate<void(...)> t{ [](int& a, bool, const int b) { std::cout << a + b << std::endl; }};

    test.emplace(0, [&]() { return a + 3; });

    axl::Delegate<> s;

    std::cout << "test[0]() => " << test[0]() << std::endl;
    std::cout << "del(\"bobby\") => " << del("bobby") << std::endl;

    std::cout << "sizeof(d) => " << sizeof(d) << std::endl;

    std::cout << "d() => " << d() << std::endl;
    std::cout << "t() => " << std::endl;
    int z = 3;
    t(z, false, b);

    auto lambda = [](int) { std::cout << "hello" << std::endl; };

    using F = decltype(lambda);
    using G = decltype(axl::bind<&std::strlen>());

    // static_assert(axl::traits::is_callable_impl<G>::value);

    static_assert(std::is_convertible_v<F, axl::traits::function_pointer_t<F>>);

    // FIXME: captureless lambdas are cannot be bound for now
    static_assert(std::is_empty_v<F>&& std::is_default_constructible_v<F>);

    t.bind([]() { std::cout << "hello" << std::endl; });

    t();

 //   axl::Delegate<int(axl::AnyArg)> lol{ +[](int a, const char* b) { std::cout << "In LOL => " << b << std::endl; return a; } };

    // std::cin >> a;

	return 0;
}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
}