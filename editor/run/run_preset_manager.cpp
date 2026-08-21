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
#include "editor/editor_string_names.h"
#include "editor/inspector/editor_inspector.h"
#include "editor/run/run_preset.h"
#include "editor/settings/editor_settings.h"
#include "editor/themes/editor_scale.h"
#include "scene/gui/box_container.h"
#include "scene/gui/item_list.h"
#include "scene/gui/split_container.h"
#include "scene/scene_string_names.h"

void RunPresetManagerDialog::_on_add_pressed() {
	presets.push_back(new_preset->duplicate());

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
	for (Ref<RunPreset> preset : default_presets) {
		presets.append(preset->duplicate());
	}

	_update_preset_list();
}

void RunPresetManagerDialog::_on_preset_selected(int p_index) {
	presets_list->select(p_index);
	preset_inspector->edit(presets[p_index].ptr());
	preset_inspector->show();
	info_message_vbox->hide();
}

void RunPresetManagerDialog::_on_preset_property_edited() {
	int presets_index = presets_list->get_selected_items()[0];
	presets_list->set_item_text(presets_index, presets[presets_index]->get_preset_name());
	presets_list->set_item_icon(presets_index, presets[presets_index]->get_icon());
	save_presets();
}

void RunPresetManagerDialog::_update_preset_list() {
	Ref<RunPreset> selected_preset = (Ref<RunPreset>)preset_inspector->get_edited_object();
	int selected_index = 0;
	if (presets_list->is_anything_selected()) {
		selected_index = presets_list->get_selected_items()[0];
	}
	presets_list->clear();
	for (Ref<RunPreset> &preset : presets) {
		presets_list->add_item(preset->get_preset_name(), preset->get_icon());
		if (preset == selected_preset) {
			presets_list->select(presets_list->get_item_count() - 1);
		}
	}
	if (!presets_list->is_anything_selected()) {
		if (selected_index < presets.size()) {
			_on_preset_selected(selected_index);
		} else if (presets.size() > 0) {
			_on_preset_selected(presets.size() - 1);
		} else {
			preset_inspector->edit(nullptr);
			preset_inspector->hide();
			info_message_vbox->show();
		}
	}
	save_presets();
	emit_signal("presets_changed");
}

void RunPresetManagerDialog::save_presets() {
	Array presets_array;
	for (Ref<RunPreset> preset : presets) {
		presets_array.append(preset);
	};
	EditorSettings::get_singleton()->set_project_metadata("editor_run_bar", "presets", presets_array);
}

void RunPresetManagerDialog::_bind_methods() {
	ADD_SIGNAL(MethodInfo("presets_changed"));
}

void RunPresetManagerDialog::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_THEME_CHANGED: {
			_update_preset_list();
			break;
		}
		case NOTIFICATION_ENTER_TREE: {
			get_parent()->connect(SceneStringName(theme_changed), callable_mp(this, &RunPresetManagerDialog::_on_parent_theme_changed));
			_on_parent_theme_changed();
		} break;
	}
}

void RunPresetManagerDialog::_on_parent_theme_changed() {
	presets_list->set_fixed_icon_size(get_theme_constant("class_icon_size", EditorStringName(Editor)) * Vector2(1, 1));
}

