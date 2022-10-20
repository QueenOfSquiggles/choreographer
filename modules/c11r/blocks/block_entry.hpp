#ifndef BLOCK_ENTRY_H
#define BLOCK_ENTRY_H

#include "../c11r_lang.hpp"

class BlockEntryInstance;

class BlockEntry : public Block
{
    GDCLASS(BlockEntry, Block);

    friend class BlockInstance;
    friend class BlockEntryInstance;
public:
    int get_output_sequence_port_count() const { return 1; }
	bool has_input_sequence_port() const { return false; }

	String get_output_sequence_port_text(int p_port) const { return "Event"; }

	int get_input_value_port_count() const { return 0; }
	int get_output_value_port_count() const { return 0; }

	PropertyInfo get_input_value_port_info(int p_idx) const { return PropertyInfo(); }
	PropertyInfo get_output_value_port_info(int p_idx) const { return PropertyInfo(); }

    String get_caption() const { return "" ;}
	String get_text() const  { return "" ;}
	String get_category() const  { return "" ;}

    BlockInstance *instance(C11RScriptInstance *p_instance);

    TypeGuess guess_output_type(TypeGuess *p_inputs, int p_output) const { return TypeGuess(); }

	BlockEntry(){}
	~BlockEntry(){}
};


#endif // BLOCK_ENTRY_H