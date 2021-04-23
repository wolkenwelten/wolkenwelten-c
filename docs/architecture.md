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