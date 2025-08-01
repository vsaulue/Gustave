# 3.3. Structures

This sections explains how to inspect [structures](../../../lexicon.md#structure) in a `SyncWorld`.

Structures are automatically created/deleted by the `SyncWorld` when it is modified, so users can't directly edit structures. They can be inspected by getting a `StructureReference` object.

## Prerequisites

- [Selecting a Gustave configuration](../../01-getting-started/index.md)
- [Creating an empty World](../01-creating-world/index.md): we'll reuse the [`newWorld()`](../01-creating-world/index.md#configuring-a-world) function.

```c++
--8<-- "docs/tutorials/03-world-api/03-world-structures/main.cpp:create-world"
```

- [Adding & inspecting world blocks](../02-world-blocks/index.md): we'll add a few blocks for this tutorial:

```c++
--8<-- "docs/tutorials/03-world-api/03-world-structures/main.cpp:add-blocks"
```

Here's a visual representation of this world:

![](world.svg)

## List all structures and their blocks

`world.structures()` is a [forward range](https://en.cppreference.com/w/cpp/ranges/forward_range) of `StructureReference`. You can iterate over it using a [range-based for loop](https://en.cppreference.com/w/cpp/language/range-for). Then a `StructureReference` has a `.blocks()` method, which works like `world.blocks()`, and is a range of `BlockReference` objects.

So iterating over all structures and listing their blocks is a simple double loop:

```c++
--8<-- "docs/tutorials/03-world-api/03-world-structures/main.cpp:list-world-structures"
```

!!! note
    Both `world.structures()` and `structure.blocks()` are unordered containers, so all structures and blocks are given in no particular order.

Possible output:

```
List of structures (size = 3)
- structure of 2 blocks:
  - { "x": 8, "y": 0, "z": 0}
  - { "x": 7, "y": 0, "z": 0}
- structure of 5 blocks:
  - { "x": 4, "y": 0, "z": 0}
  - { "x": 3, "y": 0, "z": 0}
  - { "x": 2, "y": 0, "z": 0}
  - { "x": 0, "y": 0, "z": 0}
  - { "x": 1, "y": 0, "z": 0}
- structure of 3 blocks:
  - { "x": 0, "y": 2, "z": 0}
  - { "x": 0, "y": 0, "z": 0}
  - { "x": 0, "y": 1, "z": 0}
```

In the above output:

- The `2 blocks` structure is the blue one on the right.
- The `5 blocks` structure is the green one, and includes the 2 foundations
- The `3 blocks` structure is the red one on the left, and includes the bottom-left foundation.

## List all structures of a block

A `BlockReference` has a `.structures()` method returning a range of `StructureReference`, similar to `world.structures()`:

```c++
--8<-- "docs/tutorials/03-world-api/03-world-structures/main.cpp:list-block-structures"
```

Expected output:

```
Structures of block { "x": 0, "y": 0, "z": 0} (size = 2):
- structure of 5 blocks
- structure of 3 blocks
Structures of block { "x": 7, "y": 0, "z": 0} (size = 1):
- structure of 2 blocks
```

!!! note
    A non-foundation block always has exactly 1 structure. A foundation block can have 0 to 6 structures in a cuboid world.

## Structure status: valid and solved

A `StructureReference` has two important methods about its status:

- `isValid()`: checks if the structure still exists in the world. If `false`, all other operations on this reference will throw. This can happen if a `world.modify()` call deleted the structure.
- `isSolved()`: checks if the solver was able to find a solution. It `false`, it is possible get the list of blocks, [links and contacts](../../../lexicon.md#links-and-contacts) of this structure, but reading forces and stresses will throw. This can happen if a structure has no foundation (or the solver didn't converge).

Running this test code:

```c++
--8<-- "docs/tutorials/03-world-api/03-world-structures/main.cpp:structure-status"
```

Should give the following output:

```
Statut of structure of block { "x": 0, "y": 1, "z": 0}: solved
Statut of structure of block { "x": 7, "y": 0, "z": 0}: not solved
```
