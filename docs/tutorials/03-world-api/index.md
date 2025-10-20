# 3. World API

This section will describe Gustave's high-level API: [Worlds](../../lexicon.md#world).

Its usage is simple: run a **transaction** to add/remove/replace [blocks](../../lexicon.md#block) in a world. It will automatically group adjacent blocks into independent [structures](../../lexicon.md#structure), and compute [force distributions](../../lexicon.md#force-distribution). It is then possible to inspect the **stress ratio** of each [link](../../lexicon.md#links-and-contacts), to test if each structure is stable (and if not, determine which [link](../../lexicon.md#links-and-contacts) fails).

For now, this API only support grids of cuboids (like a Minecraft world).

!!! note
    The examples here will only use 2D constructions for easier visualisation, but 3D is supported.

!!! note
    In this tutorial, all quantities are interpreted in the metric system. This is not mandatory when using the `unitless` distribution. Quantities can be interpreted in another natural system of unit: if 1 unit of length is an imperial `foot`, 1 unit of mass is an imperial `pound`, and 1 unit of time is a `second`, then 1 unit of force must be interpreted as a `pound.foot/second²`.
