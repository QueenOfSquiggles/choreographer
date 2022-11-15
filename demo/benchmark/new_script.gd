extends Node

onready var c11r_test := $Node
onready var visual_script_test := $VisualScript_Test

func _ready():
	var sc = c11r_test.get_script()
	print("C11R Script: %s" % str(sc))
	
	sc = visual_script_test.get_script()
	print("VS Script: %s" % str(sc))
	
	
#	c11r_test.custom_func()
	

