echo "Executing Godot Engine with shared libs"
cd ./godot-3.5.1-stable
export LD_LIBRARY_PATH="$PWD/bin"
./bin/godot.x11.tools.64
