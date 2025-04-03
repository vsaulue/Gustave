# Tutorial 01: Getting started

If not done yet, please read the [install guide](../../docs/install.md) to download Gustave and link it to your project.

Gustave is a customisable library. Most of its classes takes a *library configuration* template parameter that enables using others:
* 3d math implementations (ex: 3d vectors)
* units/dimensional analysis libraries (ex: using imperial units, or just not using any dimensional analysis)
* floating-point precision (float or double)

The Gustave's core library with a configuration is called a **distribution**. You can eventually make your own, but to get started, you can use one of the 2 standard distribution.

## Distribution: StdStrictUnit vs StdUnitless

Both these distributions just require a C++20 standard library to work. The difference is the unit/dimensional-analysis used:
* `StdUnitless` doesn't do any dimensional analysis.
* `StdStrictUnit` enforces dimensional analysis: any invalid operation (ex: assigning a velocity to a force), will produce an atrocious compile-time errors that will rival the average Boost library. This is extremely valuable for Gustave's internal tests, but probably not worth the trouble for you.

**Conclusion:** for your sanity, use `StdUnitless`.

## Precision: float vs double

Currently Gustave either only use `float` or `double`. With 32-bit precision, Gustave has stability issues and just won't work. Use 64-bit float (which is likely `double`, unless you have an exotic compiler).

**Conclusion:** use `double` (64-bit floats).

## Getting started: basic maths

This example will just show how to do basic maths with Gustave's `StdUnitless`:
```c++
// One global header per distribution
#include <gustave/distribs/std/unitless/Gustave.hpp>

static void unitlessDemo() {
    // Choosing a distribution, with double precision
    using G = gustave::distribs::std::unitless::Gustave<double>;

    std::cout << "Gustave 'Unitless' Demo\n\n";
    auto g = G::vector3(0, -9.8, 0);
    auto mass = 1000.0;
    auto weight = mass * g;
    std::cout << "g = " << g << '\n';
    std::cout << "mass = " << mass << '\n';
    std::cout << "weight = mass * g = " << weight << '\n';

    // the following statement would not compile with StrictUnit, but will work with Unitless.
    std::cout << "wrong_operation = mass + g.y() = " << mass + g.y() << '\n';
}
```

Expected output:
```
Gustave 'Unitless' Demo

g = { "x": 0, "y": -9.8, "z": 0 }
mass = 1000
weight = mass * g = { "x": 0, "y": -9800, "z": 0 }
wrong_operation = mass + g.y() = 990.2
```

See the [full code example](main.cpp), which also contains the equivalent demonstration for `StdStrictUnit`.
