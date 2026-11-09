/**************************************************************************/
/*  run_preset_manager_dialog.cpp                                         */
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

#include "run_preset_manager_dialog.h"

#include "core/object/callable_mp.h"
#include "core/object/class_db.h"
#include "editor/editor_node.h"
#include "editor/editor_string_names.h"
#include "editor/editor_undo_redo_manager.h"
#include "editor/gui/editor_icon_manager.h"
#include "editor/inspector/editor_inspector.h"
#include "editor/run/run_preset.h"
#include "editor/settings/editor_command_palette.h"
#include "editor/settings/editor_settings.h"
#include "editor/themes/editor_scale.h"
#include "scene/gui/box_container.h"
#include "scene/gui/popup_menu.h"
#include "scene/gui/split_container.h"
#include "scene/scene_string_names.h"

void RunPresetManagerDialog::_on_add_pressed() {
	EditorUndoRedoManager *undo_redo = EditorUndoRedoManager::get_singleton();
	undo_redo->create_action(TTR("Create New Preset"));
	undo_redo->add_do_method(this, "_add_preset", new_preset);
	undo_redo->add_undo_method(this, "_remove_preset", presets.size());
	undo_redo->commit_action();
}

void RunPresetManagerDialog::_on_restore_pressed() {
	EditorUndoRedoManager *undo_redo = EditorUndoRedoManager::get_singleton();
	undo_redo->create_action(TTR("Restore Default Presets"));
	for (Ref<RunPreset> preset : default_presets) {
		undo_redo->add_do_method(this, "_add_preset", preset);
		undo_redo->add_undo_method(this, "_remove_preset", presets.size());
	}

	undo_redo->commit_action();
	_update_preset_list();
}

void RunPresetManagerDialog::_add_preset(Ref<RunPreset> p_preset, int p_index) {
	p_preset = p_preset->duplicate(true);
	if (p_index == -1) {
		presets.push_back(p_preset);
	} else {
		presets.insert(p_index, p_preset);
	}
	p_preset->activate_editor_shortcut();
	_update_preset_list();
}

void RunPresetManagerDialog::_remove_preset(int p_index) {
	presets[p_index]->deactivate_editor_shortcut();
	presets.remove_at(p_index);
	_update_preset_list();
}

void RunPresetManagerDialog::_on_preset_selected(int p_index) {
	presets_list->select(p_index);
	preset_inspector->edit(presets[p_index].ptr());
	preset_inspector->show();
	info_message_vbox->hide();
}

void RunPresetManagerDialog::_on_preset_clicked(int p_index, const Vector2 &p_local_mouse_pos, int p_button) {
	if (p_button != int(MouseButton::RIGHT)) {
		return;
	}

	preset_context_menu->set_position(get_mouse_position() + get_position());

	preset_index_pressed = p_index;
	preset_context_menu->popup();
}

