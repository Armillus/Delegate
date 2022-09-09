#include <iostream>
// #include <windows.h>

#include "include/Delegate/Delegate.hpp"

#include <array>
#include <string_view>

int main() try
{
    int a = 5, b = 3, c = 8;
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

    constexpr axl::Delegate del { axl::bind<&std::strlen>() };

    axl::Delegate d { [&]() { return a + b + c + 3; } };
    constexpr axl::Delegate e { [](int num) { return num * num; } };

    std::unordered_map<int, axl::Delegate<int()>> test;

    axl::Delegate<void(...)> t{ [](int& a, bool, const int b) { 
        std::cout << a + b << std::endl; 
    }};

    test.emplace(0, [&]() { return a + 3; });

    axl::Delegate<> s;

    std::cout << "test[0]() => " << test[0]() << std::endl;
    std::cout << "del(\"bobby\") => " << del("bobby") << std::endl;

    std::cout << "sizeof(d) => " << sizeof(d) << std::endl;

    std::cout << "d() => " << d() << std::endl;
    std::cout << "t() => " << std::endl;
    int z = 3;
    t(z, false, b);

    t.bind([]() { std::cout << "hello" << std::endl; });

    t();

    // std::cin >> a;

	return 0;
}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
}