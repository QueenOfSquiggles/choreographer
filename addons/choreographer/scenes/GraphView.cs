#if TOOLS
namespace c11r;

using Godot;
using System;
using System.Collections.Generic;

[Tool]
public partial class GraphView : PanelContainer
{

	PackedScene nodePanel;
	GraphEdit graph;

	List<GraphNode> clipboard = new();
	List<GraphNode> selectedNodes = new();

	public override void _Ready()
	{
		// onready's
		graph = GetNode<GraphEdit>("%GraphEdit");
		nodePanel = GD.Load<PackedScene>("res://addons/choreographer/scenes/node_graph_popup.tscn");

		// Connect all important signals
		graph.DuplicateNodesRequest += DuplicateNodesRequestEventHandler;
		graph.CopyNodesRequest += CopyNodesRequestEventHandler;
		graph.PasteNodesRequest += PasteNodesRequestEventHandler;
		graph.DisconnectionRequest += DisconnectionRequestEventHandler;
		graph.NodeDeselected += NodeDeselectedEventHandler;
		graph.ConnectionToEmpty += ConnectionToEmptyEventHandler;
		graph.ConnectionFromEmpty += ConnectionFromEmptyEventHandler;
		graph.DeleteNodesRequest += DeleteNodesRequestEventHandler;
		graph.ConnectionDragStarted += ConnectionDragStartedEventHandler;
		graph.ConnectionDragEnded += ConnectionDragEndedEventHandler;
		graph.ConnectionRequest += ConnectionRequestEventHandler;
		graph.PopupRequest += PopupRequestEventHandler;
		graph.NodeSelected += NodeSelectedEventHandler;
	}

#region GraphEdit Events

#region Clipboard & Duplication

	public void DuplicateNodesRequestEventHandler()
	{
		foreach(var n in selectedNodes) DuplicateNode(n);	
	}

	public void CopyNodesRequestEventHandler()
	{
		clipboard.Clear();
		clipboard.AddRange(selectedNodes);
	}

	public void PasteNodesRequestEventHandler()
	{
		foreach(var n in clipboard) DuplicateNode(n);	
	}

	private void DuplicateNode(GraphNode node)
	{
		GD.PushWarning("Duplication request for node: " + node.Name);
	}

#endregion

#region Selection

	public void NodeSelectedEventHandler(Node node)
	{
		if(node is GraphNode) selectedNodes.Add(node as GraphNode);
	}

	public void NodeDeselectedEventHandler(Node node)
	{
		if(node is GraphNode) selectedNodes.Remove(node as GraphNode);
	}

#endregion
	
#region Connections	

	public void ConnectionToEmptyEventHandler(StringName fromNode, long fromPort, Vector2 releasePosition)
	{
	}

	public void ConnectionFromEmptyEventHandler(StringName toNode, long toPort, Vector2 releasePosition)
	{
	}

	public void DisconnectionRequestEventHandler(StringName fromNode, long fromPort, StringName toNode, long toPort)
	{
	}

	public void ConnectionDragStartedEventHandler(string fromNode, long fromPort, bool isOutput)
	{
	}

	public void ConnectionDragEndedEventHandler()
	{
	}

	public void ConnectionRequestEventHandler(StringName fromNode, long fromPort, StringName toNode, long toPort)
	{
	}

#endregion

#region Node Management

	public void DeleteNodesRequestEventHandler(Godot.Collections.Array nodes)
	{
		foreach(var n in nodes)
		{
			selectedNodes.Remove((GraphNode)n);
			((Node)n).QueueFree();
		} 
	}

	public void PopupRequestEventHandler(Vector2 position)
	{
		var inst = nodePanel.Instantiate<PopupPanel>();
		AddChild(inst);
		inst.PopupHide += () => { inst.QueueFree(); }; // I <3 Lambdas
		inst.Popup(new Rect2i((Vector2i)position, Vector2i.Zero));
	}

#endregion	

#endregion
}
#endif