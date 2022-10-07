namespace c11r;

using Godot;


/// Summary:
///     An interface for any event bus handler. This uses the c11r event bus system which fragments signals across individual event busses. This allows for multiple event busses to exist without needed to properly maintain any of them because the core EventBus singleton will handle processing.
public interface IEventBusHandler
{
    void ConnectEventBus(string signalName, Object targetObject, string targetMethod, params Object[] );
    void TriggerEventBusSignal(string signalName);
    bool HasEventBusSignal(string signalName);

}