void RunPresetManagerDialog::_on_preset_property_edited() {
	int presets_index = presets_list->get_selected_items()[0];
	presets_list->set_item_text(presets_index, presets[presets_index]->get_custom_preset_name());
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
		presets_list->add_item(preset->get_custom_preset_name(), preset->get_icon());
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

void RunPresetManagerDialog::_on_preset_context_menu_item_pressed(int p_id) {
	EditorUndoRedoManager *undo_redo = EditorUndoRedoManager::get_singleton();
	switch (p_id) {
		case PRESET_CONTEXT_MENU_DUPLICATE: {
			undo_redo->create_action(TTR("Duplicate Preset"));
			undo_redo->add_do_method(this, "_add_preset", presets[preset_index_pressed]);
			undo_redo->add_undo_method(this, "_remove_preset", preset_index_pressed + 1);
			undo_redo->commit_action();
			_update_preset_list();
		} break;
		case PRESET_CONTEXT_MENU_DELETE: {
			undo_redo->create_action(TTR("Delete Preset"));
			undo_redo->add_do_method(this, "_remove_preset", preset_index_pressed);
			undo_redo->add_undo_method(this, "_add_preset", presets[preset_index_pressed], preset_index_pressed);
			undo_redo->commit_action();
			_update_preset_list();
		} break;
	}
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

	ClassDB::bind_method(D_METHOD("_move_preset", "from_index", "to_index"), &RunPresetManagerDialog::_move_preset);
	ClassDB::bind_method(D_METHOD("_add_preset", "preset", "index"), &RunPresetManagerDialog::_add_preset, DEFVAL(-1));
	ClassDB::bind_method(D_METHOD("_remove_preset", "index"), &RunPresetManagerDialog::_remove_preset);
}

void RunPresetManagerDialog::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			get_parent()->connect(SceneStringName(theme_changed), callable_mp(this, &RunPresetManagerDialog::_on_parent_theme_changed));

			_on_parent_theme_changed();
		} break;

		case NOTIFICATION_DRAG_END: {
			if (!gui_is_drag_successful()) {
				current_drop_index = -1;
				set_process_internal(false);
				_update_preset_list();
			}
		} break;
		case NOTIFICATION_INTERNAL_PROCESS: {
			int new_drop_index = presets_list->get_item_at_position(presets_list->get_local_mouse_position());
			if (new_drop_index != current_drop_index) {
				presets_list->move_item(current_drop_index, new_drop_index);
				current_drop_index = new_drop_index;
			}
		} break;
		case NOTIFICATION_VISIBILITY_CHANGED: {
			set_process_shortcut_input(is_visible());
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

	Label *presets_label = memnew(Label);
	vbox->add_child(presets_label);
	presets_label->set_text(TTR("Presets:"));

	presets_list = memnew(ItemList);
	vbox->add_child(presets_list);
	presets_list->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	presets_list->set_theme_type_variation("ItemListSecondary");
	presets_list->connect(SceneStringName(item_selected), callable_mp(this, &RunPresetManagerDialog::_on_preset_selected));
	presets_list->connect(SNAME("item_clicked"), callable_mp(this, &RunPresetManagerDialog::_on_preset_clicked));
	SET_DRAG_FORWARDING_GCD(presets_list, RunPresetManagerDialog);

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
	new_preset->set_editor_icon(SNAME("Play"));
	new_preset->set_use_current_mode(true);
	new_preset->set_use_current_destination(true);
	new_preset->set_run_xr_enabled_use_current(true);

	main_scene_preset = memnew(RunPreset);
	main_scene_preset->set_custom_preset_name(TTR("Run Main Scene"));
	main_scene_preset->set_mode(RunMode::RUN_MAIN);
	main_scene_preset->set_use_current_mode(false);
	main_scene_preset->set_show_toolbar_use_current(true);
	main_scene_preset->set_run_xr_enabled_use_current(true);
	main_scene_preset->set_pinned(true);
	main_scene_preset->set_movie_maker_enabled_use_current(true);
	main_scene_preset->set_editor_icon(SNAME("MainPlay"));
	default_presets.push_back(main_scene_preset);

	current_scene_preset = memnew(RunPreset);
	current_scene_preset->set_custom_preset_name(TTR("Run Current Scene"));
	current_scene_preset->set_mode(RunMode::RUN_CURRENT);
	current_scene_preset->set_use_current_mode(false);
	current_scene_preset->set_show_toolbar_use_current(true);
	current_scene_preset->set_run_xr_enabled_use_current(true);
	current_scene_preset->set_pinned(true);
	current_scene_preset->set_movie_maker_enabled_use_current(true);
	current_scene_preset->set_editor_icon(SNAME("PlayScene"));
	current_scene_preset->set_shortcut(ED_SHORTCUT_AND_COMMAND("editor/run_current_scene", TTRC("Run Current Scene"), Key::F6));
	ED_SHORTCUT_OVERRIDE("editor/run_current_scene", "macos", KeyModifierMask::META | Key::R);
	default_presets.push_back(current_scene_preset);

	remote_run_preset = memnew(RunPreset);
	remote_run_preset->set_custom_preset_name(TTR("Remote Deploy"));
	remote_run_preset->set_mode(RunMode::RUN_MAIN);
	remote_run_preset->set_use_current_mode(false);
	remote_run_preset->set_destination(DESTINATION_REMOTE);
	remote_run_preset->set_use_current_destination(false);
	remote_run_preset->set_select_remote_platform_id(true);
	remote_run_preset->set_select_remote_device_id(true);
	remote_run_preset->set_show_toolbar(false);
	remote_run_preset->set_run_xr_enabled_use_current(true);
	remote_run_preset->set_pinned(true);
	remote_run_preset->set_movie_maker_enabled_use_current(true);
	remote_run_preset->set_editor_icon(SNAME("PlayRemote"));
	default_presets.push_back(remote_run_preset);

	custom_scene_preset = memnew(RunPreset);
	custom_scene_preset->set_custom_preset_name(TTR("Run Custom Scene"));
	custom_scene_preset->set_mode(RunMode::RUN_CUSTOM);
	custom_scene_preset->set_use_current_mode(false);
	custom_scene_preset->set_show_toolbar_use_current(true);
	custom_scene_preset->set_run_xr_enabled_use_current(true);
	custom_scene_preset->set_pinned(false);
	custom_scene_preset->set_movie_maker_enabled_use_current(true);
	custom_scene_preset->set_editor_icon(SNAME("PlayCustom"));
	custom_scene_preset->set_shortcut(ED_SHORTCUT_AND_COMMAND("editor/run_specific_scene", TTRC("Run Specific Scene"), KeyModifierMask::CTRL | KeyModifierMask::SHIFT | Key::F5));
	ED_SHORTCUT_OVERRIDE("editor/run_specific_scene", "macos", KeyModifierMask::META | KeyModifierMask::SHIFT | Key::R);
	default_presets.push_back(custom_scene_preset);

	Array presets_data = EditorSettings::get_singleton()->get_project_metadata("editor_run_bar", "presets", default_presets);
	for (Ref<RunPreset> preset_data : presets_data) {
		presets.push_back(preset_data);
	}

	preset_context_menu = memnew(PopupMenu);
	preset_context_menu->add_icon_item(EditorIconManager::get_icon(SNAME("Duplicate")), TTR("Duplicate"), PRESET_CONTEXT_MENU_DUPLICATE);
	preset_context_menu->add_icon_item(EditorIconManager::get_icon(SNAME("Remove")), TTR("Delete"), PRESET_CONTEXT_MENU_DELETE);
	preset_context_menu->connect(SceneStringName(id_pressed), callable_mp(this, &RunPresetManagerDialog::_on_preset_context_menu_item_pressed));

	add_child(preset_context_menu);
	_update_preset_list();
}

