extends Block

func _init() -> void:
	# this sets the custom namespace of the block. Needs to be unique to avoid breaking functionality
	block_namespace = "test.custom_block"


"""
This is the bread & butter of custom blocks. We get an array of values from previous nodes. Then we return the outputs as an array. Indices go from top to bottom so top-most input/output is index=0.  

Any amount of functionality can be performed in the function. Ideally it should be small enough that it's not creating a "black box" system, because that kind of functionality should be implemented with a Sub-Graph.

This is a resource, not a node so it isn't part of the scene tree, but the inputs can include an object reference such as scene tree or target node.
"""
func _call_block(arg_stack: Array) -> Array:
	print("debug - %s block has been called" % block_namespace)
	return []

