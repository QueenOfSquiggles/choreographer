namespace c11r;

using System.Collections.Generic;
using System.Diagnostics;
using Godot;

public static partial class EventBus
{
    public struct Connection {
        string signalName;
        Object targetObj;
        string targetMethod;
    };

    private static List<Connection> queuedConnections = new();
    public static Dictionary<string, IEventBusHandler> connections = new();

    public static void Connect(string signalName, Object targetObj, string targetMethod)
    {
    }

    public static void Trigger(string signalName)
    {
        // Note for any potential reader: Debug.Assert is stripped from non-debug builds. So when exported, these will not be evaluated, nor cause any issues. They exist here to ensure your systems are in compliance. It is not recommended to change this <3 
        Debug.Assert(connections.ContainsKey(signalName), $"No handler found for signal '{signalName}'");
        connections[signalName].
    }

    public static void RegisterHandler(IEventBusHandler handler){}





}