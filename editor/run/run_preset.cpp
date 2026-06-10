/**************************************************************************/
/*  run_preset.cpp                                                        */
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

#include "run_preset.h"

#include "core/io/resource_loader.h"
#include "core/object/class_db.h"
#include "editor/editor_node.h"
#include "editor/editor_string_names.h"
#include "editor/export/editor_export.h"
#include "scene/resources/texture.h"

bool RunPreset::update_options() {
	cached_options.clear();
	if (get_destination() == RunDestination::DESTINATION_REMOTE && (get_remote_platform_id() == -1 || get_select_remote_platform_id())) {
		for (int i = 0; i < EditorExport::get_singleton()->get_export_platform_count(); i++) {
			Ref<EditorExportPlatform> eep = EditorExport::get_singleton()->get_export_platform(i);
			Ref<EditorExportPreset> preset = EditorExport::get_singleton()->get_runnable_preset_for_platform(eep);
			if (preset.is_null()) {
				continue;
			}
			const int device_count = MIN(eep->get_options_count(), 9000);
			String error;
			for (int j = 0; j < device_count; j++) {
				cached_options.push_back(RunPresetOptions{
						EditorExport::get_singleton()->encode_platform_device_id(i, j),
						eep->get_option_label(j),
						eep->get_option_icon(j),
				});
			}
		}
	} else if (get_destination() == RunDestination::DESTINATION_REMOTE && (get_remote_device_id() == -1 || get_select_remote_device_id())) {
		Ref<EditorExportPlatform> eep = EditorExport::get_singleton()->get_export_platform(get_remote_platform_id());
		const int device_count = MIN(eep->get_options_count(), 9000);
		for (int j = 0; j < device_count; j++) {
			cached_options.push_back(RunPresetOptions{
					EditorExport::get_singleton()->encode_platform_device_id(get_remote_platform_id(), j),
					eep->get_option_label(j),
					eep->get_option_icon(j),
			});
		}
	}
	emit_signal("changed");
	return false;
}

void RunPreset::set_option(int p_option_id) {
	int platform_id = EditorExport::decode_platform_from_id(p_option_id);
	int device_id = EditorExport::decode_device_from_id(p_option_id);
	set_remote_platform_id(platform_id);
	set_remote_device_id(device_id);
}

void RunPreset::stop() {
	if (select_remote_platform_id) {
		remote_platform_id = -1;
	}
	if (select_remote_device_id) {
		remote_device_id = -1;
	}
	if (mode == RunMode::RUN_CUSTOM && select_custom_scene_path) {
		custom_scene_path = "";
	}
}

String RunPreset::get_name() const {
	return name;
}
void RunPreset::set_name(const String &p_name) {
	name = p_name;
	emit_signal("changed");
}

bool RunPreset::get_use_custom_icon() const {
	return use_custom_icon;
}
void RunPreset::set_use_custom_icon(bool p_use) {
	use_custom_icon = p_use;
	emit_signal("changed");
	notify_property_list_changed();
}

String RunPreset::get_custom_icon_file_path() const {
	if (custom_icon.is_null()) {
		return "";
	}
	return custom_icon->get_path();
}
void RunPreset::set_custom_icon_file_path(const String &p_file_path) {
	custom_icon = ResourceLoader::load(p_file_path);
	emit_signal("changed");
}

String RunPreset::get_editor_icon() const {
	return editor_icon;
}
void RunPreset::set_editor_icon(const String &p_icon) {
	editor_icon = p_icon;
	emit_signal("changed");
}

RunMode RunPreset::get_mode() const {
	return mode;
}
void RunPreset::set_mode(RunMode p_mode) {
	mode = p_mode;
	emit_signal("changed");
}

bool RunPreset::get_use_current_mode() const {
	return use_current_mode;
}
void RunPreset::set_use_current_mode(bool p_use) {
	use_current_mode = p_use;
	emit_signal("changed");
}

int RunPreset::get_mode_as_int() const {
	if (use_current_mode) {
		return -1;
	}
	return static_cast<int>(mode);
}

void RunPreset::set_mode_as_int(int p_mode) {
	if (p_mode == -1) {
		use_current_mode = true;
	} else {
		mode = static_cast<RunMode>(p_mode);
		use_current_mode = false;
	}
	emit_signal("changed");
}

String RunPreset::get_custom_scene_path() const {
	return custom_scene_path;
}
void RunPreset::set_custom_scene_path(const String &p_path) {
	custom_scene_path = p_path;
	select_custom_scene_path = custom_scene_path.is_empty();
	emit_signal("changed");
}

bool RunPreset::needs_selecting_custom_scene_path() const {
	return select_custom_scene_path;
}

