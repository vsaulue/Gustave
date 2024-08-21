# Terminology

This page describes a few terms used in Gustave's documentation and API.

## Block

A **block** is the basic construction unit of anything. This could be a piece of floor, wall, pillar, ceiling...

- Each block has a **weight**.
- Each block can be designated as a **foundation**. A foundation is a block that won't move regardless of the force exerted on it by other block (e.g. they are solidly anchored to the ground or a cliff's side).

In the high-level [World](#world) API, the block has its own maximum **[compression/tensile/shear pressure](https://en.wikipedia.org/wiki/Strength_of_materials#Stress_terms)** values. These values have two roles:

- they are used to determine if the block (and its structure) is stable.
- they are used by the [solver](#solver) to generate a closer to optimal [force distribution](#force-distribution). For example, Gustave will try to avoid causing compression stress on a block with low compression strength.

## Links and contacts

A **link** describes how two blocks can transfer a force between each other. A **link** is symmetrical: the link from block `A` to `B` is also a link from block `B` to `A`.

The two blocks of a link must satisfy the following conditions:

- they are distincts (it is not possible to link a block with itself)
- at least one block must not be a **foundation**. In Gustave, two foundations never transfer forces directly between each other.

Each **link** between `A` and `B` describes two opposite **contacts**:

- a contact on the surface of `A` (from `B`).
- a contact on the surface of `B` (from `A`).

## Structure

A **structure** groups a set of [blocks](#block) with their [links](#links-and-contacts). Two structures are **independent** if they don't share a non-foundation block.

## Force distribution

The goal of a **force distribution** is to assign a force vector to each [contact](#links-and-contacts) in a [structure](#structure). This represents the force transfer between each block of a structure.

In Gustave, they respect the following conventions:

- for the contact on the surface of `A` (from `B`), this vector represents the force exerted on `A` by `B`.
- Opposite contacts follow [Newton's 3rd law of motion](https://en.wikipedia.org/wiki/Newton%27s_laws_of_motion): they have opposite force vectors.

## Solver

A **solver** finds a [force distribution](force-distribution) for a given [structure](#structure) that satisfies equilibrium conditions:

- Non-foundation blocks: the sum of forces exerted on it (its weight, and the forces from links) must be zero (within a margin of error), following [Newton's 1st law of motion](https://en.wikipedia.org/wiki/Newton%27s_laws_of_motion).
- Foundation blocks: no restriction on the sum of forces.

The goal of a solver is to transfer the weight of all non-foundation blocks in a [structure](#structure) to its foundation blocks.

There is currently a single solver in Gustave, called **Force1**:

- Generates force vectors in one dimension (colinear to the gravitational acceleration vector), hence the name.
- No restriction on block size, position, or contact area geometry.
- By far the biggest performance bottleneck in Gustave, and will likely remain that way (even if there is still a lot of room for optimisation left).

## World

The **world** is the higher API level of Gustave. A user can simply place/remove [blocks](#block) into it, and the world will:

- generate links between adjacent [blocks](#blocks)
- regroup them into minimal, independent [structures](#structure)
- pass them to a [solver](#solver) to generate equilibrium [force distributions](#force-distribution), evaluate stresses, and checks if any block is subjected to stresses above their maximum [compression/tensile/shear pressure](https://en.wikipedia.org/wiki/Strength_of_materials#Stress_terms) values.

The World API is currently limited to cuboid grids: mean that blocks must be cuboid, aligned on global axis, and
