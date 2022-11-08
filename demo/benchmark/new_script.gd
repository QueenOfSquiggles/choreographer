extends Node

onready var child := $Node2D

func _ready():
	child.call("custom_func")
