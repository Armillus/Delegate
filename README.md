# Overview

**As easy as C#, as fast as C++.**

Developed at the beginning as a part of a much larger project to help me to achieve good performance in critical scenarios, 
Delegate has grown and aims to provide an easy to use way to replace your `std::function` efficiently. 

While being as safe as a standard `std::function`, Delegate are smaller, faster, safe and they **don't use any heap allocation**, 
whatever the size of the function you store inside.

## Why Delegate?

I wasn't really happy with `std::function` in my projects, for different reasons.

While `std::function` is an incredibly useful tool for any modern C++ developper, they lack of versatility in some configurations.
For example, due to their templated type, you can't store a map of functions with different signatures, which is restrictive sometimes.

Moreover, from a performance point of view, when the function they store is too large, they unavoidably use a heap allocation, which is slow.
Inside, `std::function` make use of tuples, which slows down the process as well.

I know that different replacements have been proposed so far to overcome the performance inconvenience, like `std::inplace_function`,
but to the best of my knowledge, it hasn't been accepted and there is no other implementation that suits me. Thus, this is why I came
with my own replacement for `std::function`

## How it works?

Under the hood, Delegate come from a simple idea.
When you execute a function, the signature of the latter is fixed. Usually, this signature is checked at compile time with templates.

The problem that I have with this implementation is that you can't store a map of functions of different signatures in C++ (at least not in a standard and reliable way).
So, how can we store the fixed type of a function signature without using templates? By hashing!

The whole trick of Delegate is to store the function signature as a compile time hash. Thus, when someone calls the function, the parameters and the return type
provided by the caller are hashed (always at compile time) and compared to the internal hash of the Delegate. If signatures don't match, an exception is thrown.

This mechanism provides a safe way to store the type of the function. Moreover, it does not perform any heap allocation, because the original target function
is encapsulated in a functor lambda, decayed as a simple function pointer.

By storing only C-style function pointers (we can allow it because of the safe hash check before any call to the function) and a hash, 
Delegate will always weight 16 bytes in memory (a std::function is about 64 bytes, so 4 times more). No more, no less.

# Documentation

Here are some examples of usage of Delegate in different scenarios. However, note that some limitations exist, refer to the appropriate section for more information.

```cpp
// C++ includes
#include <iostream>

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
    
    // Execute some actions!
    float x = 5.f;
    std::cout << callbacks["mul"].call<float>(x, 5.f) << std::endl;
    
    Position a { 5.f, 3.f };
    Position b { 5.f, 12.f };

    std::cout << std::boolalpha << callbacks["are_pos_x_equal"].call<bool>(a, b) << std::endl;

    // A delegate instantiated on its own
    axl::Delegate d { +[](int a, int& b) { return a + b; } };
    
    int c = 5;
    std::cout << d.call<int>(5, c) << std::endl;
    
    // Assign a new function with the same signature to 'd'
    d = [](int a, int& b) { return a - b; };

    std::cout << d.call<int>(5, c) << std::endl;
    return 0;
}
```

# Benchmarks

I did not run enough tests for now, so feel free to contribute to this section.
Anyway, you can find below the few tests results which are available.

Note that in Debug, Delegate is usually much slower than std::function. The performance benefits come with the Release mode, because the compiler
can properly optmize the code in this configuration.

# Limitations

For now, only C++ 17 and C++ 20 are supported by Delegate.
Concerning the limitations, you should first ensure that your compiler is recent enough to support Delegate.

The most embarassing issue is that Delegate does not support capturing lambdas. It comes from the fact that prior to C++20,
capturing lambdas can't be decayed as function pointers.

Thus, you can't expect to do that in your code:
```cpp
int a = 5;
axl::Delegate d { +[base](int& b) { return base + b; } };  // KO: Won't compile
```

# Contribute
