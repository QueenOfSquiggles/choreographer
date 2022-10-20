#ifndef BLOCK_CUSTOM_H
#define BLOCK_CUSTOM_H

#include "../block.hpp"

class BlockCustom : public Block
{
    GDCLASS(BlockCustom, Block);

public:
    int get_output_sequence_port_count() const;
	bool has_input_sequence_port() const;

	String get_output_sequence_port_text(int p_port) const;

	int get_input_value_port_count() const;
	int get_output_value_port_count() const;

	PropertyInfo get_input_value_port_info(int p_idx) const;
	PropertyInfo get_output_value_port_info(int p_idx) const;

    String get_caption() const;
	String get_text() const;
	String get_category() const;

    BlockInstance *instance(C11RScriptInstance *p_instance);

    TypeGuess guess_output_type(TypeGuess *p_inputs, int p_output) const;

	BlockCustom(){} // TODO refactor into cpp
	~BlockCustom(){}
};

#endif // BLOCK_CUSTOM_H