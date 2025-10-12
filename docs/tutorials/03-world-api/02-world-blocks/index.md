# 3.2. Blocks

This sections explains how to inspect/add/modify/remove [blocks](../../../lexicon.md#block) in a `SyncWorld`

!!! note
    All world modifications are done through a `Transaction`. A transaction can do multiple insertions/modifications/deletions at once. For performance reasons, always try to group your modifications in as few transactions as possible.

## Prerequisites

- [Selecting a Gustave configuration](../../01-getting-started/index.md)
- [Creating an empty SyncWorld](../01-creating-world/index.md): we'll reuse the [`newWorld()`](../01-creating-world/index.md#configuring-a-world) function.

```c++
--8<-- "docs/tutorials/03-world-api/02-world-blocks/main.cpp:create-world"
```

## List all blocks

`world.blocks()` is a [forward range](https://en.cppreference.com/w/cpp/ranges/forward_range). You can iterate over it using a [range-based for loop](https://en.cppreference.com/w/cpp/language/range-for):

```c++
--8<-- "docs/tutorials/03-world-api/02-world-blocks/main.cpp:printBlocks"
```

`auto const& block` is a C++ reference to a `BlockReference` object. Through this object, we can get any public property of the block:

- its mass: `block.mass()`
- its maximum stress `block.maxPressureStress()`
- if it's a foundation: `block.isFoundation()`
- the structures it belongs to, its contacts, its current stress ratio...

!!! note
    The iteration order is **implementation defined**. It will probably be different from the order of insertion, or the order of the block indices (like an [unordered_map](https://en.cppreference.com/w/cpp/container/unordered_map)).

## Insert blocks

First, let's define a few constants for all the blocks in this tutorial:

```c++
--8<-- "docs/tutorials/03-world-api/02-world-blocks/main.cpp:block-constants"
```

Then we can create a simple tower in the `SyncWorld` with a `Transaction` like this:

```c++
--8<-- "docs/tutorials/03-world-api/02-world-blocks/main.cpp:insert-blocks"
```

The key method is `World::Transaction::addBlock(BlockConstructionInfo const&)`. The `BlockConstructionInfo` has a constructor taking 4 arguments:

- `blockIndex`: the position of the block in the world.
- `maxBlockStress`: the maximum constraints this block can withstand before failing.
- `mass`: the mass of this block.
- `isFoundation`: whether this block is a [foundation](../../../lexicon.md#block).

Possible output of [`printBlocks()`](#list-all-blocks):

```
--8<-- "generated-snippets/tuto-03-02.txt:insert-blocks"
```

## Delete blocks

Again, any `world` modification goes through a `Transaction`. To remove blocks, just use `Transaction::removeBlock(BlockIndex)`:

```c++
--8<-- "docs/tutorials/03-world-api/02-world-blocks/main.cpp:delete-blocks"
```

This transaction will remove the 2 blocks at the top of the tower.

Possible output of [`printBlocks()`](#list-all-blocks):

```
--8<-- "generated-snippets/tuto-03-02.txt:delete-blocks"
```

## Modify blocks

To modify a block, remove & add it in the same transaction:

```c++
--8<-- "docs/tutorials/03-world-api/02-world-blocks/main.cpp:modify-blocks"
```

This transaction replaced the block at the top of the tower with a block of mass `heavyMass`.

Possible output of [`printBlocks()`](#list-all-blocks):

```
--8<-- "generated-snippets/tuto-03-02.txt:modify-blocks"
```

## Inspect a block

There are two ways to get a `BlockReference` by index:

- `world.blocks().at(BlockIndex)`: returns a `BlockReference` only if the block exists, throws otherwise.
- `world.blocks().find(BlockIndex)`: always returns a `BlockReference` that must be tested for validity (`block.isValid()`) before use.

The following example shows how to safely inspect a block using `find()`:

```c++
--8<-- "docs/tutorials/03-world-api/02-world-blocks/main.cpp:inspect-blocks"
```

Expected output:

```
--8<-- "generated-snippets/tuto-03-02.txt:inspect-blocks"
```