void RunPreset::set_running_scene_path(const String &p_path) {
	custom_scene_path = p_path;
}

RunDestination RunPreset::get_destination() const {
	return destination;
}
void RunPreset::set_destination(RunDestination p_destination) {
	destination = p_destination;
	update_options();
}

bool RunPreset::get_use_current_destination() const {
	return use_current_destination;
}
void RunPreset::set_use_current_destination(bool p_use) {
	use_current_destination = p_use;
	emit_signal("changed");
}

int RunPreset::get_destination_as_int() const {
	if (use_current_destination) {
		return -1;
	}
	return static_cast<int>(destination);
}

void RunPreset::set_destination_as_int(int p_destination) {
	if (p_destination == -1) {
		use_current_destination = true;
	} else {
		destination = static_cast<RunDestination>(p_destination);
		use_current_destination = false;
	}
	update_options();
	notify_property_list_changed();
}

int RunPreset::get_remote_platform_id() const {
	return remote_platform_id;
}
void RunPreset::set_remote_platform_id(int p_id) {
	remote_platform_id = p_id;
	emit_signal("changed");
}

bool RunPreset::get_select_remote_platform_id() const {
	return select_remote_platform_id;
}
void RunPreset::set_select_remote_platform_id(bool p_select) {
	select_remote_platform_id = p_select;
	emit_signal("changed");
}

int RunPreset::get_remote_platform_id_as_int() const {
	if (select_remote_platform_id) {
		return -1;
	}
	return remote_platform_id;
}

void RunPreset::set_remote_platform_id_as_int(int p_id) {
	if (p_id == -1) {
		select_remote_platform_id = true;
	} else {
		remote_platform_id = p_id;
		select_remote_platform_id = false;
	}
	update_options();
	notify_property_list_changed();
}

int RunPreset::get_remote_device_id() const {
	return remote_device_id;
}
void RunPreset::set_remote_device_id(int p_id) {
	remote_device_id = p_id;
	emit_signal("changed");
}

bool RunPreset::get_select_remote_device_id() const {
	return select_remote_device_id;
}
void RunPreset::set_select_remote_device_id(bool p_select) {
	select_remote_device_id = p_select;
	emit_signal("changed");
}

int RunPreset::get_remote_device_id_as_int() const {
	if (select_remote_device_id) {
		return -1;
	}
	return remote_device_id;
}

void RunPreset::set_remote_device_id_as_int(int p_id) {
	if (p_id == -1) {
		select_remote_device_id = true;
	} else {
		remote_device_id = p_id;
		select_remote_device_id = false;
	}
	emit_signal("changed");
	notify_property_list_changed();
}

bool RunPreset::get_show_toolbar() const {
	return show_toolbar;
}
void RunPreset::set_show_toolbar(bool p_show) {
	show_toolbar = p_show;
	emit_signal("changed");
}

bool RunPreset::get_show_toolbar_use_current() const {
	return show_toolbar_use_current;
}
void RunPreset::set_show_toolbar_use_current(bool p_use) {
	show_toolbar_use_current = p_use;
	emit_signal("changed");
}

int RunPreset::get_show_toolbar_as_int() const {
	if (show_toolbar_use_current) {
		return -1;
	}
	return static_cast<int>(show_toolbar);
}

void RunPreset::set_show_toolbar_as_int(int p_show) {
	if (p_show == -1) {
		show_toolbar_use_current = true;
	} else {
		show_toolbar = bool(p_show);
		show_toolbar_use_current = false;
	}
	emit_signal("changed");
}

bool RunPreset::get_run_xr_enabled() const {
	return run_xr_enabled;
}
void RunPreset::set_run_xr_enabled(bool p_enabled) {
	run_xr_enabled = p_enabled;
	emit_signal("changed");
}

bool RunPreset::get_run_xr_enabled_use_current() const {
	return run_xr_enabled_use_current;
}
void RunPreset::set_run_xr_enabled_use_current(bool p_use) {
	run_xr_enabled_use_current = p_use;
	emit_signal("changed");
}

int RunPreset::get_run_xr_enabled_as_int() const {
	if (run_xr_enabled_use_current) {
		return -1;
	}
	return static_cast<int>(run_xr_enabled);
}

void RunPreset::set_run_xr_enabled_as_int(int p_enabled) {
	if (p_enabled == -1) {
		run_xr_enabled_use_current = true;
	} else {
		run_xr_enabled = bool(p_enabled);
		run_xr_enabled_use_current = false;
	}
	emit_signal("changed");
}

bool RunPreset::is_pinned() const {
	return pinned;
}
void RunPreset::set_pinned(bool p_pinned) {
	pinned = p_pinned;
	emit_signal("changed");
}

