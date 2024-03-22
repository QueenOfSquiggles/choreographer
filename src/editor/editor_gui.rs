use godot::{
    engine::{
        control::LayoutPreset, Control, HBoxContainer, IControl, Label, LabelSettings, MenuBar,
        Panel, PanelContainer, VBoxContainer,
    },
    prelude::*,
};

#[derive(GodotClass)]
#[class(tool, init, base=Control)]
pub struct C11REditorGUI {
    node: Base<Control>,
}

#[godot_api]
impl IControl for C11REditorGUI {
    fn ready(&mut self) {
        /*
        r
        - VBox
         - MenuBar
         - HBox
          - LeftPanel (.1)
          - MainScreen (.8)
          - RightPanel (.1)
         */
        let mut gui = self.create_gui();
        self.base_mut().add_child(gui.clone().upcast());
        gui.set_anchors_and_offsets_preset(LayoutPreset::FULL_RECT);
        self.base_mut()
            .set_anchors_and_offsets_preset(LayoutPreset::FULL_RECT);
    }

    fn process(&mut self, _delta: f64) {
        //pass
    }
}

impl C11REditorGUI {
    fn create_left_panel(&mut self) -> Gd<Control> {
        let mut panel = PanelContainer::new_alloc();
        let mut vbox = VBoxContainer::new_alloc();
        let mut header = Label::new_alloc();
        let mut lbl_header = LabelSettings::new_gd();
        lbl_header.set_font_size(18);
        header.set_label_settings(lbl_header);
        header.set_text("Left Panel".into());
        vbox.add_child(header.upcast());
        panel.add_child(Panel::new_alloc().upcast());
        panel.add_child(vbox.upcast());
        panel.upcast()
    }
    fn create_main_panel(&mut self) -> Gd<Control> {
        let mut panel = PanelContainer::new_alloc();
        let mut vbox = VBoxContainer::new_alloc();
        let mut header = Label::new_alloc();
        let mut lbl_header = LabelSettings::new_gd();
        lbl_header.set_font_size(18);
        header.set_label_settings(lbl_header);
        header.set_text("Center Panel".into());
        vbox.add_child(header.upcast());
        panel.add_child(Panel::new_alloc().upcast());
        panel.add_child(vbox.upcast());
        panel.upcast()
    }
    fn create_right_panel(&mut self) -> Gd<Control> {
        let mut panel = PanelContainer::new_alloc();
        let mut vbox = VBoxContainer::new_alloc();
        let mut header = Label::new_alloc();
        let mut lbl_header = LabelSettings::new_gd();

        vbox.add_child(header.clone().upcast());
        panel.add_child(Panel::new_alloc().upcast());
        panel.add_child(vbox.upcast());

        lbl_header.set_font_size(18);
        header.set_label_settings(lbl_header);
        header.set_text("Right Panel".into());

        panel.upcast()
    }
    fn create_gui(&mut self) -> Gd<Control> {
        let mut left_panel = self.create_left_panel();
        let mut main_panel = self.create_main_panel();
        let mut right_panel = self.create_right_panel();
        let mut hbox = HBoxContainer::new_alloc();
        let menu = MenuBar::new_alloc();
        let mut vbox = VBoxContainer::new_alloc();

        hbox.add_child(left_panel.clone().upcast());
        hbox.add_child(main_panel.clone().upcast());
        hbox.add_child(right_panel.clone().upcast());
        vbox.add_child(menu.upcast());
        vbox.add_child(hbox.clone().upcast());

        left_panel.set_anchors_and_offsets_preset(LayoutPreset::FULL_RECT);
        main_panel.set_anchors_and_offsets_preset(LayoutPreset::FULL_RECT);
        right_panel.set_anchors_and_offsets_preset(LayoutPreset::FULL_RECT);
        hbox.set_anchors_and_offsets_preset(LayoutPreset::FULL_RECT);
        left_panel.set_stretch_ratio(0.1);
        right_panel.set_stretch_ratio(0.1);

        vbox.upcast()
    }
}