Variant RunPresetManagerDialog::get_drag_data_fw(const Point2 &p_point, Control *p_from) {
	if (p_from != presets_list) {
		return Variant();
	}
	int index = presets_list->get_item_at_position(p_point);
	if (index < 0) {
		return Variant();
	}

	Dictionary drag_data;
	drag_data["type"] = "run_preset_manager";
	drag_data["index"] = index;
	presets_list->select(index);
	set_process_internal(true);
	current_drop_index = index;
	return drag_data;
}

bool RunPresetManagerDialog::can_drop_data_fw(const Point2 &p_point, const Variant &p_data, Control *p_from) const {
	Dictionary d = p_data;
	if (!d.has("type") || String(d["type"]) != "run_preset_manager") {
		return false;
	}
	return true;
}

void RunPresetManagerDialog::drop_data_fw(const Point2 &p_point, const Variant &p_data, Control *p_from) {
	Dictionary drag_data = p_data;
	int from_index = drag_data["index"];
	int to_index = presets_list->get_item_at_position(p_point);
	EditorUndoRedoManager *undo_redo = EditorUndoRedoManager::get_singleton();
	undo_redo->create_action(TTR("Move Preset"));
	undo_redo->add_do_method(this, "_move_preset", from_index, to_index);
	undo_redo->add_undo_method(this, "_move_preset", to_index, from_index);
	undo_redo->commit_action();
	current_drop_index = -1;
	set_process_internal(false);
}

void RunPresetManagerDialog::_move_preset(int p_from_index, int p_to_index) {
	Ref<RunPreset> preset = presets[p_from_index];
	presets.remove_at(p_from_index);
	presets.insert(p_to_index, preset); // to update with the right thing as there is off by   error depending on whether the move is up or down
	_update_preset_list();
}

void RunPresetManagerDialog::shortcut_input(const Ref<InputEvent> &p_event) {
	const Ref<InputEventKey> k = p_event;
	if (k.is_valid() && k->is_pressed()) {
		bool handled = false;

		if (ED_IS_SHORTCUT("ui_undo", p_event)) {
			EditorNode::get_singleton()->undo();
			handled = true;
		}

		if (ED_IS_SHORTCUT("ui_redo", p_event)) {
			EditorNode::get_singleton()->redo();
			handled = true;
		}

		if (handled) {
			set_input_as_handled();
		}
	}
}