Ref<Texture2D> RunPreset::get_icon() const {
	Ref<Texture2D> icon;
	if (use_custom_icon) {
		icon = custom_icon;
	} else {
		icon = EditorNode::get_singleton()->get_editor_theme()->get_icon(editor_icon, EditorStringName(EditorIcons));
	}
	return icon;
}

Dictionary RunPreset::to_dict() const {
	Dictionary dict;
	dict["name"] = name;
	dict["pinned"] = pinned;

	dict["use_custom_icon"] = use_custom_icon;
	if (use_custom_icon) {
		dict["custom_icon"] = custom_icon->get_path();
	} else {
		dict["editor_icon"] = editor_icon;
	}

	dict["mode"] = get_mode_as_int();
	if (!use_current_mode) {
		dict["custom_scene_path"] = custom_scene_path;
	}
	dict["destination"] = get_destination_as_int();
	dict["show_toolbar"] = get_show_toolbar_as_int();
	dict["run_xr_enabled"] = get_run_xr_enabled_as_int();
	dict["remote_platform_id"] = get_remote_platform_id_as_int();
	dict["remote_device_id"] = get_remote_device_id_as_int();
	if (destination == DESTINATION_REMOTE && !select_custom_scene_path) {
		// For backward compatibility, we need to save the remote platform id even if select_remote_platform_id is false, since before this property was added, the remote platform id was always saved.
		dict["custom_scene_path"] = custom_scene_path;
	}

	return dict;
}

Ref<RunPreset> RunPreset::from_dict(const Dictionary &dict) {
	Ref<RunPreset> preset = memnew(RunPreset);
	if (dict.is_empty()) {
		return preset;
		// If the dictionary is empty, we return a default preset. This can happen if the metadata was not set yet, or if it was cleared because of an error during loading (e.g. due to incompatible data).
	}
	preset->name = dict.get("name", "");
	preset->set_mode_as_int(static_cast<RunMode>(int(dict.get("mode", 0))));
	if (preset->get_mode() == RUN_CUSTOM) {
		preset->set_custom_scene_path(dict.get("custom_scene_path", ""));
	}
	preset->set_custom_scene_path(dict.get("custom_scene_path", ""));
	preset->set_destination_as_int(static_cast<RunDestination>(int(dict.get("destination", 0))));
	preset->set_remote_platform_id_as_int(dict.get("remote_platform_id", -1));
	preset->set_remote_device_id_as_int(dict.get("remote_device_id", -1));
	preset->set_show_toolbar_as_int(dict.get("show_toolbar", true));
	preset->set_run_xr_enabled_as_int(dict.get("run_xr_enabled", false));
	preset->set_use_custom_icon(dict.get("use_custom_icon", false));
	if (preset->get_use_custom_icon()) {
		preset->custom_icon = ResourceLoader::load(dict.get("custom_icon", ""));
	} else {
		preset->set_editor_icon(dict.get("editor_icon", ""));
	}
	preset->set_pinned(bool(dict.get("pinned", false)));
	return preset;
}

