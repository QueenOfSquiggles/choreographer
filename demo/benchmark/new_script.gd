extends Node

onready var c11r_test := $Node
onready var visual_script_test := $VisualScript_Test

func _ready():
	print("c11r has number %s" % str(c11r_test.debug_number))
	c11r_test.custom_func()

