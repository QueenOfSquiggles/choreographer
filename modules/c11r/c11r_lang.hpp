#pragma once
#ifndef C11R_LANG_H
#define C11R_LANG_H

#include "core/os/thread.h"
#include "core/script_language.h"

class C11RScriptInstance;
class C11RScript;
class BlockInstance;
class Block;

typedef Ref<Block> (*BlockRegisterFunc)(const String &p_type);

// basically this redirects to the sub-header files. This way the individual classes can be constructed in smaller files, and speed recompiles when making small changes to a single class. Personally prefer to avoid uber-files
#include "lang/block.hpp"
#include "lang/block_instance.hpp"
#include "lang/c11r_script.hpp"
#include "lang/c11r_script_instance.hpp"
#include "lang/c11r_script_function_state.hpp"
#include "lang/c11r_script_language.hpp"

//aid for registering
template <class T>
static Ref<Block> create_node_generic(const String &p_name) {
	Ref<T> node;
	node.instance();
	return node;
}

#endif // C11R_LANG_H