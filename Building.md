# Building Choreographer (c11r)

This is largely shown in the [C++ Modules Tutorial, in the godot documentation](https://docs.godotengine.org/en/stable/development/cpp/custom_modules_in_cpp.html).

## Building Static (Release)

```bash
scons -j8 custom_modules=../modules c11r=static
```

The `c11r` flag can be either `dynamic` or `static`. Dynamic will compile the module as a shared library, which is external to the godot executable. Static will compile it "into" the executable, which is the standard process for stable release versions since it creates a single executable file, without worry of losing track of external files. This is the recommended system for building this yourself.

## Building with a shared lib (development)

```bash
scons -j8 custom_modules=../modules c11r=dynamic
```

After you have run the previous command, you could also use this command to just compile the module, without recompiling the engine.

```bash
scons -j8 custom_modules=../modules c11r=dynamic bin/libc11r.<platform>.<arch>.<lib extenstion>
```
For example, on Linux 64 bit, it would be `bin/libc11r.x11.tools.64.so`

See [the tutorial](https://docs.godotengine.org/en/stable/development/cpp/custom_modules_in_cpp.html#improving-the-build-system-for-development) for more information on building this way.

## Help

More information about building on specific platforms can be found [here on the Godot Docs](https://docs.godotengine.org/en/stable/development/compiling/index.html).

If you have interest in development, either to contribute to this project, or to make your own modifications, feel free to review the documentation on [developing custom modules](https://docs.godotengine.org/en/stable/development/compiling/index.html). It has a wealth of information that can help you get started with development for this project, or even for Godot Engine directly.


# Changing godot engine version for development
Current development is targeted against release 3.5.1-stable. A main benefit to this is that the godot repository has the releases for stable versions. In the top-left corner of the release, you can find the release commit, which is the last commit for the release. Clicking on that will open information about the specific commit and see the full commit ID. That is what is needed to change the engine version we build against.

The commit ID for 3.5.1 is `6fed1ffa313c6760fa88b368ae580378daaef0f0`

So to change the godot submodule to that version, we need to run

```bash
cd godot
git checkout 6fed1ffa313c6760fa88b368ae580378daaef0f0
```

git should spit out some information, and show the title of the commit. If that matches what you see on GitHub, you've done it correctly. Now you can start trying to compile and tackling the ocean of bugs that no doubt appeared from the sudden change.

Because the module is developed outside of the submodule directory, this change is non-destructive. You shouldn't be touching the submodule code, rather focus on the c11r module code. This can let upgrading to newer versions of the engine super easy. That is until we upgrade to GDExtension.