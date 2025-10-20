# 1. Getting started

If not done yet, please read the [install guide](../../install.md) to download Gustave and link it to another project.

Gustave is a customisable library. Most of its classes takes a *library configuration* template parameter that enables using others:

- 3d math implementations (ex: 3d vectors)
- units/dimensional analysis libraries (ex: using imperial units, or just not using any dimensional analysis)
- floating-point precision (float or double)

The Gustave's core library with a configuration is called a **distribution**. It is possible to create new configurations downstream. But to get started quickly, Gustave provides 2 standard distribution.

## Distribution: StdStrictUnit vs StdUnitless

Both these distributions just require a C++20 standard library to work. The difference is the unit/dimensional-analysis used:

- `StdUnitless` doesn't do any dimensional analysis.
- `StdStrictUnit` enforces dimensional analysis: any invalid operation (ex: assigning a velocity to a force), will produce atrocious compile-time errors that will rival the average Boost library. This is extremely valuable for Gustave's internal tests, but probably too troublesome for other projects.

**Conclusion:** for simplicity, use `StdUnitless`.

## Precision: float vs double

Currently Gustave either only use `float` or `double`. With 32-bit precision, Gustave has stability issues and just won't work. Use 64-bit float (which is likely `double`, unless using an exotic compiler).

**Conclusion:** use `double` (64-bit floats).

## Getting started: basic maths

This example will just show how to do basic maths with Gustave's `StdUnitless`:

```c++
--8<-- "docs/tutorials/01-getting-started/main.cpp:include-unitless"

--8<-- "docs/tutorials/01-getting-started/main.cpp:unitlessDemo"
```

Expected output:

```
--8<-- "generated-snippets/tuto-01.txt:unitless-demo"
```

See the [full code example](main.cpp), which also contains the equivalent demonstration for `StdStrictUnit`.
