#include "block_pack.hpp"


void BlockPack::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("_set_namespace_prefix"), &BlockPack::_set_namespace_prefix);
    ClassDB::bind_method(D_METHOD("_get_namespace_prefix"), &BlockPack::_get_namespace_prefix);
    ClassDB::bind_method(D_METHOD("_set_custom_block_scripts"), &BlockPack::_set_custom_block_scripts);
    ClassDB::bind_method(D_METHOD("_get_custom_block_scripts"), &BlockPack::_get_custom_block_scripts);

    ADD_PROPERTY(PropertyInfo(Variant::STRING, "namespace_prefix"), "_set_namespace_prefix", "_get_namespace_prefix");
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "custom_block_scripts", PROPERTY_HINT_NONE, "Script"), "_set_custom_block_scripts", "_get_custom_block_scripts");
}

void BlockPack::_set_namespace_prefix(const String p_namespace_prefix)
{
    namespace_prefix = p_namespace_prefix;
}

String BlockPack::_get_namespace_prefix()
{
    return namespace_prefix;
}

void BlockPack::_set_custom_block_scripts(const Array p_custom_block_scripts)
{
    custom_block_scripts = p_custom_block_scripts;
    Array remove_queue;
    /*
    for(int i = 0; i < custom_block_scripts.size(); i++)
    {
        // strip non script elements, queued because we are currently iterating through the array
        Variant var = custom_block_scripts[i];

        if(var.get_type() == Variant::NIL) continue;
        if(var.get_type() != Variant::OBJECT)
        {
            remove_queue.append(var);
            continue;
        }
        Script *obj = Object::cast_to<Script>(var);
        if (obj == nullptr)
        {
            WARN_PRINT(vformat("Error adding %s", var));
            remove_queue.append(var);
            continue;
        }
    }
    for(int i = 0; i < remove_queue.size(); i++)
    {
        Variant element = remove_queue[i];
        custom_block_scripts.erase(element);
    }
    */
}

Array BlockPack::_get_custom_block_scripts()
{
    return custom_block_scripts;
}