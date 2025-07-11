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

A **structure** is a set of [blocks](#block), and [links](#links-and-contacts) between these blocks. It is the input problem of a [solver](#solver).

Structures should be:

- **complete:** if a structure contains a non-foundation block, it should also contains all blocks linked to it.
- **independent:** two different structures should not share a non-foundation block. However they might share foundations.
- **minimal:** if a structure contains two non-foundation blocks, they are (possibly indirectly) linked together by a chain of non-foundation blocks.

**Complete, independent** structures are a requirement for Gustave's [force distributions](#force-distribution) to have any logical sense.

Using only **minimal** structures is strongly advised for performance:

- [solving](#solver) a big structure is slower than splitting it into independent structures and solving those separately. The resulting [force distributions](#force-distribution) are the same.
- if a single structure is updated (e.g. by adding/removing blocks), only this structure needs to be solved again. The solution of any independent structure remains valid.

## Force distribution

The goal of a **force distribution** is to assign a force vector to each [contact](#links-and-contacts) in a [structure](#structure). This represents the force transfer between each block of a structure.

In Gustave, they respect the following conventions:

- for the contact on the surface of `A` (from `B`), this vector represents the force exerted on `A` by `B`.
- Opposite contacts follow [Newton's 3rd law of motion](https://en.wikipedia.org/wiki/Newton%27s_laws_of_motion): they have opposite force vectors.

## Solver

A **solver** finds a [force distribution](#force-distribution) for a given [structure](#structure) that satisfies equilibrium conditions:

- Non-foundation blocks: the [net force](https://en.wikipedia.org/wiki/Net_force) must be zero (within a margin of error), following [Newton's 1st law of motion](https://en.wikipedia.org/wiki/Newton%27s_laws_of_motion).
- Foundation blocks: no restriction on the net force.

The goal of a solver is to transfer the weight of all non-foundation blocks in a [structure](#structure) to its foundation blocks.

There is currently a single solver in Gustave, the `F1Solver` (Force 1):

- It generates force vectors in one dimension (colinear to the gravitational acceleration vector), hence the name.
- No restriction on block size, position, or contact area geometry.
- By far the biggest performance bottleneck in Gustave, and will likely remain that way (even if there is still a lot of room for optimisation left).

## World

The **world** is the higher API level of Gustave. A user can simply place/remove [blocks](#block) into it, and the world will:

- generate links between adjacent [blocks](#block)
- regroup them into minimal, independent [structures](#structure)
- pass them to a [solver](#solver) to generate equilibrium [force distributions](#force-distribution), evaluate stresses, and checks if any block is subjected to stresses above their maximum [compression/tensile/shear pressure](https://en.wikipedia.org/wiki/Strength_of_materials#Stress_terms) values.

The World API is currently limited to cuboid grids:

- all blocks are cuboid of the same dimensions
- they must be aligned to the global axes
