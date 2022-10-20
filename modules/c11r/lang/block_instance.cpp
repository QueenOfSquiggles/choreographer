#include "block_instance.hpp"

#include "core/core_string_names.h"
#include "core/os/os.h"
#include "core/project_settings.h"
#include "scene/main/node.h"
#include "modules/visual_script/visual_script_func_nodes.h"
#include "modules/visual_script/visual_script_nodes.h"

BlockInstance::BlockInstance() {
	sequence_outputs = nullptr;
	input_ports = nullptr;
}

BlockInstance::~BlockInstance() {
	if (sequence_outputs) {
		memdelete_arr(sequence_outputs);
	}

	if (input_ports) {
		memdelete_arr(input_ports);
	}

	if (output_ports) {
		memdelete_arr(output_ports);
	}
}
