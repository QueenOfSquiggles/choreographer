# Choreographer (c11r)

Choreographer is intended to be a visual scripting solution for the Godot game engine. The main goal of this scripting system is to improve upon previous iterations, while still existing as an optional system.

Heavy inspiration is taken from Unreal engine's "Blueprints" as that scripting solution is a very well developed system that is approachable to beginner developers.

# Design Goals

## Modular

This scripting system is intended to provide a highly modular system for developers with different skills

### Node Packs (GDScript/C# skilled developers)

A "Node Pack" would be an addon for Godot that provides a set of nodes to this addon as well. The workflow for creating a Node Pack will be worked out as this project is developed. The intended process is that these addons could be scripted in GDScript or C# because the interaface for registering custom nodes should be simple enough for a single user to make custom nodes for themselves. But a Node Pack is intended to have a pack of coherent nodes that all serve a very specific purpose

#### General Purpose vs Specific Purpose

One of the flaws with the original Godot Visual Scripting was that, like the engine, it was designed to be general purpose. This means that any developer, for any project would be able to use it for their project. However, this became a flaw in the Visual Scripting side because in order to be truely general purpose, the differences between visual scripting and text-based scripting became fewer and fewer. And generally, typing is going to be faster than visual scripting.

The core benefit that I (QueenOfSquiggles) believe Visual Scripting has over traditional scripting is that it can have single nodes that exeucte over multiple frames (even over several minutes) and performing specific sequences or logic. This would speed up development. And generally, the more specific the solution, the more useful it would be. For example, a Node Pack for 2D platformer nodes would be able to provide considerations like "coyote time", and unique signals like "head_jumped_on" which would be incredibly useful for a platformer game, but these nodes would just be bloat in a 3D DOOM clone, or a software tool. In order to ensure that this is useful to all developers, this core addon will provide the framework for visual scripting, while allowing anyone to implement their own custom nodes that perform custom logic. Because this will be based on the Godot scripting languages, this will allow nodes to execute any code that could normally be written in standard code systems.


### Sub-Nodes (c11r skilled developers)

For developers with less programming experience, or simply to speed up development in c11r, a "sub-node" system is planned. This concept should be familiar to other node-scripting systems. Essentially, users can save a node graph as a custom node, which would then be able to execute a common sequence in multiple locations by use of the custom node. This also means that Node Packs could be comprised of sub-nodes exclusively, however this would not allow them to implement any functionality that doesn't already exist in the scripting system.

## Fun

Because of the visual nature of this scripting, it's possible to add extra effects to the scripting experience to make it more enjoyable.

### Feedback for different events such as
- Node Creation
- Node Deletion
- Copy, Paste, Duplicate Nodes
- Connection Made
- Connection Removed
- Add Node Popup appearing

This feedback could take the form of particle effects, "screen shake" (simulated), and audio feedback. All of these features would be optional and/or require external addons to implement. This will be worked on after core functionality. Since the scripting has to function first.

## Debugging Support

In a debugging environment, it is intended that the node graph would also serve as a debugger because the nodes that are currently being executed will be highlighted (or some other feedback) to show the user which nodes are being performed. This would be helpful as different addons could add nodes that are more complex that require execution over several frames.

## For beginners to game development

Godot itself is fairly approachable and has extensive documentation. The goal of c11r is to let scripting be even more approachable as well. GDScript is an easy language to learn, and would provide more capabilities, but does require some syntax learning and practice. Ideally, a node-graph based system that focuses on Signals and Events as triggers for certain functions would create a more robust scripting environment that is highly approachable even for beginners to programming.

## For experienced developers

Maybe? I (QueenOfSquiggles) have not personally used a visual scripting language for game development in a long time, so I can't say whether this scripting solution will be a better option compared to GDScript or C#, ignoring programming skill factors. Very likely C# would be the most runtime efficient solution for scripting as well as having robust libraries with NuGet packages.

# Origins

I (QueenOfSquiggles) desperately need/needed a job (September 2022). And a recommended portfolio building project is to create a "game development tool". And while I could just as easily make something small, fast, and generally useless to the larger gamedev community, I want to make something that will make game development easier and more enjoyable. The Godot workflow is already very enjoyable as it is, but hopefully this scripting solution can add some more joy to the process of creation.


# Contributing

Until version 1.0, this addon will not be ready for public consumption. If you want to help with development, contact me (QueenOfSquiggles).

Please refer to the Design.md file which will be updated with design plans if plans are changing.

~~Contributions are greatly appreciated. Opening an issue for any bugs or feature requests are welcome. Be sure to see if you can find an open issue for what you are looking for first!~~

~~Issues will be available to be worked on.~~
~~Pull Requests are for solving open issues.~~

# Development

The main branch will track the latest version of godot, with other branches being broken down into "development" and "stable" versions. For example, the "3.5.1-development" would be an unstable development branch for Godot Engine version 3.5.1 (the latest stable version as or writing). And "3.5.1-stable" would be a PR-only branch which is almost guaranteed to be stable for development.

Because this is currently be developed as a "Module" for godot, it does need to be compiled along with the Godot Engine as well as using custom export formats. In the port to Godot 4.X, this is expected to change, but at the current moment, development is targeting 3.5.1 since that is a stable version. That way, we know that if Godot crashes during development, it's our fault, not a bug in a alpha or beta version.

## Financial Help

Direct code and open issues will help with development. But another consideration is money in pocket. If you would like to contribute financially to the development of this project, you can donate to the team individually. Perhaps at some point we will have a single common system, but for now it is on an individual basis:

##### Support QueenOfSquiggles
- [Kofi](https://ko-fi.com/queenofsquiggles)


> *This section will be expanded should team members be added <3*

# Contact

QueenOfSquiggles - Twitter: [@OfSquiggles](https://twitter.com/OfSquiggles)

# Licensing

The current license is listed in License.txt, which is the MIT License, following Godot Engine's lead.

The goal of using this license is to be as permissive as possible, while still protecting the developers and contributors from any potential bad actors.

The simple, human readable terms are this: Feel free to use this in public, commercial solutions, and any project you could normally create legally in Godot.

Attribution is **greatly** appreciated. If you would like to provide an attribution for this extension and its developers, you can list it/us as:

```
Choreographer (c11r)
developers:

QueenOfSquiggles

```
This will be updated to include contributors as listed on GitHub
