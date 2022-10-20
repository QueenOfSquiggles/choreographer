#include "block_entry.hpp"

class BlockEntryInstance : public BlockInstance
{

public:
	BlockEntry *block;
	C11RScriptInstance *instance;

	//virtual int get_working_memory_size() const { return 0; }

	virtual int step(const Variant **p_inputs, Variant **p_outputs, StartMode p_start_mode, Variant *p_working_mem, Variant::CallError &r_error, String &r_error_str) {
        //TODO implement entry logic
		return 0;
	}

};

BlockInstance *BlockEntry::instance(C11RScriptInstance *p_instance) {
    BlockEntryInstance* instance = memnew(BlockEntryInstance);
    instance->block = this;
    instance->instance = p_instance;
    return instance;
}