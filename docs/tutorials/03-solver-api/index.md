# 3. Solver API

This section will describe Gustave's low-level API: [Solvers](../../lexicon.md#solver).

!!! note
    In this tutorial, all quantities are interpreted in the metric system. This is not mandatory when using the `unitless` distribution. You can interpret them in another natural system of unit: if you choose to interpret 1 unit of length as an imperial `foot`, 1 unit of mass as an imperial `pound`, and 1 unit of time as a `second`, then 1 unit of force must be interpreted as a `pound.foot/second²`.

## Prerequisites

- [Selecting a Gustave configuration](../01-getting-started/index.md)

We'll choose the `std::unitless` distribution with `double` precision:

```c++
// Choosing the Std Unitless distribution, with double precision
#include <gustave/distribs/std/unitless/Gustave.hpp>

using G = gustave::distribs::std::unitless::Gustave<double>;
```

We'll also define some convenient type aliases:

```c++
using Structure = G::Solvers::Structure;
using Solver = G::Solvers::F1Solver;
```

In this tutorial we'll aim at creating the following structure, and computing its force distribution:

![](world.svg)

## Create a new solver structure

To create an empty `Structure`, just use its default constructor. We'll store ours in a [std::shared_ptr](https://en.cppreference.com/w/cpp/memory/shared_ptr), as it will be required later by the [solver](../../lexicon.md#solver).

```c++
auto structure = std::make_shared<Structure>();
std::cout << "Structure of " << structure->nodes().size() << " blocks\n";
std::cout << "Structure of " << structure->links().size() << " links\n";
```

Expected output:

```
Structure of 0 blocks
Structure of 0 links
```

## Add blocks to a structure

Use `Structure::addNode(...)`. This takes a `Structure::Node` object, whose constructor has 2 arguments:

