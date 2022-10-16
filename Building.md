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
