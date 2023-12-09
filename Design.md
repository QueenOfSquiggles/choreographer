> **Preface:** For simplicity, this document is written in present tense for specific end-goals and in future tense for possible but unlikely goals. This project could be considered to be "complete" once all present tense declarations are true. 

# Choreographer
Choreographer (C11R) is a GDExtension based scripting language for Godot 4.2+.

Similar to Unreal Engine's Blueprints system, it uses a Node-Graph Event system to manage code paths. Where an Event is equivalent to a "method" or "function" and is not necessarily related to Events in the context of an Observer Pattern.

## User Groups
There are three core user groups of interest for Choreographer:
- Game developers
- Tooling Developers
- Team Collaboration

### Game Developers
Game developers are (usually) less concerned with what they use to make a game and more concerned with actually finishing and releasing their game. To that end, Choreographer is intended to be highly usable for increasing efficacy of development as well as lowering the barrier to entry for developers who may not have experience with traditional programming languages.
### Tooling Developers
Tooling developers enjoy creating tools for Game Developers to use. They enjoy clean and documented systems which allow them to create interesting resources with minimal effort. Tooling developers are more likely to contribute back to the source project if they deem it worthy and/or interesting.
In order to serve tools developers, as well as to leverage their skills, Choreographer provides a simple "Block" registration system which allows custom logic blocks to be registered from a plugin and can be used from within the Choreographer Scripting Interface
### Team Collaboration / Professional Development
Teams, both for smaller ventures like game jams and larger ventures like a complete game company, require specific features that may not necessarily be needed for individuals. For example teams will benefit from being able to leverage Version Control Systems such as git to collaborate more effectively. Additionally, teams are typically comprised of people with a diversity of skills and experience. In order to best leverage their unique skills and talents, they need to be able to leverage each other in mutually beneficial ways.

In order to best serve teams and professional groups, Choreographer uses a text-based (XML) file format, which allows easily readable files which can ease aches and pains from merge conflicts.
Additionally, the same feature which allows tooling developers to register custom logic blocks allows individual team members to provide logic blocks internally to their own team. And those highly specialized logic blocks can be used to increase the efficiency and ease of development for all team members.

# Language Specification
## Definitions
"Script" - a single file and class of Choreographer.
"Block" - a single node in the graph of a script.
"Event" - a method contained in a C11R Script comprised of a directed acyclic graph of Blocks.
"Connection" - a relationship between two Blocks marked by a line
"Emit"/"Emitting" - refers to a situation where the connection is on the right side of the block meaning it is an output
"Receive"/"Receiving" - refers to a situation where the connection is on the left side of the block meaning it is an input

## Graph Systems Review (Mathematical)
The graphs in Choreographer are "directed acyclic graphs", which is a specific type of graph which allows for validation of certain properties. For simplicity the details are described here:

"Directed" - connections between nodes go in a particular direction and can but are not necessarily bi-directional
"Acyclic" - there are no sequences of connections in the graph which would allow a "cycle", or a chain that loops infinitely. 

Directed and Acyclic graphs are both long-standing mathematical descriptions and have a plethora of algorithms available for managing them.

In terms of the mathematical description, "nodes" can also be referred to as "vertices", but in this specific case we call them "Blocks" to avoid confusion with the `Node` types in Godot.

## Event Systems
Each `Event` is a sequence of operations. A specific type of connection called the `Event Connection` is connected across the graph to determine the flow of logic. Other connections are of a value type, called `Value Connection`. Internally, these are an `enum Connection` which allows every connection to be one of each type, not both, and not nothing(`null`).

### `Event Connection`
Event Connections work through a push system. Once the block emitting the event connection completes execution, the event connections which are marked as active are used to find the next Block(s) in the event graph. 


### `Value Connection`
Value connections work through a pull system. When a block is triggered to execute, it checks in memory for the input data. If any receiving port has stale (`null`) data, it first triggers the emitting block to regenerate that data, this can trigger recursively if necessary.

## Block Definitions
Blocks can be defined in a variety of ways depending on the source of their creation.

All block definitions require a data segment and a logic segment as follows:
- Name (String)
- Documentation (String) (Optional)
- Inputs (Array\<Connection\>)
- Outputs (Array\<Connection\>)
- Execution Function (sig: `fn(Vec<Connection>) -> Vec<Connection>`)
- Input Icons (Array\<String\>)
- Output Icons (Array\<String\>)
## Core Library
The Core Library, or the collection of Block Definitions made available by Choreographer itself, are all namespaced to "core". These block definitions are the basic units of logic for Choreographer and are registered internally by defining them as rust structs that implement the `IBlock` trait and submitting them to the `ChoreographerServer` registry.

## Third Party Libraries
Third Party Libraries are collections of blocks registered by some means besides Choreographer. This is the intended way to add specificity to Choreographer.

While it is potentially possible to define and register these block definitions through an additional GDExtension project, it is recommended to write custom block definitions in GDScript and register them through a plugin. 