- **mass:** the mass of the block (in kilogram)
- **isFoundation:** a boolean to indicate if this block is a [foundation](../../lexicon.md#block)

It returns a `NodeIndex`, a unique identifier of this block in the structure. These indices are generated sequentially (0,1,2,...).

```c++
auto const blockMass = 3'000.0; // kilogram
//          xy
auto const n00 = structure->addNode(Structure::Node{ blockMass, true });
auto const n01 = structure->addNode(Structure::Node{ blockMass, false });
auto const n02 = structure->addNode(Structure::Node{ blockMass, false });
auto const n12 = structure->addNode(Structure::Node{ blockMass, false });
auto const n22 = structure->addNode(Structure::Node{ blockMass, false });
auto const n21 = structure->addNode(Structure::Node{ blockMass, false });
auto const n20 = structure->addNode(Structure::Node{ blockMass, true });
std::cout << "Structure of " << structure->nodes().size() << " blocks\n";
std::cout << "Structure of " << structure->links().size() << " links\n";
```

Expected output:

```
Structure of 7 blocks
Structure of 0 links
```

## Add links to a structure

Use `Structure::addLink(...)`. This takes a `Structure::Link` object, whose constructor has 4 arguments:

- **localNodeId:** the index of the first node
- **otherNodeId:** the index of the second node
- **normal:** the normal vector on the surface of **localNode**
- **conductivity:** a compression/shear/tensile quantity in `Newton/metre`. This is used by the [solver](../../lexicon.md#solver) to compute a [force distribution](../../lexicon.md#force-distribution): the higher the 3 components are, the more force will be transmitted through this link. It should be proportional to the maximum nominal force/pressure of the contact surface, and inversely proportional to the distance between the 2 nodes.

It sequentially returns a `LinkIndex`, which uniquely identifies this link in the structure.

```c++
// { compression, shear, tensile } in Newton/metre
auto const wallConductivity = G::Model::ConductivityStress{ 1'000'000.0, 500'000.0, 200'000.0 };
auto const roofConductivity = G::Model::ConductivityStress{ 100'000.0, 500'000.0, 100'000.0 };

auto const plusY = G::NormalizedVector3{ 0.0, 1.0, 0.0 };
auto const plusX = G::NormalizedVector3{ 1.0, 0.0, 0.0 };

// left wall
auto const l00_01 = structure->addLink(Structure::Link{ n00, n01, plusY, wallConductivity });
structure->addLink(Structure::Link{ n01, n02, plusY, wallConductivity });
// right wall
auto const l20_21 = structure->addLink(Structure::Link{ n20, n21, plusY, wallConductivity });
structure->addLink(Structure::Link{ n21, n22, plusY, wallConductivity });
// roof
structure->addLink(Structure::Link{ n02, n12, plusX, roofConductivity });
structure->addLink(Structure::Link{ n12, n22, plusX, roofConductivity });

std::cout << "Structure of " << structure->nodes().size() << " blocks\n";
std::cout << "Structure of " << structure->links().size() << " links\n";
```

Here we stored 2 link indices for later use: `l00_01` (between foundation `n00` and block `n01`) and `l20_21` (between foundation `n20` and block `n21`).

Expected output:

```
Structure of 7 blocks
Structure of 6 links
```

**Note:** It is possible to mix calls to `Structure::addLink()` and `Structure::addNode()`. The only requirement is that when `addLink()` is called, the nodes in the new link are already declared.

## Configure a solver

For now the only [solver](../../lexicon.md#solver) available is the `F1Solver`, which generate force vectors that are always colinear with the gravity acceleration vector `g`.

The two mandatory parameters for the solver are:

- **g:** the gravity acceleration vector in `metre/second²`
- **solverPrecision:** the precision of the solver. The closer to 0 it is, the more accurate the solutions will be according to Newton's 1st law of motion (the net force of non-foundation blocks will be closer to 0).

```c++
auto const g = G::vector3(0.f, -10.f, 0.f); // gravity acceleration (metre/second²).
auto const solverPrecision = 0.01; // precision of the force balancer (here 1%).
auto const solver = Solver{ Solver::Config{ g, solverPrecision } };

std::cout << "Solver gravity vector = " << solver.config().g() << '\n';
std::cout << "Solver target max error = " << solver.config().targetMaxError() << '\n';
```

Expected output:

```
Solver gravity vector = { "x": 0, "y": -10, "z": 0 }
Solver target max error = 0.01
```

## Solve a structure

Simply use `F1Solver::run(structure)`. This returns a `SolverResult` object with a `isSolved()` method, indicating if a solution within the target `solverPrecision` was reached.

```c++
    auto const solverResult = solver.run(structure);
    std::cout << "solution.isSolved() = " << solverResult.isSolved() << '\n';
```

Expected output:

```
solution.isSolved() = 1
```

!!! warning
    Do not modify `structure` after passing it to the solver.

## Inspect a solution's force vectors

If a `SolverResult` object is solved, the `solution()` method returns a `Solution` object. This object provides convenient methods to inspect nodes, contacts, and forces.

```c++
    auto const& solution = solverResult.solution();
    std::cout << "Force vector on block 00 by 01 = " << solution.contacts().at({l00_01, true}).forceVector() << '\n';
    std::cout << "Force vector on block 21 by 22 = " << solution.contacts().at({l20_21, false}).forceVector() << '\n';
```

A few comments on this block of code:

- `solution.contacts()` returns a range to access any link/contact through a `ContactReference`
- `.at({l00_01, true})` gets us the contact from its `ContactIndex`. This contact index is made from the link index `l00_01`, and a `true` boolean indicating that we want the contact on the surface of `localNodeId`, so `n00` in this case
- `.forceVector()` gets the force vector acting on the surface of `n00` from `n01`, transmitted through link `l00_01`

Possible output:

```
Force vector on block 00 by 01 = { "x": 0, "y": -75000, "z": 0 }
Force vector on block 21 by 22 = { "x": -0, "y": 75000, "z": -0 }
```

Comments about this output:

- **line 1**: This is the force of contact index `{l00_01, true}`, so the force on the foundation `n00` from `n01`. Since the structure is symmetrical, we expect this force to be half the total weight of the structure. Total weight of the structure: `5 * blockMass * g = {0, -150'000, 0} Newton`
- **line 2**: This is the force of contact index `{l20_21, false}`. The `false` inverses the `localNodeId` and `otherNodeId` of the contact: so this is the force on `n21` by the foundation `n20`. The foundation prevents `n21` from falling by pushing in the opposite direction of the gravity vector `g`. So the Y coordinate is positive.
