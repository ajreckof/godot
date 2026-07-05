/**************************************************************************/
/*  run_preset.h                                                          */
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

#pragma once

#include "core/io/resource.h"

class Texture2D;
class InputEventKey;
class Shortcut;

enum RunMode {
	RUN_MAIN,
	RUN_CURRENT,
	RUN_CUSTOM,
};

enum PlayPopupMenuItem {
	PLAY_POPUP_RUNNING_PRESET_OVERRIDE,
	PLAY_POPUP_RUN_SCENE_SEPARATOR,
	PLAY_POPUP_RUN_MAIN,
	PLAY_POPUP_RUN_CURRENT,
	PLAY_POPUP_RUN_SELECT_SCENE,
	PLAY_POPUP_RUN_DESTINATION_SEPARATOR,
	PLAY_POPUP_RUN_DESTINATION_EMBEDDED_IN_EDITOR,
	PLAY_POPUP_RUN_DESTINATION_FLOATING_WINDOW,
	PLAY_POPUP_RUN_OPTIONS_SEPARATOR,
	PLAY_POPUP_RUN_OPTIONS_SHOW_TOOLBAR,
	PLAY_POPUP_RUN_OPTIONS_RUN_XR_ENABLED,
	PLAY_POPUP_RUN_OPTIONS_MOVIE_MAKER_ENABLED,
	PLAY_POPUP_RUN_OPTIONS_MOVIE_MAKER_OPTIONS,
	PLAY_POPUP_RUN_DESTINATION_REMOTE = 1 << 4,
	PLAY_POPUP_SELECTED_SCENE = 1 << 5,
};
const int PLAY_POPUP_EXTRA_INFO = 6; // Number of bits used for encoding extra info in the popup item IDs.

enum RunDestination {
	DESTINATION_EMBEDDED_IN_EDITOR,
	DESTINATION_FLOATING_WINDOW,
	DESTINATION_REMOTE,
};

struct RunPresetOptions {
	int id;
	String name;
	Ref<Texture2D> icon;

	RunPresetOptions(int p_id, const String &p_name, const Ref<Texture2D> &p_icon) : id(p_id), name(p_name), icon(p_icon) {}
};
class RunPreset : public Resource {
	GDCLASS(RunPreset, Resource);

public:
	void update_from_current_preset(const Ref<RunPreset> &p_current_preset);

	void update_options();
	Vector<RunPresetOptions> get_options() const { return cached_options; }
	void set_option(int p_option_idx);

	void stop();

	String get_preset_name() const;
	String get_custom_preset_name() const;
	void set_preset_name(const String &p_name);

	bool get_use_custom_icon() const;
	void set_use_custom_icon(bool p_use);

	String get_custom_icon_uid() const;
	void set_custom_icon_uid(const String &p_uid);

	String get_editor_icon() const;
	void set_editor_icon(const String &p_icon);

	RunMode get_mode() const;
	void set_mode(RunMode p_mode);

	bool get_use_current_mode() const;
	void set_use_current_mode(bool p_use);

	int get_mode_as_int() const;
	void set_mode_as_int(int p_mode);

	String get_custom_scene_path() const;
	void set_custom_scene_path(const String &p_path);
	bool needs_selecting_custom_scene_path() const;
	void set_running_scene_path(const String &p_path);

	RunDestination get_destination() const;
	void set_destination(RunDestination p_destination);

	bool get_use_current_destination() const;
	void set_use_current_destination(bool p_use);

	int get_destination_as_int() const;
	void set_destination_as_int(int p_destination);

	int get_remote_platform_id() const;
	void set_remote_platform_id(int p_id);

	bool get_select_remote_platform_id() const;
	void set_select_remote_platform_id(bool p_select);

	int get_remote_platform_id_as_int() const;
	void set_remote_platform_id_as_int(int p_id);

	int get_remote_device_id() const;
	void set_remote_device_id(int p_id);

	bool get_select_remote_device_id() const;
	void set_select_remote_device_id(bool p_select);

	int get_remote_device_id_as_int() const;
	void set_remote_device_id_as_int(int p_id);

	bool get_show_toolbar() const;
	void set_show_toolbar(bool p_show);

	bool get_show_toolbar_use_current() const;
	void set_show_toolbar_use_current(bool p_use);

	int get_show_toolbar_as_int() const;
	void set_show_toolbar_as_int(int p_show);

	bool get_run_xr_enabled() const;
	void set_run_xr_enabled(bool p_enabled);

	bool get_run_xr_enabled_use_current() const;
	void set_run_xr_enabled_use_current(bool p_use);

	int get_run_xr_enabled_as_int() const;
	void set_run_xr_enabled_as_int(int p_enabled);

	bool is_pinned() const;
	void set_pinned(bool p_pinned);

	Ref<InputEventKey> get_input_event() const;
	void set_input_event(const Ref<InputEventKey> &p_input_event);

	Ref<Shortcut> get_shortcut() const;

	Ref<Texture2D> get_icon() const;

protected:
	static void _bind_methods();
	void _validate_property(PropertyInfo &p_property) const;

private:
	String preset_name;
	bool use_custom_icon = false;
	Ref<Texture2D> custom_icon = nullptr;
	String editor_icon = "";
	RunMode mode = RunMode::RUN_MAIN;
	bool use_current_mode = false;
	String custom_scene_path;
	bool select_custom_scene_path;
	RunDestination destination = RunDestination::DESTINATION_EMBEDDED_IN_EDITOR;
	bool use_current_destination = false;
	int remote_platform_id = -1;
	bool select_remote_platform_id = false;
	int remote_device_id = -1;
	bool select_remote_device_id = false;
	bool show_toolbar = true;
	bool show_toolbar_use_current = false;
	bool run_xr_enabled = false;
	bool run_xr_enabled_use_current = false;
	bool pinned = false;
	Ref<InputEventKey> input_event;
	Ref<Shortcut> shortcut;
	Vector<RunPresetOptions> cached_options;

public:
	RunPreset();
};
