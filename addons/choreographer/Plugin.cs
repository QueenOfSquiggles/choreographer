#if TOOLS
namespace c11r;

using Godot;
using System;

[Tool]
public partial class Plugin : EditorPlugin
{

	Control graphView;



	public override void _EnterTree()
	{
		graphView = (GD.Load("res://addons/choreographer/scenes/GraphView.tscn") as PackedScene).Instantiate() as Control;
		AddControlToBottomPanel(graphView, "Choreographer");
	}

	public override void _ExitTree()
	{
		RemoveControlFromBottomPanel(graphView);
	}
}


#endif
