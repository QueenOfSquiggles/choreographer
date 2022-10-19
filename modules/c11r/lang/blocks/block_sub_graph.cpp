#include "block_sub_graph.hpp"

// FIXME fill out stub

int BlockSubGraph::get_output_sequence_port_count() const 
{
// FIXME fill out stub
    return false;
}

bool BlockSubGraph::has_input_sequence_port() const 
{
// FIXME fill out stub
    return false;
}

String BlockSubGraph::get_output_sequence_port_text(int p_port) const 
{ 
// FIXME fill out stub
    return ""; 
}

int BlockSubGraph::get_input_value_port_count() const 
{ 
// FIXME fill out stub
    return 0; 
}

int BlockSubGraph::get_output_value_port_count() const 
{ 
// FIXME fill out stub
    return 0; 
}

PropertyInfo BlockSubGraph::get_input_value_port_info(int p_idx) const 
{
// FIXME fill out stub
    return PropertyInfo();
}

PropertyInfo BlockSubGraph::get_output_value_port_info(int p_idx) const 
{
// FIXME fill out stub
    return PropertyInfo();
}

String BlockSubGraph::get_caption() const 
{ 
// FIXME fill out stub
    return ""; 
}

String BlockSubGraph::get_text() const 
{ 
// FIXME fill out stub
    return ""; 
}

String BlockSubGraph::get_category() const {
// FIXME fill out stub
    return  "";
}

BlockInstance *BlockSubGraph::instance(C11RScriptInstance *p_instance) 
{
// FIXME fill out stub
    return nullptr;
}

Block::TypeGuess BlockSubGraph::guess_output_type(Block::TypeGuess *p_inputs, int p_output) const 
{
// FIXME fill out stub
    return TypeGuess();
}