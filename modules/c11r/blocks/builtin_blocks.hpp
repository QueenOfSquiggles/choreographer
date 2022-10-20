#ifndef BUILTIN_BLOCK_H
#define BUILTIN_BLOCK_H

#include "block_composite.hpp"
#include "block_custom.hpp"
#include "block_sub_graph.hpp"
#include "block_entry.hpp"

_FORCE_INLINE_ static void register_builtin_blocks()
{
    // event/action/signal blocks
    // TODO implement some action blocks (functions and signal receivers)

    // boolean logic nodes
    // TODO implement basic logic nodes (and, or, nor, nand, xor, etc...)

    // flow control nodes
    // TODO implement some basic flow control (if, for, while)

    // script composition nodes
    // FIXME do these need to not be registered??
    ClassDB::register_class<BlockComposite>();
    ClassDB::register_class<BlockCustom>();
    ClassDB::register_class<BlockSubGraph>();
}

#endif // BUILTIN_BLOCK_H