The reasoning behind this is that I am not entirely certain how to use Choreographer as a library for type definitions without simply appending it to the project as a dependency, which could potentially cause problems with FFI. Should someone smarter than me know how this part of GDExtension/FFI works and is able to help with this side, registering custom blocks through GDExtension could become possible. However, please remember that the end user will require a binary for every platform they may want to publish their game on, and as such the number of GDExtension libraries in their project will exponentially increase the difficulty of exporting if those libraries are not pre-compiled for the user's target platforms (Think Mobile and Apple platforms, which are tricky).

### How Third Party Block Definitions work internally
Internally, there is a "CustomBlock" block definition which redirects standard `IBlock` function calls to a series of `Callable`s point at particular function names. 

> In the future, when abstract method definitions become available, I want this to be refactored to use abstract methods instead, which should ease the pain of development

The method of storage for these callables should be `Option<Callable>` such that when encountering a value of `None`, we can emit a warning message and skip processing.

# Virtual Heap Memory
Choreographer uses a heap memory abstraction. That is, a `Vec<Option<Variant>>` which allows indices to serve as a memory address and value checking on the rust-side of things.

The organization of the heap is as such:
1. Script local variables (effectively static as variables cannot be de-initialized until the script itself is dropped)
2. Temporary Event execution memory (dropped upon event completion)
	- An option available to the user could be to choose between memory usage and performance. For example, to save performance costs, the memory heap could simply be nulled (`None`) out rather than resizing the `Vec`. But this could create a memory cost as each execution unit increases the size of the nothing array. The immediate deletion would remove need for some kind of garbage collection system, which I see as a band-aid for bad memory management.

Each script is in charge of its own virtual heap memory. 

> Note, though it is referred to as "virtual" the data being stored is also literally in the program's heap memory. But by virtualizing it the scripts create an effective memory arena such that the entire block of memory can be dropped all at once when the script instance is dropped.

## How do individual blocks interact with the virtual heap?
The script execution maps inputs and outputs to virtual heap addresses (`usize` value) such that when an output is calculated it is assigned to the address that inputs requiring that value are also targeting. A value goes stale once there are no remaining inputs that target that address, and as such can be dropped. 

It is possible to develop a system by which stale memory is detected throughout the execution of an event graph, this would certainly increase the efficiency of larger event graphs, however this may be difficult to implement based on a variety of factors. Testing will be needed.

# Choreographer Scripting Interface (CSI)
The CSI is a visual representation of the data model contained in memory and in the corresponding `.c11r` file. Hence, the reasonable approach towards developing the CSI is to use the Model-View-Controller system. 

The CSI is effectively the View and Controller as the Model is the only component required for execution at runtime. CSI displays a graph of blocks using Godot's `GraphEdit` systems. And all changes are marshalled out to the controller component which in turn serializes changes in data.

## The `.c11r` format
Choreographer Script files use the `c11r` extension and are in the well tested XML file format. The logic behind using XML is that it allows attributes to be appended onto a tag, which allows for a more detailed description of individual objects without necessitating deep nesting like JSON would require. Additionally, as HTML is a superset of XML, those familiar with web development should be able to easily read `c11r` files with minimal effort.

### Example `.c11r` data/schema
```xml
<?xml version="1.0"?>
<c11r>
    <godot uid="02123"></godot>
    <dependencies>
        <res guid="02165" path="res://asdaslkhjasd"></res>
    </dependencies>
    <vars>
        <item type="0" name="some_var" value="12" isconst="false"></item>
        <item type="12" name="another_var" value="something" isconst="false"></item>
    </vars>
    <blocks>
        <block id="0001" type="IfElse"></block>
    </blocks>
    <connections>
        <link left_id="0001" left_port="0" right_id="0002" right_port="0"></link>
    </connections>
    <metadata>
        <doc id="0001">
            This is a docstring
        </doc>
        <gui id="0001" pos="1.2;3.6"></gui>
        <comment id="0001" pos="1.2;1.0">
            This is a comment block
        </comment>
    </metadata>
</c11r>
```

# Testing Challenges
Unit testing is incredibly helpful for verifying that functions operate as intended when changes are made. Unfortunately for godot-rust, the actual components that interoperate with Godot's types cannot be used because the definitions are only known when Godot loads the library into memory and initializes it over FFI. What this means is that ***no component that uses Godot types can be unit tested without a very specific setup***. There are currently tools and attempts to create a custom testing environment for godot-rust, however as of now I believe the most effective way to test a godot-rust project is to extract what logic is possible away from the Godot Types. For example the serialization/deserialization systems are based on a submodule that emits and receives structured metadata rather than a Godot type. This allows these systems to be unit tested.

# Continuation of documentation
This here is all I can think of that should be included at this present moment. Unfortunately I am a mortal and am prone to mistakes, so please let me know if something needs to be changed and/or updated by creating an issue in the repository. 
