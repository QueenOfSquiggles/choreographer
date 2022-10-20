#ifndef C11R_SCRIPT_FUNCTION_STATE_H
#define C11R_SCRIPT_FUNCTION_STATE_H

#include "../c11r_lang.hpp"
#include "block_instance.hpp"
#include "c11r_script_instance.hpp"

class C11RScriptFunctionState : public Reference {
	GDCLASS(C11RScriptFunctionState, Reference);
	friend class C11RScriptInstance;

	ObjectID instance_id;
	ObjectID script_id;
	C11RScriptInstance *instance;
	StringName function;
	Vector<uint8_t> stack;
	int working_mem_index;
	int variant_stack_size;
	BlockInstance *node;
	int flow_stack_pos;
	int pass;

	Variant _signal_callback(const Variant **p_args, int p_argcount, Variant::CallError &r_error);

protected:
	static void _bind_methods();

public:
	void connect_to_signal(Object *p_obj, const String &p_signal, Array p_binds);
	bool is_valid() const;
	Variant resume(Array p_args);
	C11RScriptFunctionState();
	~C11RScriptFunctionState();
};

#endif // C11R_SCRIPT_FUNCTION_STATE_H