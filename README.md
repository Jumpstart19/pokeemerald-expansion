# New Features

1) All boulders now slide when pushed on ice (see FAQ for details on how to only implement this feature).
<img width="240" height="160" alt="Boulder_Slip" src="https://github.com/user-attachments/assets/7368f7c9-49ed-47a0-a76b-6b67c19f0bf9" />

2) Ice boulders: A new boulder type that can be pushed onto water tiles to create walkable ice platforms. Ice boulders automatically slide across the ice platforms they create.
<img width="240" height="160" alt="Ice_Boulder" src="https://github.com/user-attachments/assets/633f3a33-e449-4c62-935a-c9cfa538291e" />

3) Magma boulders: Another new boulder type that can be pushed into water tiles to create walkable platforms (“magma platforms”). Magma boulders slide across ice tiles, melting them and leaving water tiles in their place.
<img width="240" height="160" alt="Magma_Boulder" src="https://github.com/user-attachments/assets/ee6ad1f6-c054-4b73-8988-1b5f22b58c37" />

# Feature Requirements

## When using any feature

- 1 flag (not strictly required, but included since it will almost always be used with these features)

## When using ice and magma boulders

- 10 8x8 tiles in the Caves secondary tileset
- Most of the remaining space in the Caves secondary metatiles (in most cases, much of this can be ignored – see FAQ) 

# Usage

- All boulders use the standard `EventScript_StrengthBoulder` script.
- Ice boulders use the `OBJ_EVENT_GFX_ICE_BOULDER` sprite, and magma boulders use the `OBJ_EVENT_GFX_MAGMA_BOULDER` sprite.
- The magma platform sprites can be changed to any of the 5 cave palettes by assigning a different metatile label with the format `METATILE_Cave_Cave#_NoWater`, where # is a number between 1 and 5, to `MAGMA_METATILE_NO_WATER` [here](https://github.com/Jumpstart19/pokeemerald-expansion/blob/a36a8e5fba4631e71477b9ed56820a5b986a2fde/src/fieldmap.c#L1084).
- In the likely event that you want boulders to remain in their positions while offscreen, set the flag `FLAG_DONT_REMOVE_OFFSCREEN_BOULDER` before pushing any boulders.

# Known Bugs/Limitations

## Bugs
- Reflections occasionally show on tiles that are not normally reflective during and after sliding on ice tiles.

## Limitations
- Currently this branch is only designed to work with deep water tiles (i.e. not lake or puddle water tiles).
- Generally this is not compatible with sand tiles. While sand tiles with deep water metatile behavior will become ice tiles/magma platforms when the corresponding boulder is pushed onto them, the sand tiles will not revert back to their original state if turned into an ice tile and melted. Magma platforms also do not blend properly with metatiles that have sand metatile behavior but also have visible water in any of their layers.

# FAQ
## Q: How do I only install the feature where boulders slide on ice?
A: Copy these two commits: [1](https://github.com/rh-hideout/pokeemerald-expansion/commit/7698cbebebe1e082ccb96364224e9c921a7aef09) and [2](https://github.com/rh-hideout/pokeemerald-expansion/commit/8afda1c9a676d4711b8455a6b2b3c6726b4aae53).

## Q: I don't need metatiles for magma platforms for all of the cave palettes. Can I delete these?
A: Yes! You can safely delete any groups of 16 metatiles that start with a metatile with the label `Cave#_NoWater`, where # is a number between 1 and 5. You can also delete the corresponding metatiles with labels `Cave#_Ledge_N`. Just delete any references to the associated `METATILE_Cave_Cave#_NoWater` and `METATILE_Cave_Cave#_Ledge_N` in the code.

## Q: How can I implement this feature branch while using custom graphics?
A:
For ice and magma boulder sprites:
- Update `graphics/object_events/pics/misc/ice_boulder.png` and `graphics/object_events/pics/misc/magma_boulder.png`, as well as `graphics/object_events/palettes/ice_magma_boulder.pal`. It is strongly recommended to use the same palette for the two new boulders to avoid any palette errors.

For ice platforms formed by ice boulders:
- Create a new metatile and give it the metatile label `Ice_Platform`. Delete/replace the existing `Ice_Platform` label at metatile `0x39E` in the Caves secondary metatiles.

For magma platforms created by magma boulders:
- Create a new set of 16 metatiles following the example of metatiles starting with `0x39F` (metatile label `Cave1_NoWater`) in the Caves secondary metatiles. The absolute position of these metatiles does not matter, but **the added metatiles must be in the same sequential order as in the 5 provided examples.** That is, the metatiles must be designed with water on the same sides as in the examples (the first metatile has no visible water, the second has water on the north edge, the third has water on the east edge, etc.). For ease of implementation, give the first metatile in this sequence the metatile label `Cave1_NoWater`.

For water ledges created by melting ice platforms:
- Similar to the above process, create a new set of 8 metatiles following the example of metatiles starting with `0x3EF` (metatile label `Water`) in the Caves secondary metatiles. Again, following the same sequential order as the example with vanilla graphics is critical. Give the first metatile in this sequence the metatile label `Water`.

Also, create new metatiles for the ledges in the range `0x3F7`-`0x3F9`, giving these new tiles the same metatile labels as the metatiles that were replaced (`Water_Ledge_NE`, `Water_Ledge_NW`, and `Cave1_Ledge_N`, respectively).

## Q: How do I change the animation that plays when a magma boulder creates a platform in water?
A: Change the field effect that is called [here](https://github.com/Jumpstart19/pokeemerald-expansion/blob/a36a8e5fba4631e71477b9ed56820a5b986a2fde/src/field_control_avatar.c#L1452).

# Questions/Bug Reports
Please ping @Jumpstart in the Team Aqua's Hideout discord server.
