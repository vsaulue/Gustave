# 2.2. Blocks

This sections explains how to inspect/add/modify/remove [blocks](../../../lexicon.md#block) in a `SyncWorld`

!!! note
    All world modifications are done through a `Transaction`. A transaction can do multiple insertions/modifications/deletions at once. For performance reasons, always try to group your modifications in as few transactions as possible.

## Prerequisites

- [Selecting a Gustave configuration](../../01-getting-started/index.md)
- [Creating an empty SyncWorld](../01-creating-world/index.md): we'll reuse the [`newWorld()`](../01-creating-world/index.md#configuring-a-world) function.

```c++
auto world = newWorld();
```

## List all blocks

`world.blocks()` is a [forward range](https://en.cppreference.com/w/cpp/ranges/forward_range). You can iterate over it using a [range-based for loop](https://en.cppreference.com/w/cpp/language/range-for):

```c++
auto printBlocks = [&world]() -> void {
    auto const& blocks = world.blocks();
    std::cout << "List of blocks (size = " << blocks.size() << "):\n";
    for (auto const& block : blocks) {
        std::cout << "- " << block.index() << ": mass = " << block.mass() << ", isFoundation = " << block.isFoundation() << '\n';
    }
};
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
auto const maxBlockStress = G::Model::PressureStress{
    100'000.0, // compression (Pascal)
    50'000.0,  // shear (Pascal)
    20'000.0,  // tensile (Pascal)
};
auto const heavyMass = 10'000.0;  // kilogram
auto const lightMass = 1'000.0;   // kilogram
```

Then we can create a simple tower in the `SyncWorld` with a `Transaction` like this:

```c++
{
    auto tr = World::Transaction{};
    tr.addBlock({ { 0,0,0 }, maxBlockStress, heavyMass, true }); // foundation block at coordinates {0,0,0}.
    for (int i = 1; i <= 6; ++i) {
        tr.addBlock({ { 0,i,0 }, maxBlockStress, lightMass, false }); // non-foundation block at coordinates {0,i,0}.
    }
    world.modify(tr);
}
```

The key method is `World::Transaction::addBlock(BlockConstructionInfo const&)`. The `BlockConstructionInfo` has a constructor taking 4 arguments:

- `blockIndex`: the position of the block in the world.
- `maxBlockStress`: the maximum constraints this block can withstand before failing.
- `mass`: the mass of this block.
- `isFoundation`: whether this block is a [foundation](../../../lexicon.md#block).

So the previous chunk of code:

- creates a transaction `tr`
- adds a single foundation block at `{0,0,0}` of mass `heavyMass`
- adds 6 non-foundation blocks of mass `lightMass` on top of this foundation
- runs the transaction using `world.modify(tr)`

Possible output of [`printBlocks()`](#list-all-blocks):

```
List of blocks (size = 7):
- { "x": 0, "y": 0, "z": 0}: mass = 10000, isFoundation = 1
- { "x": 0, "y": 1, "z": 0}: mass = 1000, isFoundation = 0
- { "x": 0, "y": 3, "z": 0}: mass = 1000, isFoundation = 0
- { "x": 0, "y": 2, "z": 0}: mass = 1000, isFoundation = 0
- { "x": 0, "y": 4, "z": 0}: mass = 1000, isFoundation = 0
- { "x": 0, "y": 5, "z": 0}: mass = 1000, isFoundation = 0
- { "x": 0, "y": 6, "z": 0}: mass = 1000, isFoundation = 0
```

## Delete blocks

Again, any `world` modification goes through a `Transaction`. To remove blocks, just use `Transaction::removeBlock(BlockIndex)`:

```c++
{
    auto tr = World::Transaction{};
    tr.removeBlock({ 0,6,0 });
    tr.removeBlock({ 0,5,0 });
    world.modify(tr);
}
```

This transaction will remove the 2 blocks at the top of the tower.

Possible output of [`printBlocks()`](#list-all-blocks):

```
List of blocks (size = 5):
- { "x": 0, "y": 0, "z": 0}: mass = 10000, isFoundation = 1
- { "x": 0, "y": 1, "z": 0}: mass = 1000, isFoundation = 0
- { "x": 0, "y": 3, "z": 0}: mass = 1000, isFoundation = 0
- { "x": 0, "y": 2, "z": 0}: mass = 1000, isFoundation = 0
- { "x": 0, "y": 4, "z": 0}: mass = 1000, isFoundation = 0
```

## Modify blocks

To modify a block, remove & add it in the same transaction:

```c++
auto tr = World::Transaction{};
tr.removeBlock({ 0,4,0 });
tr.addBlock({ {0,4,0}, maxBlockStress, heavyMass, false });
world.modify(tr);
```

This transaction replaced the block at the top of the tower with a block of mass `heavyMass`.

Possible output of [`printBlocks()`](#list-all-blocks):

```
List of blocks (size = 5):
- { "x": 0, "y": 0, "z": 0}: mass = 10000, isFoundation = 1
- { "x": 0, "y": 1, "z": 0}: mass = 1000, isFoundation = 0
- { "x": 0, "y": 3, "z": 0}: mass = 1000, isFoundation = 0
- { "x": 0, "y": 4, "z": 0}: mass = 10000, isFoundation = 0
- { "x": 0, "y": 2, "z": 0}: mass = 1000, isFoundation = 0
```

## Inspect a block

There are two ways to get a `BlockReference` by index:

- `world.blocks().at(BlockIndex)`: returns a `BlockReference` only if the block exists, throws otherwise.
- `world.blocks().find(BlockIndex)`: always returns a `BlockReference` that must be tested for validity (`block.isValid()`) before use.

The following example shows how to safely inspect a block using `find()`:

```c++
auto inspectBlock = [&world](World::BlockIndex const& blockId) -> void {
    std::cout << "Block at " << blockId << ": ";
    auto const blockRef = world.blocks().find(blockId);
    if (blockRef.isValid()) {
        std::cout << "mass = " << blockRef.mass() << ", isFoundation = " << blockRef.isFoundation();
    } else {
        std::cout << "invalid";
    }
    std::cout << '\n';
};
inspectBlock({ 0,0,0 });
inspectBlock({ 0,1,0 });
inspectBlock({ 9,9,9 });
```

Expected output:

```
Block at { "x": 0, "y": 0, "z": 0}: mass =  10000, isFoundation = 1
Block at { "x": 0, "y": 1, "z": 0}: mass =  1000, isFoundation = 0
Block at { "x": 9, "y": 9, "z": 9}: invalid
```
