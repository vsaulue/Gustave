# Tutorial: Creating an empty world

This section explains how to create a new `SyncWorld` object.

## Choosing a distribution

First, let's choose the `std::unitless` distribution with `double` precision, as explained in [getting started](../../01-getting-started/README.md):

```c++
// Choosing the Std Unitless distribution, with double precision
#include <gustave/distribs/std/unitless/Gustave.hpp>

using G = gustave::distribs::std::unitless::Gustave<double>;
```

## Configuring a world

A `SyncWorld` can be created with this helper function:

```c++
using World = G::Worlds::SyncWorld;

[[nodiscard]]
static World newWorld() {
    auto g = G::vector3(0.f, -10.f, 0.f); // gravity acceleration (metre/second²).
    auto solverPrecision = 0.01; // precision of the force balancer (here 1%).
    auto blockSize = G::vector3(1.f, 1.f, 1.f); // block dimension (cube with 1m edge).

    auto solverConfig = World::Solver::Config{ g, solverPrecision };
    return World{ blockSize, World::Solver{ solverConfig } };
}
```

A `SyncWorld` requires some configuration. Here are the key parameters:

- `g` is the gravity acceleration vector. Here it's `10 m/s²` in the -Y direction.
- `solverPrecision`: the precision of the [solver](../../../docs/lexicon.md#Solver). A Lower value gives a more accurate solutions (respecting Newton's 1st law of mostion), at a performance cost.
- `blockSize`: the dimensions of the blocks. Here each block is a `1 m` wide cube.

Currently the only [solver](../../../docs/lexicon.md#Solver) available is `F1Solver`. It generates [force distributions](../../../docs/lexicon.md#force-distribution) in which all forces transferred between blocks are collinear with the gravity vector `g`.

## Usage

Running this test code:

```c++
int main() {
    auto world = newWorld();

    std::cout << "Tutorial: creating a new SyncWorld.\n\n";

    std::cout << "- number of blocks = " << world.blocks().size() << '\n';
    std::cout << "- number of structures = " << world.structures().size() << '\n';
}
```

Should give the following output:

```
Tutorial: creating a new SyncWorld.

- number of blocks = 0
- number of structures = 0
```

`world.blocks()` and `world.structures()` are [forward ranges](https://en.cppreference.com/w/cpp/ranges/forward_range) which can be used to inspect the content of our world. As expected for an empty world, they have a size of 0.
