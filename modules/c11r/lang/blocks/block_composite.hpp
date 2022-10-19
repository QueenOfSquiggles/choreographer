#ifndef BLOCK_COMPOSITE_H
#define BLOCK_COMPOSITE_H

#include "../block.hpp"

class BlockComposite : public Block
{
    GDCLASS(BlockComposite, Block);

public:
    int get_output_sequence_port_count() const override;
	bool has_input_sequence_port() const override;

	String get_output_sequence_port_text(int p_port) const override;

	int get_input_value_port_count() const override;
	int get_output_value_port_count() const override;

	PropertyInfo get_input_value_port_info(int p_idx) const override;
	PropertyInfo get_output_value_port_info(int p_idx) const override;

    String get_caption() const override;
	String get_text() const;
	String get_category() const override;

    BlockInstance *instance(C11RScriptInstance *p_instance) override;

    TypeGuess guess_output_type(TypeGuess *p_inputs, int p_output) const override;

};

#endif