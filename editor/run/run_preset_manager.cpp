/**************************************************************************/
/*  run_preset_manager.cpp                                                */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "run_preset_manager.h"

#include "core/object/callable_mp.h"
#include "editor/inspector/editor_inspector.h"
#include "editor/run/run_preset.h"
#include "editor/settings/editor_settings.h"
#include "scene/gui/box_container.h"
#include "scene/gui/item_list.h"
#include "scene/gui/split_container.h"
#include "scene/scene_string_names.h"

void RunPresetManagerDialog::_on_add_pressed() {
	presets.push_back(RunPreset::from_dict(new_preset));

	_update_preset_list();
}

void RunPresetManagerDialog::_on_remove_pressed() {
	PackedInt32Array selected_items = presets_list->get_selected_items();
	if (selected_items.is_empty()) {
		return;
	}

	int selected_index = selected_items[0];
	presets.remove_at(selected_index);

	_update_preset_list();
}

void RunPresetManagerDialog::_on_restore_pressed() {
	for (Dictionary preset : default_presets) {
		presets.append(RunPreset::from_dict(preset));
	}

	_update_preset_list();
}

void RunPresetManagerDialog::_on_preset_selected(int p_index) {
	Ref<RunPreset> preset = presets[p_index];
	preset_inspector->edit(preset.ptr());
	preset_inspector->show();
	no_presets_selected_label->hide();
}

void RunPresetManagerDialog::_on_preset_property_edited() {
	int presets_index = presets_list->get_selected_items()[0];
	presets_list->set_item_text(presets_index, presets[presets_index]->get_name());
	presets_list->set_item_icon(presets_index, presets[presets_index]->get_icon());
	save_presets();
}

void RunPresetManagerDialog::_update_preset_list() {
	Ref<RunPreset> selected_preset = (Ref<RunPreset>)preset_inspector->get_edited_object();
	presets_list->clear();
	for (Ref<RunPreset> &preset : presets) {
		presets_list->add_item(preset->get_name(), preset->get_icon());
		if (preset == selected_preset) {
			presets_list->select(presets_list->get_item_count() - 1);
		}
	}
	if (!presets_list->is_anything_selected()) {
		preset_inspector->hide();
		no_presets_selected_label->show();
	}
	save_presets();
	emit_signal("presets_changed");
}

void RunPresetManagerDialog::save_presets() {
	Array presets_array;
	for (Ref<RunPreset> preset : presets) {
		presets_array.append(preset->to_dict());
	};
	EditorSettings::get_singleton()->set_project_metadata("editor_run_bar", "presets", presets_array);
}

void RunPresetManagerDialog::_bind_methods() {
	ADD_SIGNAL(MethodInfo("presets_changed"));
}

RunPresetManagerDialog::RunPresetManagerDialog() {
	HSplitContainer *split = memnew(HSplitContainer);
	add_child(split);

	VBoxContainer *vbox = memnew(VBoxContainer);
	split->add_child(vbox);

	presets_list = memnew(ItemList);
	vbox->add_child(presets_list);
	presets_list->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	// TODO : reprendre ca pour faire en sorte que ca soit call qu'une fois par frame
	presets_list->connect(SceneStringName(item_selected), callable_mp(this, &RunPresetManagerDialog::_on_preset_selected));

	HBoxContainer *button_bar = memnew(HBoxContainer);
	vbox->add_child(button_bar);

	Button *add_button = memnew(Button);
	add_button->set_text("Add");
	add_button->connect(SceneStringName(pressed), callable_mp(this, &RunPresetManagerDialog::_on_add_pressed));
	button_bar->add_child(add_button);

	Button *restore_button = memnew(Button);
	restore_button->set_text("Restore defaults");
	restore_button->connect(SceneStringName(pressed), callable_mp(this, &RunPresetManagerDialog::_on_restore_pressed));
	button_bar->add_child(restore_button);

	Button *remove_button = memnew(Button);
	remove_button->set_text("Remove");
	remove_button->connect(SceneStringName(pressed), callable_mp(this, &RunPresetManagerDialog::_on_remove_pressed));
	button_bar->add_child(remove_button);

	preset_inspector = memnew(EditorInspector);
	preset_inspector->connect(SNAME("property_edited"), callable_mp(this, &RunPresetManagerDialog::_on_preset_property_edited).unbind(1));
	split->add_child(preset_inspector);

	new_preset["name"] = TTR("New Preset");
	new_preset["editor_icon"] = SNAME("Play");
	new_preset["use_current_mode"] = true;
	new_preset["use_current_destination"] = true;
	new_preset["run_xr_enabled_use_current"] = true;

	Dictionary current_scene_preset;
	current_scene_preset["name"] = TTR("Run Current Scene");
	current_scene_preset["mode"] = RunMode::RUN_CURRENT;
	current_scene_preset["use_current_destination"] = true;
	current_scene_preset["show_toolbar_use_current"] = true;
	current_scene_preset["run_xr_enabled_use_current"] = true;
	current_scene_preset["pinned"] = true;
	current_scene_preset["editor_icon"] = SNAME("PlayScene");
	default_presets.push_back(current_scene_preset);

	Dictionary remote_run_preset;
	remote_run_preset["name"] = TTR("Remote Deploy");
	remote_run_preset["mode"] = RunMode::RUN_MAIN;
	remote_run_preset["destination"] = DESTINATION_REMOTE;
	remote_run_preset["select_remote_platform_id"] = true;
	remote_run_preset["select_remote_device_id"] = true;
	remote_run_preset["show_toolbar"] = false;
	remote_run_preset["run_xr_enabled_use_current"] = true;
	remote_run_preset["pinned"] = true;
	remote_run_preset["editor_icon"] = SNAME("PlayRemote");
	default_presets.push_back(remote_run_preset);

	Dictionary main_scene_preset;
	main_scene_preset["name"] = TTR("Run Custom Scene");
	main_scene_preset["mode"] = RunMode::RUN_CUSTOM;
	main_scene_preset["use_current_destination"] = true;
	main_scene_preset["show_toolbar_use_current"] = true;
	main_scene_preset["run_xr_enabled_use_current"] = true;
	main_scene_preset["pinned"] = false;
	main_scene_preset["editor_icon"] = SNAME("PlayCustom");
	default_presets.push_back(main_scene_preset);

	TypedArray<Dictionary> presets_data = EditorSettings::get_singleton()->get_project_metadata("editor_run_bar", "presets", default_presets);
	for (Dictionary preset_data : presets_data) {
		presets.push_back(RunPreset::from_dict(preset_data));
	}

	no_presets_selected_label = memnew(Label);
	no_presets_selected_label->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	no_presets_selected_label->set_text(TTR("No preset selected! Please select a preset to edit it here."));
	split->add_child(no_presets_selected_label);

	_update_preset_list();
}
