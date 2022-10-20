#include "block_composite.hpp"

// FIXME fill out stub

int BlockComposite::get_output_sequence_port_count() const 
{
// FIXME fill out stub
    return false;
}

bool BlockComposite::has_input_sequence_port() const 
{
// FIXME fill out stub
    return false;
}

String BlockComposite::get_output_sequence_port_text(int p_port) const 
{ 
// FIXME fill out stub
    return ""; 
}

int BlockComposite::get_input_value_port_count() const 
{ 
// FIXME fill out stub
    return 0; 
}

int BlockComposite::get_output_value_port_count() const 
{ 
// FIXME fill out stub
    return 0; 
}

PropertyInfo BlockComposite::get_input_value_port_info(int p_idx) const 
{
// FIXME fill out stub
    return PropertyInfo();
}

PropertyInfo BlockComposite::get_output_value_port_info(int p_idx) const 
{
// FIXME fill out stub
    return PropertyInfo();
}

String BlockComposite::get_caption() const 
{ 
// FIXME fill out stub
    return ""; 
}

String BlockComposite::get_text() const 
{ 
// FIXME fill out stub
    return ""; 
}

String BlockComposite::get_category() const {
// FIXME fill out stub
    return  "";
}

BlockInstance *BlockComposite::instance(C11RScriptInstance *p_instance) 
{
// FIXME fill out stub
    return nullptr;
}

Block::TypeGuess BlockComposite::guess_output_type(TypeGuess *p_inputs, int p_output) const 
{
// FIXME fill out stub
    return TypeGuess();
}