void RunPreset::_bind_methods() {
	ADD_SIGNAL(MethodInfo("changed"));

	ClassDB::bind_method(D_METHOD("get_name"), &RunPreset::get_name);
	ClassDB::bind_method(D_METHOD("set_name", "name"), &RunPreset::set_name);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "name"), "set_name", "get_name");

	ClassDB::bind_method(D_METHOD("get_use_custom_icon"), &RunPreset::get_use_custom_icon);
	ClassDB::bind_method(D_METHOD("set_use_custom_icon", "use"), &RunPreset::set_use_custom_icon);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_custom_icon"), "set_use_custom_icon", "get_use_custom_icon");

	ClassDB::bind_method(D_METHOD("get_custom_icon_file_path"), &RunPreset::get_custom_icon_file_path);
	ClassDB::bind_method(D_METHOD("set_custom_icon_file_path", "file_path"), &RunPreset::set_custom_icon_file_path);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "custom_icon_file_path", PROPERTY_HINT_FILE, "*.bmp,*.dds,*.exr,*.jpeg,*.jpg,*.hdr,*.png,*.svg,*.tga,*.webp"), "set_custom_icon_file_path", "get_custom_icon_file_path");

	ClassDB::bind_method(D_METHOD("get_editor_icon"), &RunPreset::get_editor_icon);
	ClassDB::bind_method(D_METHOD("set_editor_icon", "icon"), &RunPreset::set_editor_icon);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "editor_icon"), "set_editor_icon", "get_editor_icon");

	ClassDB::bind_method(D_METHOD("get_mode"), &RunPreset::get_mode_as_int);
	ClassDB::bind_method(D_METHOD("set_mode", "mode"), &RunPreset::set_mode_as_int);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "mode", PROPERTY_HINT_ENUM, "Use Current : -1, Main Scene, Current Scene, Custom Scene"), "set_mode", "get_mode");

	ClassDB::bind_method(D_METHOD("get_custom_scene_path"), &RunPreset::get_custom_scene_path);
	ClassDB::bind_method(D_METHOD("set_custom_scene_path", "path"), &RunPreset::set_custom_scene_path);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "custom_scene_path", PROPERTY_HINT_FILE, "*.tscn,*.scn"), "set_custom_scene_path", "get_custom_scene_path");

	ClassDB::bind_method(D_METHOD("get_destination"), &RunPreset::get_destination_as_int);
	ClassDB::bind_method(D_METHOD("set_destination", "destination"), &RunPreset::set_destination_as_int);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "destination", PROPERTY_HINT_ENUM, "Use Current : -1, Embedded in Editor, Floating Window, Remote"), "set_destination", "get_destination");

	ClassDB::bind_method(D_METHOD("get_remote_platform_id"), &RunPreset::get_remote_platform_id_as_int);
	ClassDB::bind_method(D_METHOD("set_remote_platform_id", "id"), &RunPreset::set_remote_platform_id_as_int);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "remote_platform_id", PROPERTY_HINT_ENUM, "Choose On Play : -1"), "set_remote_platform_id", "get_remote_platform_id");

	ClassDB::bind_method(D_METHOD("get_remote_device_id"), &RunPreset::get_remote_device_id_as_int);
	ClassDB::bind_method(D_METHOD("set_remote_device_id", "id"), &RunPreset::set_remote_device_id_as_int);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "remote_device_id", PROPERTY_HINT_ENUM, "Choose On Play : -1"), "set_remote_device_id", "get_remote_device_id");

	ClassDB::bind_method(D_METHOD("get_show_toolbar"), &RunPreset::get_show_toolbar_as_int);
	ClassDB::bind_method(D_METHOD("set_show_toolbar", "show"), &RunPreset::set_show_toolbar_as_int);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "show_toolbar", PROPERTY_HINT_ENUM, "Use Current : -1, OFF, ON"), "set_show_toolbar", "get_show_toolbar");

	ClassDB::bind_method(D_METHOD("get_run_xr_enabled"), &RunPreset::get_run_xr_enabled_as_int);
	ClassDB::bind_method(D_METHOD("set_run_xr_enabled", "enabled"), &RunPreset::set_run_xr_enabled_as_int);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "run_xr_enabled", PROPERTY_HINT_ENUM, "Use Current : -1, OFF, ON"), "set_run_xr_enabled", "get_run_xr_enabled");

	ClassDB::bind_method(D_METHOD("is_pinned"), &RunPreset::is_pinned);
	ClassDB::bind_method(D_METHOD("set_pinned", "pinned"), &RunPreset::set_pinned);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "pinned"), "set_pinned", "is_pinned");
}

void RunPreset::_validate_property(PropertyInfo &p_property) const {
	if (p_property.name == "editor_icon") {
		if (use_custom_icon) {
			p_property.usage &= ~PROPERTY_USAGE_EDITOR;
		} else {
			p_property.usage |= PROPERTY_USAGE_EDITOR;
		}
	} else if (p_property.name == "custom_icon_file_path") {
		if (use_custom_icon) {
			p_property.usage |= PROPERTY_USAGE_EDITOR;
		} else {
			p_property.usage &= ~PROPERTY_USAGE_EDITOR;
		}
	} else if (p_property.name == "remote_platform_id") {
		if (destination == DESTINATION_REMOTE) {
			p_property.hint_string = "Choose On Play:-1";
			for (int i = 0; i < EditorExport::get_singleton()->get_export_platform_count(); i++) {
				Ref<EditorExportPlatform> eep = EditorExport::get_singleton()->get_export_platform(i);
				p_property.hint_string += "," + eep->get_name();
			}
			p_property.usage |= PROPERTY_USAGE_EDITOR;
		} else {
			p_property.usage &= ~PROPERTY_USAGE_EDITOR;
		}
	} else if (p_property.name == "remote_device_id") {
		if (destination == DESTINATION_REMOTE && !select_remote_platform_id) {
			p_property.hint_string = "Choose On Play:-1";
			Ref<EditorExportPlatform> eep = EditorExport::get_singleton()->get_export_platform(remote_platform_id);
			for (int i = 0; i < eep->get_options_count(); i++) {
				p_property.hint_string += "," + eep->get_option_label(i);
			}
			p_property.usage |= PROPERTY_USAGE_EDITOR;
		} else {
			p_property.usage &= ~PROPERTY_USAGE_EDITOR;
		}
	}
}
