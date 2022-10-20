#ifndef BLOCK_INSTANCE_H
#define BLOCK_INSTANCE_H

#include "../c11r_lang.hpp"
#include "block.hpp"

class BlockInstance {
	friend class C11RScriptInstance;
	friend class C11RScriptLanguage; //for debugger

	enum { //input argument addressing
		INPUT_SHIFT = 1 << 24,
		INPUT_MASK = INPUT_SHIFT - 1,
		INPUT_DEFAULT_VALUE_BIT = INPUT_SHIFT, // from unassigned input port, using default value (edited by user)
	};

	int id;
	int sequence_index;
	BlockInstance **sequence_outputs;
	int sequence_output_count;
	Vector<BlockInstance *> dependencies;
	int *input_ports;
	int input_port_count;
	int *output_ports;
	int output_port_count;
	int working_mem_idx;
	int pass_idx;

	Block *base;

public:
	enum StartMode {
		START_MODE_BEGIN_SEQUENCE,
		START_MODE_CONTINUE_SEQUENCE,
		START_MODE_RESUME_YIELD
	};

	enum {
		STEP_SHIFT = 1 << 24,
		STEP_MASK = STEP_SHIFT - 1,
		STEP_FLAG_PUSH_STACK_BIT = STEP_SHIFT, //push bit to stack
		STEP_FLAG_GO_BACK_BIT = STEP_SHIFT << 1, //go back to previous node
		STEP_NO_ADVANCE_BIT = STEP_SHIFT << 2, //do not advance past this node
		STEP_EXIT_FUNCTION_BIT = STEP_SHIFT << 3, //return from function
		STEP_YIELD_BIT = STEP_SHIFT << 4, //yield (will find C11RScriptFunctionState state in first working memory)

		FLOW_STACK_PUSHED_BIT = 1 << 30, //in flow stack, means bit was pushed (must go back here if end of sequence)
		FLOW_STACK_MASK = FLOW_STACK_PUSHED_BIT - 1

	};

	_FORCE_INLINE_ int get_input_port_count() const { return input_port_count; }
	_FORCE_INLINE_ int get_output_port_count() const { return output_port_count; }
	_FORCE_INLINE_ int get_sequence_output_count() const { return sequence_output_count; }

	_FORCE_INLINE_ int get_id() const { return id; }

	virtual int get_working_memory_size() const { return 0; }

	virtual int step(const Variant **p_inputs, Variant **p_outputs, StartMode p_start_mode, Variant *p_working_mem, Variant::CallError &r_error, String &r_error_str) = 0; //do a step, return which sequence port to go out

	Ref<Block> get_base_node() { return Ref<Block>(base); }

	BlockInstance();
	virtual ~BlockInstance();
};
#endif // BLOCK_INSTANCE_H