# choreographer

An solution for Godot for visual scripting

# Core Design Conceit
The node and event system needs to be extensible. Ideally in a way that other addons can add their own events, nodes, etc. That way the **Core** can be super streamlined and only contain bare-essentials like type handling and conditionals.


## Challenges:
* Serializable node graphs
	* Needs to contain not just the flow of the nodes, but the logic of them too.
	* Otherwise maybe it "compiles" into a bytecode of raw instructions. (GDScript if I'm smart enough?)
		* This would be 2 files per graph, the bytecode, and a meta-data file for node positioning and connections
	* Ideally a format compatible with Godot's conceit of VCS & human-readable data formats
* How to add a custom script type? And can I make it work correctly
* Figure out how to properly manage coordinates on a GraphEdit. Previous experience was kinda fucky

# Minimum Viable Product
## FPS Controller
If I can make a visual scripting system that can handle a first person controller, it's well on its way to being a solid foundation

# Stretch Ideas

> [!question]- Sub-Graph System
> Allow users to save fragments of a graph into a subgraph. Treat is basically as a custom node that references to the subgraph for logic processing. This would allow writing reusable code, with dynamic inputs and outputs.
> Ideally, my test of this would be creating an extra addon for the Core addon that creates a bunch of FPS graphs

> [!question]- Visual Execution/Debugging Feedback
> Create some kind of visual feedback similar to Unreal where the node that is currently being executed gets lit up when executed. This would require the node data to be stored bi-directionally in the serialization. As the bytecode would need to be aware of the graph, and the graph would need to be aware of the bytecode
>
>Possibly use this in a "Debugging" mode toggle so that the inspection can be disabled at runtime for efficiency.


# Serialization Format
##### Graph File
This will need some heavy consideration for how to properly manage it. Ideally we could use the standard Resource file format to be standard with other resources, but might need some extra specification to ensure all needed data can be loaded.

### Reasoning
Bytecode at the top, with Node IDs assigned to each execution means that debugging mode can just find the node by ID. And because it's an optional method, we can just not do that when node debugging is off. The bonus is that bytecode at the top means that processing of the file can actually end processing as soon as it hits another heading. So we can stash extra data without affecting runtime

The metadata can be consumed by the back-end as well as the Choreography editor, so things like custom themes for particular graphs can be set. Maybe even have a simpler approach such as having overrides for the main colours of a GraphEdit node