RunPresetManagerDialog::RunPresetManagerDialog() {
	HSplitContainer *split = memnew(HSplitContainer);
	split->set_custom_minimum_size(Size2(500, 240) * EDSCALE);
	add_child(split);

	VBoxContainer *vbox = memnew(VBoxContainer);
	split->add_child(vbox);

	presets_list = memnew(ItemList);
	vbox->add_child(presets_list);
	presets_list->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	presets_list->set_theme_type_variation("ItemListSecondary");
	presets_list->connect(SceneStringName(item_selected), callable_mp(this, &RunPresetManagerDialog::_on_preset_selected));

	HBoxContainer *button_bar = memnew(HBoxContainer);
	vbox->add_child(button_bar);

	Button *add_button = memnew(Button);
	add_button->set_text("Add");
	add_button->connect(SceneStringName(pressed), callable_mp(this, &RunPresetManagerDialog::_on_add_pressed));
	button_bar->add_child(add_button);

	Button *restore_button = memnew(Button);
	restore_button->set_text("Restore Defaults");
	restore_button->connect(SceneStringName(pressed), callable_mp(this, &RunPresetManagerDialog::_on_restore_pressed));
	button_bar->add_child(restore_button);

	Button *remove_button = memnew(Button);
	remove_button->set_text("Remove");
	remove_button->connect(SceneStringName(pressed), callable_mp(this, &RunPresetManagerDialog::_on_remove_pressed));
	button_bar->add_child(remove_button);

	preset_inspector = memnew(EditorInspector);
	preset_inspector->connect(SNAME("property_edited"), callable_mp(this, &RunPresetManagerDialog::_on_preset_property_edited).unbind(1));
	preset_inspector->set_theme_type_variation("TreeSecondary");
	split->add_child(preset_inspector);

	info_message_vbox = memnew(VBoxContainer);
	split->add_child(info_message_vbox);
	info_message_vbox->set_alignment(BoxContainer::AlignmentMode::ALIGNMENT_CENTER);
	info_message_vbox->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	info_message_vbox->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	info_message_vbox->hide();

	Label *info_message = memnew(Label);
	info_message_vbox->add_child(info_message);
	info_message->set_focus_mode(Control::FOCUS_ACCESSIBILITY);
	info_message->set_text(TTRC("Add a preset to edit it."));
	info_message->set_vertical_alignment(VERTICAL_ALIGNMENT_CENTER);
	info_message->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
	info_message->set_autowrap_mode(TextServer::AUTOWRAP_WORD_SMART);
	info_message->set_custom_minimum_size(Size2(100 * EDSCALE, 0));
	info_message->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT, Control::PRESET_MODE_KEEP_SIZE, 8 * EDSCALE);

	Button *add_animation_player = memnew(Button);
	info_message_vbox->add_child(add_animation_player);
	add_animation_player->set_text(TTRC("Add Preset"));
	add_animation_player->set_tooltip_text(TTRC("Add a new preset."));
	add_animation_player->set_h_size_flags(Control::SIZE_SHRINK_CENTER);
	add_animation_player->connect(SceneStringName(pressed), callable_mp(this, &RunPresetManagerDialog::_on_add_pressed));

	new_preset = memnew(RunPreset);
	new_preset->set_preset_name(TTR("New Preset"));
	new_preset->set_editor_icon(SNAME("Play"));
	new_preset->set_use_current_mode(true);
	new_preset->set_use_current_destination(true);
	new_preset->set_run_xr_enabled_use_current(true);

	Ref<RunPreset> current_scene_preset = memnew(RunPreset);
	current_scene_preset->set_preset_name(TTR("Run Current Scene"));
	current_scene_preset->set_mode(RunMode::RUN_CURRENT);
	current_scene_preset->set_use_current_destination(true);
	current_scene_preset->set_show_toolbar_use_current(true);
	current_scene_preset->set_run_xr_enabled_use_current(true);
	current_scene_preset->set_pinned(true);
	current_scene_preset->set_movie_maker_enabled_use_current(true);
	current_scene_preset->set_editor_icon(SNAME("PlayScene"));
	default_presets.push_back(current_scene_preset);

	Ref<RunPreset> remote_run_preset = memnew(RunPreset);
	remote_run_preset->set_preset_name(TTR("Remote Deploy"));
	remote_run_preset->set_mode(RunMode::RUN_MAIN);
	remote_run_preset->set_destination(DESTINATION_REMOTE);
	remote_run_preset->set_select_remote_platform_id(true);
	remote_run_preset->set_select_remote_device_id(true);
	remote_run_preset->set_show_toolbar(false);
	remote_run_preset->set_run_xr_enabled_use_current(true);
	remote_run_preset->set_pinned(true);
	remote_run_preset->set_movie_maker_enabled_use_current(true);
	remote_run_preset->set_editor_icon(SNAME("PlayRemote"));
	default_presets.push_back(remote_run_preset);

	Ref<RunPreset> main_scene_preset = memnew(RunPreset);
	main_scene_preset->set_preset_name(TTR("Run Custom Scene"));
	main_scene_preset->set_mode(RunMode::RUN_CUSTOM);
	main_scene_preset->set_use_current_destination(true);
	main_scene_preset->set_show_toolbar_use_current(true);
	main_scene_preset->set_run_xr_enabled_use_current(true);
	main_scene_preset->set_pinned(false);
	main_scene_preset->set_movie_maker_enabled_use_current(true);
	main_scene_preset->set_editor_icon(SNAME("PlayCustom"));
	default_presets.push_back(main_scene_preset);

	Array presets_data = EditorSettings::get_singleton()->get_project_metadata("editor_run_bar", "presets", default_presets);
	for (Ref<RunPreset> preset_data : presets_data) {
		presets.push_back(preset_data);
	}
	_update_preset_list();
}
