- Precision Value for World generation, can be different for each chungus, the farther any players/animals are the less precise the simulation gets
- SHRINK THE BLOCKS!
- Proper water system
- Proper light system
- Add block variations by having each face use a 2x2 texture, and instead of having uv be in the range of 0-1 use 0-0.5 for single blocks, thereby adding some slight variations along the way.
- Animals don't spawn after the limit has been reached, especially problematic if the limit is already reached with animals inside the spawn area which never gets freed
- Simulate/Store Oxygen levels in chunks and have it spread to neighboring chunks, trees/plants increase oxygen, fires/animals/players decrease oxygen

--- Nujel work ---
+ Write "compile" lambda that resolves all symbols for a given λ, which should make things a lot faster
+ Store symbols in a BTree
+ Arguments should all be single greek lowercase letters to easily distinguish them from normal test/code, also it looks kinda schway
+ Native Hashmaps
+ Native Pointers with an associated type ID, which the GC uses for allocing and freeing the native object (especially useful for GUI widgets)
+ TCO?...!

--- Off-stream work ---
+ Fancy Ropes
+ Fix WASM Singleplayer

--- Low Prio Work ---------------------------------------------------------
- Disable players equiping random items in their equipment slots
- Make Bunny populations more stable
- Remove bigChungus from serverSide
- Enable Compression for wasm/ws connections
- Migrate to OpenAL
- Simple Checksums for Savegames
- Implement projectile flag to have the projectile bounce off surfaces
---------------------------------------------------------------------------

(A) Armor
(A) Proper Character Models
(A) Some kind of jumping at the end of some grappling hook action

(B) MacOS Code signing
(B) More worldgen variety

(C) Make Blocks Modable
(C) Enable tls/wss support (BearSSL seems nice!)
(C) Windows Code signing (seems expensive :/)
(C) Fix wasm fullscreen problems

(D) SFX work
(D) Portable/Distributable BSD versions

(E) Add VoiceChat (G.711 ulaw FTW)
