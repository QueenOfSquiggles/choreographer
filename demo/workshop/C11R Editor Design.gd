extends VBoxContainer

onready var inspection_panel := $"%PanelInspection"


func _on_BtnToggleVisible_pressed() -> void:
	inspection_panel.visible = !inspection_panel.visible
