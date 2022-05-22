This document serves to guide the development effor in a way that prioritizes gameplay
and extending the underlying runtime only as necessary.

I am trying to list things in an order that they should probably be developed in, although
the exact order is not that important.

# Legend

## [GM] = Gamemode
A Gamemode is technically just a module, although it will be treated slightly
different by the UI since a player has to choose a particular Gamemode.

## [MOD] = Module
Modules by themselves should be as small as possible and import other modules
in order to provide complex functionality.

## [C] = C Functionality
Something that the engine needs to implement and expose to Nujel in some (hopefully) sensible way.

## [NUJ] = Nujel Functionality
Something that Nujel needs to provide by that point


# Milestones

### [C] - Blocktype getter - DONE!
Functionality is mostly there, just not exposed to Nujel.  Should be quite simple to provide.

## [MOD] - Blockbreaker - DONE!
This mod should allow a player to immediatly remove a block from the world as a primary action.

## [MOD] - Blockbuilder - DONE!
This mod should give a player the ability to freely choose from all blocktypes currently known
to the game and place them via a right-click.

## [MOD] - Blockchooser - DONE!
This mod should provide a simple window when pressing the inventory key and show a nice grid
of all known blocks.

### [NUJ] - Bytevectors / SRFI-4
Nujel needs a way to store (and manipulate) bytevectors and transform them from/to strings/arrays.

### [NUJ] - LZ4 (de-)compression
Since WW already includes LZ4 we should do the same for nujel and allow usage on all SRFI-4 vectors

### [NUJ] - Binary reader macro
We need a way to efficiently encode binary data within S-Expressions, mostly to encode chunks but also for
other data like Textures, Models, Chunks, SFX and probably much more.  We need to also test if base64 would
actually be more efficient than hex, since we will probably never send/store these s-expressions uncompressed and
Hex might compress slightly better (as well as being simpler to en-/decode)

## [MOD] - Chunk import/export
A way to import/export a particular chunk and serialize in into an LZ4 compressed S-Expression.

## [MOD] - Chunky files
A format for storing blockdata, along with palette information mapping bytes to symbolic blockIDs. Plugins used may be stored but should not be loaded automatically, instead an exception should be thrown and the import cancelled.  This is so that we may migrate over to a newer/different version of a plugin with everything working so long that we have a blockType associated with a particular symbol.

### [C] - Worldgen
Make the entire worldgen process defined via Nujel, although the current algorhithm should be provided to Nujel so that we can provide
a very simple pass-through worldgen that just calls back into the old worldgen code.  Will be chungus based in the beginning, but in the
long run there needs to be some better structure.

### [C] - Weather-/Cloudcontrol
Weather and Clouds need to be controlled via Nujel since they don't make sense for all Gamemodes.

## [MOD] - Static chunks
A worldgen mod that just inserts a static chunk whenever it is called. Should allow for a sort of copy/pasting of blockdata.

### [C] - Grappling Hook - DONE!
Grappling hook must be controlled from within Nujel

### [C] - Glider
Glider must be controlled from within Nujel

### [C] - Jetpack
Jetpack must be controlled from within Nujel

### [C] - Health
Health, mostly maximum Health, must be controlled from within Nujel. Damage would be nice but not necessary in the beginning.

### [C] - Knockback multiplier
We need a way to specify a knockback multiplier that is universally supported

### [C] - Respawn
Respawning (like a delay or something) and especially respawn location needs to be controlled via Nujel

### [C] - Entity collisions / Item Pickups
No itemsystem, but instead just a way of placing entities with a specific mesh, maybe even adding some sort of rotation animation and calling a callback (A) After a specific amount of time and (B) On collision/pickup by a player

## [MOD] - Air Gun
Should move all entities in front of it far away

## [MOD] - Baseball Bat
Should damage and knockback whatever it hits

## [MOD] - Rocket launcher
Pretty self explanatory

## [C] - Persistence
Persistence should be controlled via Nujel, mostly turning it off for Gamemodes where that doesn't make sense

### [C] - Player speed multiplier
Player speed should be modifiable via Nujel, so we can slow down or speed up players as needed.

## [NUJ] - Module system
Nothing fancy, might in the beginning actually suffice to just load and evaluate a certain
files.

## [C] - Gamemodes
Actually support for gamemodes, where a player actually switches to the gamemode required by the server. Does not need
to be loaded automatically from the server in the beginning, although it should be kept in mind while implementing.

# [GM] - Creative/Development Mode
Should mostly serve to test various mods as well as function as a map editor for other gamemodes/mods.

# [GM] - Trümmerei
The first real Gamemode, an arena-shooter where just like in smash instead of damaging the other players you instead increase a knockback multiplier. Players should start with pretty much nothing and then have to run around the (static) map picking up various items/weapons as well as useful items like grappling hooks, gliders or seven-league boots

### [C] - Player models
We need to provide a couple of player models and let players decide which they want to use.

### [C] - Models / Textures / (Mesh)objects
Models need to be compiled into the client for now, it would be much more convenient if they could be provided by Nujel, meaning that they could be provided by Modules and soon the server.

### [C] - Multijump
Would be expose some sort of multi jump ability via Nujel, or at least enable its implementation in Nujel

## [MOD] - Mapchange
Should provide a convenient way to change a "map"

## [MOD] - Maprotate
Should provide a way to automatically rotate between a preselection of maps.

### [C] - Blocktype setter
Functionality is mostly there, just not exposed to Nujel.

## [MOD] - Default Blocks
Should import a set of simple and general blocks which will probably be useful to pretty much every gamemode out there, block like
grass, dirt, stone and so on, basically whatever blockTypes are currently in the game, although the snowy ones can probably be removed.
