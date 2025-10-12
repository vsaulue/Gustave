# 3.1. New world

This section explains how to create a new `SyncWorld` object.

## Choosing a distribution

First, let's choose the `std::unitless` distribution with `double` precision, as explained in [getting started](../../01-getting-started/index.md):

```c++
--8<-- "docs/tutorials/03-world-api/01-creating-world/main.cpp:distrib-unitless"
```

We'll also define some type aliases:

```c++
--8<-- "docs/tutorials/03-world-api/01-creating-world/main.cpp:type-aliases"
```

## Configuring a world

A `SyncWorld` can be created with these helper functions:

```c++
--8<-- "docs/tutorials/03-world-api/01-creating-world/main.cpp:newWorld"
```

Explanation:

- `newSolver()`: configures the solver used by the world. See [Solver API: configure a solver](../../02-solver-api/index.md#configure-a-solver) for more details.
- `blockSize`: the dimensions of the blocks. Here each block is a `1 metre` wide cube.

## Usage

Running this test code:

```c++
--8<-- "docs/tutorials/03-world-api/01-creating-world/main.cpp:newWorld-usage"
```

Should give the following output:

```
--8<-- "generated-snippets/tuto-03-01.txt:new-world-usage"
```

`world.blocks()` and `world.structures()` are [forward ranges](https://en.cppreference.com/w/cpp/ranges/forward_range) which can be used to inspect the content of our world. As expected for an empty world, they have a size of 0.
