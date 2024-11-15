# Choreographer

this is a simple scripting language written in rust, intended to be a purely visual scripting lanaguge that:
- Is *very* VCS friendly (no binary blobs)
- Is highly modular (use built-ins or scrap it and write your own custom stdlib if you want)
- Is unbound by ties to any existing system or engine
- Is cross platform


## Lofty Goals

The goals of this language are pretty lofty, and of course I am one person making little improvements whenever I feel like it. Even worse I am *not properly experienced* or even knowledgeable about writitng programming languages. So overall this project is going to be slow, naieve, and generally bad.

But hey, maybe some day it'l turn into something useful


## Crates

All of the crates use "cho-" as a prefix because `cargo add -p choreographer-lib <crate_name>` is a lot to type for adding a single crate

### Lib

This is the core of the language. It is where most of all of the strict definitions of how this language operates and is stored goes here.

### Runtime

The runtime. This need not be the end-all runtime for this langauge, but it is something I'm making to run standalone scripts as I need. Ideally this runtime will also handle registering external dependencies into the `TypeRegistry` to allow for collaboration 

### Cdylib

Currently not in active development, but the goal is to expose as much of possible from the Lib over FFI so basically any language can bind to and process this scripting language

# Licensing

Dual-licensed MIT & Apache 2.0

See license files for more details