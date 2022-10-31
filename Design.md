# choreographer

An solution for Godot for visual scripting

# Core Design Conceit
The node and event system needs to be extensible. Ideally in a way that other addons can add their own events, nodes, etc. That way the **Core** can be super streamlined and only contain bare-essentials like type handling and conditionals.

# What is a "block"?
Block is the term we are using instead of "node" for singular graph vertices. Because Godot uses "node" as a name for a different structure, we want to reduce confusion by calling our equivalent something else. So they are "blocks" like "code blocks".

# Extensibility
There are 3 core ways to extend functionality, 1 (maybe 2?) of which allow scripting in other languages, which allows for expanding functionality.

## Sub-Graphs

Sub-Graphs should be fairly self-explanatory. They are an external graph script that defines inputs and outputs. When loading into another c11r script, it is a single node with the inputs and outputs represented.

### Reasoning
This mode allows developers to simplify their code by extracting common operations into seperate files and "include" them as a single node into their scripts. This is done entirely with c11r so no knowledge of other scripting languages is required.

## Custom Blocks
Custom Block are singular nodes that are defined in a different scripting language. These nodes can be added to a BlockPack, which can be loaded on a script-by-script basis or as a default pack which loads at startup.

### Reasoning
This allows more experienced developers to create custom functinality and macro-level solutions to project-specific problems. Any level of complexity is allowed for these blocks. Because it is being scripted in a separate language, they can be made lower level or higher level. From GDSCript, to C#, to GDNative. 

The idea is two-fold:
- experienced programmers on the development team (of a game) can make specific blocks for their team-members to use that are specific to project they are working and reduce the complexity of creating content.
- community contributors can make custom BlockPacks that can be distributed and used widely. Ideally, these block packs would each solve a specific problem. For example a platformer BlockPack would have blocks for standard character motion, pathfinding, etc...

## Composite Blocks
This is the way c11r handles inheritence. Any one c11r script can have more than one composite block. However, when triggering a function, all composite nodes will be triggered in no specific order, unless the function is overridden in the child script. My hope is that these can be made in any needed scripting language, but that may be difficult?

### Reasoning
CompositeBlocks allow for a base functionality to be created with specifics altered in the child script. This is basically function overrides. Ideally, these composite blocks will have inputs so we can have exported values altered in child scripts. But overriding any changes needed. My specific vision for this is to have an "FPS Controller" script that is composited into a c11r script, and any changes needed are tweaked for the specific game. These scripts should be able to be loaded into a BlockPack as well for registry so the composite blocks can be added to scripts easily. 

### QueenOfSquiggles's Note
I really like interfaces. Specifically the way they work in the Java-similar languages (C#). One of my hopes is that these composite blocks will allow interface structures in scripting, and this would enable better structure to the development of code in a modular fashion. I will still allow for duck-typing, but the default for calling functions will be a dropdown list, with "custom" as an option.








# File Format

The plan for file formatting is to conform to Godot's built-in ConfigFile style, which is pretty much TOML. E denotes an "entry" which how we achieve functions. A, B, C, D are example blocks. The ':' separates the ID of the block and the index of the port. We define all connects as output(input) such that `E0:0(A:0)` means that the 0th output port of `E0` is connected to the 0th input port on `A`. The formatting is a bit different since we are using the ConfiFile format to be consistent with the Godot standard resource and scene descriptions.

```TOML
[ext_resource path="res://blocks/core/lang.gd" type="Script" id=1]

[defintions] # these might be converted to headers like node definitions in TSCN
E0 = {"type" : "core.lang"}
A = {"type" : "core.lang"}
E1 = {"type" : "core.lang"}
B = {"type" : "core.lang"}
C = {"type" : "core.lang"}
D = {"type" : "core.lang"}

[graph] # this could be less verbose in a different format?
E0:0 = "(A:0)"
E1:0 = "(B:0)"
B:0 = "(C:0, D:0)"

[functions]
_ready = E1
_process = E0

[meta]

A = {"test" : true, "note" : "this is a testing note"}
groups = ["(E0, A)", "B, C"]

```


I'm considering allowing a "dense" format that would reduce unecessary content while sacrificing readability. It's already rather difficult to see what is happening as is, but it fully describes a graph and execution options.

I still like keeping the internal data for each block being JSON because then making custom blocks has easy serialization for data. 