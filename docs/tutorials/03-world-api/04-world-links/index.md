# 3.4. Links, forces and stresses

This section explains how to inspect [links and contacts](../../../lexicon.md#links-and-contacts) in a `SyncWorld`.

In a world, links and contacts have a force and a stress factor computed by the [solver](../../../lexicon.md#solver). They can be used to determine which [blocks](../../../lexicon.md#block) or [structures](../../../lexicon.md#structure) are stable (or failing).

As links and contacts are quite similar, they are both manipulated with the same type: `ContactReference`.

## Prerequisites

- [Selecting a Gustave configuration](../../01-getting-started/index.md)
- [Creating an empty World](../01-creating-world/index.md): we'll reuse the [`newWorld()`](../01-creating-world/index.md#configuring-a-world) function.

```c++
--8<-- "docs/tutorials/03-world-api/04-world-links/main.cpp:create-world"
auto world = newWorld();
```

- [Adding & inspecting world blocks](../02-world-blocks/index.md): we'll add a few blocks for this tutorial.

We'll prepopulate the world:

```c++
--8<-- "docs/tutorials/03-world-api/04-world-links/main.cpp:add-blocks"
```

Which gives a chair and a floating platform:

![](world.svg)

Finally, a convenient alias to the `Direction` type, which holds an axis-aligned direction (plusX, minusX, plusY, ...):

```
--8<-- "docs/tutorials/03-world-api/04-world-links/main.cpp:direction-alias"
```

## Inspect a specific contact

To get a specific `ContactReference`, use the method `world.contacts().at(...)`. The parameter is a `ContactIndex`, made of:

- the `BlockIndex` of the block owning the contact surface.
- a `Direction`, getting a single surface of this block.

```c++
--8<-- "docs/tutorials/03-world-api/04-world-links/main.cpp:inspect-contact"
```

Expected output:

```
Contact { "blockIndex": { "x": 0, "y": 4, "z": 0}, "direction": "plusY" }: other block is { "x": 0, "y": 5, "z": 0}
```

## Check a contact's status

Like structures, a `ContactReference` has 2 important methods to check its status.

- `isValid()` checks if the contact exists. If false, almost all other operations on this reference will throw.
- `isSolved()` checks if the [solver](../../../lexicon.md#solver) was able to find a [force distribution](../../../lexicon.md#force-distribution) for the structure owning this contact. If false, some operations will throw (reading force, pressure or stress). It implies `isValid()`.

```c++
--8<-- "docs/tutorials/03-world-api/04-world-links/main.cpp:contact-status"
```

Expected output:

```
Contact { "blockIndex": { "x": 0, "y": 4, "z": 0}, "direction": "plusY" }: solved
Contact { "blockIndex": { "x": 3, "y": 8, "z": 0}, "direction": "plusX" }: unsolved
Contact { "blockIndex": { "x": 9, "y": 9, "z": 0}, "direction": "minusX" }: invalid
```

## Contact's force

For a solved `ContactReference`, the `forceVector()` method is available. It returns the force exerced by `otherBlock()` on `localBlock()` through this link:

```c++
--8<-- "docs/tutorials/03-world-api/04-world-links/main.cpp:contact-force"
```

Possible output:

```
Force vector by block { "x": 0, "y": 5, "z": 0} on block { "x": 0, "y": 4, "z": 0} = { "x": 0, "y": -120000, "z": 0 }
Force vector by block { "x": 0, "y": 0, "z": 0} on block { "x": 0, "y": 1, "z": 0} = { "x": -0, "y": 244591, "z": -0 }
Force vector by block { "x": 4, "y": 0, "z": 0} on block { "x": 4, "y": 1, "z": 0} = { "x": -0, "y": 205409, "z": -0 }
```

A few comment on this output:

- **Line 1:** According to Newton's first law of motion, this contact must receive the full weight of the 4 blocks making the "back of the chair". The weight vector of 4 blocks is `4 * mass * g = {0, -120'000, 0} Newton`.
- **Line 2 & 3:** These 2 contacts are on the surface of the 2 foundations blocks supporting the whole chair. How Gustave balance the weight between the 2 feet is implementation defined, but the sum of these two forces must be the **opposite**  of the weight vector of the chair. Chair's weight vector: `15 * mass * g = {0, -450'000, 0} Newton`

!!! note
    The [solver](../../../lexicon.md#solver) doesn't always find an exact solution (regarding Newton's first law). Remember the `solverPrecision` value from [Tutorial: Configure a solver](../../02-solver-api/index.md#configure-a-solver) ? This is where it matters: here the solver is allowed a `0.01` (1%) error factor. So the Y-value in **line 1** could be between `-118'800` and `-121'200` Newton.

## Link's stress ratio

This is the ratio `actual_pressure / max_nominal_pressure`, expressed as compression/shear/tensile components. This is an important property to check the link's stability: if all components are below 1, the link is stable. Otherwise, the link is failing.

A solved `ContactReference` has a `stressRatio()` method:

```c++
--8<-- "docs/tutorials/03-world-api/04-world-links/main.cpp:link-stress-ratio"
```

Possible output:

```
Stress of link { "blockIndex": { "x": 0, "y": 4, "z": 0}, "direction": "plusY" } = { "compression": 1.2, "shear": 0, "tensile": 0 }
Stress of link { "blockIndex": { "x": 0, "y": 1, "z": 0}, "direction": "minusY" } = { "compression": 0.489183, "shear": 0, "tensile": 0 }
Stress of link { "blockIndex": { "x": 4, "y": 1, "z": 0}, "direction": "minusY" } = { "compression": 0.410817, "shear": 0, "tensile": 0 }
```

So here the 2 feet of the chair are stable, but the link at the base of chair's back is failing.

## Stress ratio of a structure

As a conclusion to the `SyncWorld` tutorial, here is how to safely test the stress ratio of a structure (and therefore its stability):

```c++
--8<-- "docs/tutorials/03-world-api/04-world-links/main.cpp:structure-stress-ratio"
```

Possible output:

```
Max stress ratio of structure of block { "x": 0, "y": 1, "z": 0} = { "compression": 1.2, "shear": 0.169471, "tensile": 0 }
Max stress ratio of structure of block { "x": 3, "y": 8, "z": 0} = unsolved structure
```

- **Line 1** tells us that the chair is failing due to compression constraints. It is also subjected to some shear constraints that don't cause any failure.
- **Line 2** tells us that the magically floating platform has no solution.

This function is easily adaptable to:

- Get the block with the highest stress ratio (the most likely to break).
- Get a list of blocks with any stress component above 1 (= the list of failing blocks).
