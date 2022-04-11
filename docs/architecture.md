# Architectural overview

The codebase is split into 3 big categories: `client/`, `common/` and `server/`.
This is the case because WolkenWelten is basically multiplayer only, even a
singleplayer game is starting a server that runs in the background (albeit only
with a single client).
Since they also share object files we only know the context during runtime, for
that you can do an `if(isClient){}` for differing behaviour, although that should
be kept to a minimum.

Most files exist in all 3 directories where the client/server side specific files
include the common header file. If you create a new file for something gameplay
related you should try and stick to that structure so includes dont have to be
changed afterwards.

Anything physics/gameplay related should be tick based and never query the
current time or something like that, `client/src/main.c` and `server/src/main.c`
show that structure where for every 4ms passed a single tick occurs that calls
the updateAll functions of every subsystem.

For testing purposes I would highly recommend going through `client/Makefile.client`
and look at the different run* targets, although `make run` will probably be the
most useful one.


## Client Side

The client actually does some simulations of its own so the latency is not that
apparent, apart from that all player simulation is done completely on the client
side so that jumping/swinging around still works just fine even if the latency
can be measured in seconds.


## Network Protocol

The client/server are communicating over TCP/IP (WebSocket for WASM clients) and
everything is message based with a fixed header determining its type/size.
Compression is achieved by the special compressed message type that when parsed
yields many uncompressed packets that get parsed immediatly. Only server->client
is compressed because, for now, the client is not sending all that much. To take
a look at what gets sent over the network there is a network profiler that keeps
track of how many messages of which type have been received, you can call it
from the lisp console via (nprof) or (s (nprof)) depending on the side you are
interested in.

Most messages update a struct on the clientside and the server just sends those
regularly, even if nothing changed, this is so a desync will be overwritten the
nest time a sync packet for that struct arrives.


## Server Side

The server side is mostly just doing calculations and then answers to messages
from the clients. Updates are only sent in response to a player update that
every client is supposed to send each frame, this is done so as to not overwhelm
a client with messages. Apart from that the server keeps track of which chungus
each client is interested in via a bitmask, and also keeps track of changed
chungi/chunks which are regularly checked and then lead to chunkData messages
being sent out to the clients that are subscribed.


## Nujel <-> C Interface

When something happens in the game world that might be of interest to some Nujel
code the C code will call an event handler and pass along data specific to that
event.  For example when the character bounces of some block it might call the
character-collision event handlers with the value `@[:character #CHAR :block 3 :intensity 123.123]`
which might then be passed through two handlers, one which lowers a players health
if the intesity is above a certain threshold, and another one that procuses a sound
effect based on :block and intensity.  What is important is that these two handlers
are completely separate from each other, and even if one throws an exception it should
not interfere with other event handlers (although it should probably be logged).

### Modules

To allow for complete customization WolkenWelten will make use of the Nujel
Module system, where every server depends on a single module for that particular
Gamemode/Map which in turn depends on other modules providing whatevery is
necessary for the game.  In addition to that there will always be modules injected
by the client which is responsible for the control scheme and a player may choose
to override defaults by adding custom modules.

These modules can import/export values as they see fit, but most importantly they
have to bind event-handlers. this is helped by the fact that all dependencies will
be loaded and initialized before the current modules code will be evaluated, this
allows not only for appending event-handlers and using code from other modules,
but also allows for removal, filtering and general manipulation of data being fed
to other depended on modules. The order should be:
1. Core
2. Controls / Settings
3. Gamemode
4. Usercode

In the beginning these modules will have to be included in both client and server
but as soon as time permits most gameplay code should be moved to the server and
then sent to the client on connecting, this allows for convenient mod delivery,
additionally these modules should be stored on the client for later usage, as
well as to allow a client to use these modules in servers they might set up later.

In general these modules should be very fine-grained and only import what is
absolutely necessary, although some meta-modules might come in handy that do
nothing but import a selection of other (meta)modules.

By enforcing all interaction through these event-handlers we can easily change a
gamemode in an active game by resetting all event-handlers and importing the new
gamemode, triggering a reinitialization.  Additionally we can easily enhance a
running game by just writing something like `[import :items/bazooka]` into a
listener.
