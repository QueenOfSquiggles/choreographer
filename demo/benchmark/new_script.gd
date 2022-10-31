extends Node


var ch_script : C11RScript

func _ready():
	ch_script = C11RScript.new()
	ResourceSaver.save("res://saving_script.c11r", ch_script)
