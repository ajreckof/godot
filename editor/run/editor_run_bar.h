/**************************************************************************/
/*  editor_run_bar.h                                                      */
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

#include "core/os/process_id.h"
#include "editor/run/editor_run.h"
#include "scene/gui/margin_container.h"

class AcceptDialog;
class Button;
class EditorExportPreset;
class HBoxContainer;
class MenuButton;
class PanelContainer;
class ConfirmationDialog;
class RichTextLabel;
class PopupMenu;

class EditorRunBar : public MarginContainer {
	GDCLASS(EditorRunBar, MarginContainer);

	static EditorRunBar *singleton;

	enum RunMode {
		STOPPED = 0,
		RUN_MAIN,
		RUN_CURRENT,
		RUN_CUSTOM,
	};

	enum RunXRModeMenuItem {
		INVALID = -1,
		OFF = 0,
		ON = 1,
	};

	enum PlayPopupMenuItem {
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
	struct RunPreset {
		String name;
		RunMode mode = RunMode::RUN_MAIN;
		String custom_scene_path;
		RunDestination destination = RunDestination::DESTINATION_EMBEDDED_IN_EDITOR;
		int remote_platform_id = -1;
		int remote_device_id = -1;
		bool show_toolbar = true;
		bool run_xr_enabled = false;

		Dictionary to_dict() const {
			Dictionary dict;
			dict["name"] = name;
			dict["mode"] = mode;
			dict["custom_scene_path"] = custom_scene_path;
			dict["destination"] = destination;
			dict["remote_platform_id"] = remote_platform_id;
			dict["remote_device_id"] = remote_device_id;
			dict["show_toolbar"] = show_toolbar;
			dict["run_xr_enabled"] = run_xr_enabled;
			return dict;
		}

		static RunPreset from_dict(const Dictionary &dict) {
			if (dict.is_empty()) {
				return RunPreset();
				// If the dictionary is empty, we return a default preset. This can happen if the metadata was not set yet, or if it was cleared because of an error during loading (e.g. due to incompatible data).
			}
			RunPreset preset;
			preset.name = dict.get("name", "");
			preset.mode = static_cast<RunMode>(int(dict.get("mode", 0)));
			preset.custom_scene_path = dict.get("custom_scene_path", "");
			preset.destination = static_cast<RunDestination>(int(dict.get("destination", 0)));
			preset.remote_platform_id = int(dict.get("remote_platform_id", -1));
			preset.remote_device_id = int(dict.get("remote_device_id", -1));
			preset.show_toolbar = dict.get("show_toolbar", true);
			preset.run_xr_enabled = dict.get("run_xr_enabled", false);
			return preset;
		}
	};

	PanelContainer *main_panel = nullptr;
	HBoxContainer *main_hbox = nullptr;
	HBoxContainer *main_play_hbox = nullptr;
	MenuButton *main_play_menu_button = nullptr;
	PopupMenu *main_play_popup = nullptr;
	HBoxContainer *outer_hbox = nullptr;

	Button *profiler_autostart_indicator = nullptr;

	PanelContainer *recovery_mode_panel = nullptr;
	Button *recovery_mode_button = nullptr;
	Button *recovery_mode_reload_button = nullptr;
	AcceptDialog *recovery_mode_popup = nullptr;

	Button *play_button = nullptr;
	Button *pause_button = nullptr;
	Button *stop_button = nullptr;
	Button *play_scene_button = nullptr;
	Button *play_custom_scene_button = nullptr;

	EditorRun editor_run;
	ConfirmationDialog *run_native_confirm = nullptr;
	RichTextLabel *native_result_dialog_log = nullptr;
	AcceptDialog *native_result_dialog = nullptr;
	bool run_native_confirmed = false;

	bool movie_maker_enabled = false;

	String run_custom_filename;
	String run_current_filename;
	PackedStringArray last_runned_scenes;
	RunPreset current_preset;
	RunPreset *running_preset;
	int MAX_CACHED_RUN_SCENES = 3;

	void _reset_play_buttons();
	void _update_play_buttons();

	void _movie_maker_item_pressed(int p_id);
	void _write_movie_toggled(bool p_enabled);
	void _quick_run_selected(const String &p_file_path, int p_menu_item = RunXRModeMenuItem::INVALID);

	void _play_current_pressed(int p_menu_item = RunXRModeMenuItem::INVALID);
	void _play_custom_pressed(int p_menu_item = RunXRModeMenuItem::INVALID);

	void _run_scene(const String &p_scene_path = "", const Vector<String> &p_run_args = Vector<String>());
	void _run_native(const Ref<EditorExportPreset> &p_preset);
	void _confirm_run_native();

	void _profiler_autostart_indicator_pressed();
	void _generate_popup_menu();
	void _on_popup_menu_id_pressed(int p_id);
	void _save_current_preset();

	void _selected_scene(const String p_scene_path);
	void _selected_running_scene(const String p_scene_path);

private:
	static Vector<String> _get_xr_mode_play_args(RunXRModeMenuItem p_menu_item);

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	static EditorRunBar *get_singleton() { return singleton; }

	void recovery_mode_show_dialog();
	void recovery_mode_reload_project();

	void play_current_preset();
	void play_main_scene(const Vector<String> &p_play_args = Vector<String>());
	void play_current_scene(const Vector<String> &p_play_args = Vector<String>());
	void play_custom_scene(const String &p_scene_path, const Vector<String> &p_play_args = Vector<String>());

	void play_preset(RunPreset &p_preset);

	void stop_playing();
	bool is_playing() const;
	String get_playing_scene() const;

	ProcessID has_child_process(ProcessID p_pid) const;
	void stop_child_process(ProcessID p_pid);
	ProcessID get_current_process() const;

	void set_movie_maker_enabled(bool p_enabled);
	bool is_movie_maker_enabled() const;

	void update_profiler_autostart_indicator();

	Button *get_pause_button() { return pause_button; }

	HBoxContainer *get_buttons_container();

	EditorRunBar();
	Error start_run_native(int platform, int device);
	void resume_running_preset();
};
