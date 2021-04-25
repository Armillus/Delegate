# Overview

**As easy as C#, as fast as C++.**

Developed at the beginning as a part of a much larger project to help me to achieve good performance in critical scenarios, 
Delegate has grown and aims to provide an easy to use way to replace your `std::function` efficiently. 

While being as safe as a standard `std::function`, Delegates are smaller, faster and they **don't use any heap allocation**, 
whatever the size of the function you store inside. Furthermroe, it's a **header only library**, easy to integrate in your C++ 17 / C++ 20 projects.

![Benchmark example](https://github.com/Armillus/Delegate/blob/master/benchmarks/results/msvc/ExecutionTime.JPG "MSVC benchmark")

## Why Delegate?

I wasn't really happy with `std::function` in my projects, for different reasons.

While `std::function` is an incredibly useful tool for any modern C++ developper, **they lack of versatility** in some configurations.
For example, due to their templated type, you can't store a map of functions with different signatures, which is restrictive sometimes.

Moreover, from a performance point of view, when the function they store is too large, **they unavoidably use a heap allocation**, which is slow.
Inside, `std::function` make use of tuples, which slows down the process as well.

I know that different replacements have been proposed so far to overcome the performance inconvenience, like `std::inplace_function`,
but to the best of my knowledge, it hasn't been accepted and there is no other implementation that suits me. Thus, this is why I came
with my own replacement for `std::function`.

Note that by creating this project, I wanted something similar to [C# Delegates](https://docs.microsoft.com/en-us/dotnet/csharp/programming-guide/delegates/using-delegates),
but with all the power and speed of C++.

## How it works?

Under the hood, Delegate come from a simple idea.
When you execute a function, the signature of the latter is fixed. Usually, this signature is checked at compile time with templates.

The problem that I have with this implementation is that you can't store a map of functions of different signatures in C++ (at least not in a standard and reliable way).
So, how can we store the fixed type of a function signature without using templates? **By hashing!**

Thanks to [Nameof](https://github.com/Neargye/nameof), we know that we can achieve some reflection and get the the type of a function as a string at compile time.
From the string, you can then get the corresponding hash.

**The whole trick of Delegate is to store the function signature as a compile time hash**. Thus, when someone calls the function, the parameters and the return type
provided by the caller are hashed (always at compile time) and compared to the internal hash of the Delegate. If signatures don't match, an exception is thrown.

This mechanism provides a safe way to store the type of the function. Moreover, **it does not perform any heap allocation**, because the original target function
is encapsulated in a functor lambda, decayed as a simple function pointer.

By storing only C-style function pointers (we can allow it because of the safe hash check before any call to the function) and a hash, 
**Delegates will always weight 16 bytes in memory** (a std::function is about 64 bytes, so 4 times more). No more, no less.

# Documentation

Here are some examples of usage of Delegate in different scenarios. However, note that **some limitations exist**, refer to the appropriate section for more information.

## A single Delegate

```cpp
// C++ includes
#include <iostream>

// Delegate includes
#include <Delegate/Delegate.hpp>

int main()
{
    // A delegate instantiated on its own
    axl::Delegate d { +[](int a, int& b) { return a + b; } };
    int b = 5;
    
    // Res should be equal to 10
    int res = d.call<int>(5, b);
    
    // Assign a new function with the same signature to 'd'
    d = [](int a, int& b) { return a - b; };

    // Res should be equal to 0
    int res = d.call<int>(5, b);
    
    return 0;
}
```

## A map of Delegates

```cpp
// C++ includes
#include <iostream>
#include <unordered_map>

// Delegate includes
#include <Delegate/Delegate.hpp>

struct Position
{
    float x = 0.0;
    float y = 0.0;
};

int main()
{
    // A simple map of actions
    std::unordered_map<std::string, axl::Delegate> actions;

    // A multiplication action
    actions.emplace("mul", [](float& x, float y) { return x * y; });
    
    // An action to check if two positions have the same 'x' coordinate
    actions.emplace("are_pos_x_equal", [](const Position& lhs, const Position& rhs) { 
        return lhs.x == rhs.x;
    });
    
    // Execute some actions now
    float x = 5.f;
    
    // Should be equal to 25 
    float res = callbacks["mul"].call<float>(x, 5.f);
    
    Position a { 5.f,  3.f };
    Position b { 5.f, 12.f };

    // Should be true
    bool areEqual = callbacks["are_pos_x_equal"].call<bool>(a, b);

    return 0;
}
```

## Call a Delegate with the wrong parameters

```cpp
// C++ includes
#include <iostream>

// Delegate includes
#include <Delegate/Delegate.hpp>

int main()
{
    // A delegate instantiated on its own without any function
    axl::Delegate d { +[](int a, int& b) { return a + b; } };
    
    try
    {
        d.call<int>('a',  b);  // KO: The first argument is not an integer
        d.call<int>(5.0f, b);  // KO: The first argument is not an integer
    }
    catch (const BadDelegateArguments& e)  // Inherits from std::runtime_error
    {
        // Handle the exception
    }
    
    return 0;
}
```

## Call an empty Delegate

```cpp
// C++ includes
#include <iostream>

// Delegate includes
#include <Delegate/Delegate.hpp>

int main()
{
    // A delegate instantiated on its own without any function
    axl::Delegate d;
    
    try
    {
        int res = d.call<int>(5, b); // KO: The Delegate is empty
    }
    catch (const std::bad_function_call& e)
    {
        // Handle the exception
    }
    
    return 0;
}
```

# Benchmarks

**I did not run enough tests for now**, so feel free to contribute to this section. I know that benchmarking is a long and complicated process,
but hopefully it will give you an idea about Delegate's performance.

Anyway, you can find below the few tests results which are available. The code for these benchmarks can be found in the `benchmarks` folder.
**Benchmarks are made with [Google Benchmark](https://github.com/google/benchmark).**

Note that in Debug, Delegate may be slower than `std::function`. The performance benefits come with the Release mode, because the compiler
can properly optmize the code in this configuration.

## Execution time and Creation + Execution time (Delegate vs std::function)

### MSVC (19.28)

![Results](https://github.com/Armillus/Delegate/blob/master/benchmarks/results/msvc/Results.JPG "MSVC results")

### Clang (11.0)

![Results](https://github.com/Armillus/Delegate/blob/master/benchmarks/results/clang/Results.JPG "Clang results")

# Limitations

## Compatibility

**Delegate requires at least C++ 17.**
Concerning the compiler, you should first ensure that it is part of the following list to use Delegate.

### Supported compilers

* Clang: >= 5
* MSVC:  >= 15.3
* GCC:   >= 7

## Capturing lambdas

The most embarassing issue is that **Delegates does not support capturing lambdas**. It comes from the fact that
capturing lambdas can't be decayed as function pointers.

Thus, you can't expect to do that in your code:
```cpp
int base = 5;
axl::Delegate d { +[base](int& b) { return base + b; } };  // KO: Won't compile
```

## Pay attention to your references

This is more a warning than a limitation.

In Delegate, you can **pass rvalues to functions waiting for lvalue references** (if the parameter type is copyable). 
It comes from the fact that functions signatures are decayed before hashing.

It could trigger some bugs in your program, so be careful.

```cpp
axl::Delegate d { +[](int a, int& b) { return base + b; } };

d.call<int(5, 5)>;  // OK: Compile and works smoothly (produces 10 as a result)
```

# Integration

You can add the content of the `include` folder (which contains only one file) into your project to use Delegate.

The provided `CMakeLists.txt` is only an example of what you can do to compile Delegate as a libary.

# Contribute

There is no specific guide to follow to contribute to this project, but feel free to contribute in any way! I probably won't accept all changes, but
I will consider all of them with pleasure.

I just ask you to stick with the coding style used in this project, and to explain clearly the purpose of your modification.
Thank you in advance for your contributions!

# License

This repository is licensed under the [MIT License](https://en.wikipedia.org/wiki/MIT_License). You're free to use it in your personal / professional projects.
