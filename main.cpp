// Delegate.cpp : Defines the entry point for the application.
//

#include <iostream>
#include <benchmark/benchmark.h>

#include "Delegate/Delegate.hpp"

static void BM_DelegateExecutionTime(benchmark::State& state)
{
    axl::Delegate d { +[](int a, int b) { return a + b; } };

    for (auto _ : state)
    {
        d.call<int>(3, 6);
    }
}

static void BM_FunctionExecutionTime(benchmark::State& state)
{
    std::function<int (int, int)> f = [](int a, int b) { return a + b; };

    for (auto _ : state)
    {

        f(3, 6);
    }
}

BENCHMARK(BM_DelegateExecutionTime);
BENCHMARK(BM_FunctionExecutionTime);


struct Test
{
    void*         _functor         = nullptr;
    //void*         _handle          = nullptr;
    std::uint32_t _hashedSignature = 0;
};

//int main()
//{
//    axl::Delegate d { +[](int a, int& b) { return a + b; } };
//    std::function<int (int, int)> f = [](int a, int b) { return a + b; };
//
//    int ok = 3;
//    std::cout << "Delegate size: " << sizeof(d) << " | std::function size: " << sizeof(f) << std::endl;
//	return 0;
//}

BENCHMARK_MAIN();