# Coding Guidelines

This document contains a description of the coding style this project adheres
to, reading and following this document will speed up the merging of your pull
request.

## C Guidelines

### Use malloc only as a last resort, use stack or static allocations instead.
Heap allocations have to be checked for null, might result in a syscall and
have to be freed later, thats why they should be avoided.

### Always put brackets around if blocks
This is to make it explicit which part should be evaluated if the condition
holds true, also helps against errors when adding a second statement.

### Always use Tabs for indenting, and spaces for alignment
That way everyone can choose an indentation depth, also saves some bytes,
but most impartantly its what the rest of the codebase is using

### For loops should be the primary looping construct
Also good to always add a counter with a maximum number of iterations to keep
endless loops from occuring.

### Always use safe string handling routines like snprintf

### Windows/WASM get first-class support
So do not make these releases get worse.

### Prefer ints over floats and almost never use doubles
Doubles for the most part are not needed here so try and stick to floats, or
even better, ints.

### Use type bool if you return a boolean
Should make things a bit more readable that way
