#include "block_custom.hpp"

// FIXME fill out stub

int BlockCustom::get_output_sequence_port_count() const 
{
// FIXME fill out stub
    return false;
}

bool BlockCustom::has_input_sequence_port() const 
{
// FIXME fill out stub
    return false;
}

String BlockCustom::get_output_sequence_port_text(int p_port) const 
{ 
// FIXME fill out stub
    return ""; 
}

int BlockCustom::get_input_value_port_count() const 
{ 
// FIXME fill out stub
    return 0; 
}

int BlockCustom::get_output_value_port_count() const 
{ 
// FIXME fill out stub
    return 0; 
}

PropertyInfo BlockCustom::get_input_value_port_info(int p_idx) const 
{
// FIXME fill out stub
    return PropertyInfo();
}

PropertyInfo BlockCustom::get_output_value_port_info(int p_idx) const 
{
// FIXME fill out stub
    return PropertyInfo();
}

String BlockCustom::get_caption() const 
{ 
// FIXME fill out stub
    return ""; 
}

String BlockCustom::get_text() const 
{ 
// FIXME fill out stub
    return ""; 
}

String BlockCustom::get_category() const {
// FIXME fill out stub
    return  "";
}

BlockInstance *BlockCustom::instance(C11RScriptInstance *p_instance) 
{
// FIXME fill out stub
    return nullptr;
}

TypeGuess BlockCustom::guess_output_type(TypeGuess *p_inputs, int p_output) const 
{
// FIXME fill out stub
    return TypeGuess();
}