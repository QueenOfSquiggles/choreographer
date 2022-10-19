#ifndef BLOCK_PACK_H
#define BLOCK_PACK_H

#include "core/resource.h"
#include "../c11r_lang.hpp"


class BlockPack : public Resource
{
    GDCLASS(BlockPack, Resource);


    void _set_namespace_prefix(const String p_namespace_prefix);
    String _get_namespace_prefix();

    void _set_custom_block_scripts(const Array p_custom_block_scripts);
    Array _get_custom_block_scripts();

protected:
    static void _bind_methods();

public:
    String namespace_prefix;
    Array custom_block_scripts;

};



#endif // BLOCK_PACK_H