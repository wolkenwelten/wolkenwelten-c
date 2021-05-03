# Nujel

## Overview
The Scheme inspired programming language powering many parts of this game. While
Scheme was a heavy influence, it is by no means compatible to any Scheme standard.
This is mostly because Nujel is not intended to be a general-purpose language,
instead it's only objective is to be the way to describe WolkenWelten gameplay
modes, and in all other areas where a more high-level/abstract language would
make things easier than C. Additionally there is no great plan, the language will
be changed all the time to make it better suited for its task. Though some things
are already quite clear and are very unlikely to change, I will focus on some of
these below.

### Nujel is interpreted
This has multiple reasons, the most important one that JITs are hard to program,
and make porting to new Architectures a lot harder, especially to platforms like
WASM. Additionally all perfomance critical code should be written in C, with
Nujel mostly acting as some sort of glue layer. Although it would be nice if
Nujel became one of the faster interpreters, right now it is slow, super SLOW.

### Nujel has no exceptions/errors
For example `[/ 4 0]` returns #inf instead of throwing an exception. Or trying
to resolve an unknown symbol just returns the symbol instead. Whatevery the case
the Interpreter is not allowed to at any point throw an exception, even if the
interpreter runs out of memory it just starts returning #nil instead. This is
mostly because I would like to experiment with randomly mutating behavioral code,
and it would be kind of a bummer if most of these would just end in the bunny
generating Floaing Point Exceptions.

### Nujel uses brackets
This is mostly because of the standard keyboard most of us use, where typing a
parenthesis requires holding down shift and 9/0, whereas you can type brackets
directly. Although parenthesis and brackets are totaly interchageable, brackets
are still preferred.

### Nujel can do infix evaluation
The LISP way of writing things is just not something that many people are
accustomed to, so in order to make it easier to start writing Nujel you can just
write something like `[display [1 + 2 * (3 + 4)]]` and get the expected results
back. Operator precedence is also the way you would expect it to be.

### Nujel has no I/O by default
This is because Nujel will run untrusted code, and having it be able to just do
I/O would be horrible from a security standpoint, especially considering that on
joining a server the client receives and Evaluates whatever code the server
sends (which should just describe the items and general gameplay stuff, but in
the end might be whatever the server operator decides it to be).

### Nujel has an extensive testsuite
Every single test gets bundled into every single Nujel interpreter, which might
even add tests specific to that context. This should help in decreasing the
amount of accidental breakage making it into commits. Every new Nujel feature
should start as a couple of testcases describing expected behaviour.


## Future Direction
Now in the future there are a couple of things that need to be changed in order
to progress further.

Nujel still needs continuations and a proper event loop that is able to handle
thousands of coroutines. The design will mostly be based on JS and the way it
does promises/async/await but with one difference, namely that await would be
the default and the programmer has to specify if something should happen in
parallel. This should make programming quite a bit easier since one can just
write normal blocking code but still have the benefits of lightweight coroutines.

In addition to that Nujel needs a way to limit the amount of operations that a
function can do before returning a continuation upwards. This is mostly because
we only have a limited amount of time before the next frame needs to be rendered
and we might need to keep the continuation around until next frame where we can
then hopefully finish it. Additionally since we run untrusted code we need to be
able to run infinite loops without the game crashing, this is probably best
implemented by limiting the amount of operations animals can do in their AI and
also having the actual amount of operations done decrement the animals hunger/energy
level which will hopefully result in evolutionary pressure keeping complexity down
(and that would also result in the quick removal of animals with an infinite loop
in their behavioural code). This can also be used to profile code (just run a
function for X operations and see in which functions it currently is), or to
debug/single-step through code (just run for 1 operation and the have it return
a continuation). This same system should also be able to limit the amount of
values a function may allocate, since memory is finite as well.

And most importantly, Nujel needs to be able to run it's GC while it is evaluating
something, this is a massive limitation right now but should be fixed with the
work needed for supporting continuations as well.

Not much effort should be spent on adding support to third-party tools apart from
an Emacs major mode, mostly because the best way to write Nujel should be within
Nujel so the most work should be put into making this experience the best there is.


## Tutorials
A couple of tutorials introducing the language is essential, but not a priority
right now since everything is still in flux. Once the language has stabilized
and continuations/limits are implemented a series of interactive in-game tutorials
will be written inteded for everyone 9-99, not just seasoned programmers.