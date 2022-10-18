echo "Executing Godot Engine with shared libs"
cd ./godot
export LD_LIBRARY_PATH="$PWD/bin"
bin/godot.x11.tools.64 -e --path "../demo/"
