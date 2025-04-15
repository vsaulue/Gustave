# Tutorial 02: World API

This section will describe Gustave's high-level API: [Worlds](../../lexicon.md#World).

Its usage is simple: run a **transaction** to add/remove/replace [blocks](../../lexicon.md#block) in a world. It will automatically group adjacent blocks into independent [structures](../../lexicon.md#structure), and compute [force distributions](../../lexicon.md#force-distribution). It is then possible to inspect the **stress ratio** of each [link](../../lexicon.md#Links-and-contacts), to test if each structure is stable (and if not, determine which [link](../../lexicon.md#links-and-contacts) fails).

For now, this API only support grids of cuboids (like a Minecraft world). The examples here will only use 2D constructions for easier visualisation, but 3D is supported.

**Note:** We'll use the `SyncWorld` (synchronized world) implementation for this guide: the functions modifying the world don't return until its [structures](../../lexicon.md#structure) are solved.

**Note:** In this tutorial, all quantities are interpreted in the metric system. This is not mandatory when using the `unitless` distribution. You can interpret them in another natural system of unit: if you choose to interpret 1 unit of length as an imperial `foot`, 1 unit of mass as an imperial `pound`, and 1 unit of time as a `second`, then 1 unit of force must be interpreted as a `pound.foot/second²`.

## Index

1. [Creating a world](01-creating-world/README.md): how to create an empty `SyncWorld`.
1. [World blocks](02-world-blocks/README.md): how to add/remove/inspect blocks in a world.
1. [World structures](03-world-structures/README.md): how to inspect structures in a world.
1. [World links, forces and stresses](04-world-links/README.md): how to inspect links, forces, stresses, and find failing links/structures.
