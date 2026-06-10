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
class RunPreset;
class RunPresetManagerDialog;
class EditorRunBar : public MarginContainer {
	GDCLASS(EditorRunBar, MarginContainer);

	static EditorRunBar *singleton;

	PanelContainer *main_panel = nullptr;
	HBoxContainer *main_hbox = nullptr;
	HBoxContainer *main_play_hbox = nullptr;
	HBoxContainer *preset_hbox = nullptr;
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
	MenuButton *presets_menu_button = nullptr;
	RunPresetManagerDialog *run_preset_manager_dialog = nullptr;

	EditorRun editor_run;
	ConfirmationDialog *run_native_confirm = nullptr;
	RichTextLabel *native_result_dialog_log = nullptr;
	AcceptDialog *native_result_dialog = nullptr;
	bool run_native_confirmed = false;

	bool movie_maker_enabled = false;

	String run_custom_filename;
	String run_current_filename;
	PackedStringArray last_runned_scenes;
	Ref<RunPreset> current_preset;
	Ref<RunPreset> running_preset;
	int MAX_CACHED_RUN_SCENES = 3;

	void _reset_play_buttons();
	void _update_play_buttons();
	void _on_presets_menu_item_pressed(int p_id);
	void _on_presets_submenu_item_pressed(int p_option_id, int p_preset_id);

	void _movie_maker_item_pressed(int p_id);
	void _write_movie_toggled(bool p_enabled);
	void _quick_run_selected(const String &p_file_path);

	void _run_scene(const String &p_scene_path = "", const Vector<String> &p_run_args = Vector<String>());
	void _run_native(const Ref<EditorExportPreset> &p_preset);
	void _confirm_run_native();

	void _profiler_autostart_indicator_pressed();
	void _generate_popup_menu();
	void _generate_presets_buttons();
	void _update_presets_menu_button();
	void _on_popup_menu_id_pressed(int p_id);
	void _save_current_preset();

	void _selected_scene(const String p_scene_path);
	void _selected_running_scene(const String p_scene_path);

private:
	static Vector<String> _get_xr_mode_play_args(bool p_xr_enabled);

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

	void play_preset(const Ref<RunPreset> p_preset);

	void stop_playing();
	void notify_all_debug_sessions_exited();
	bool is_playing() const;
	String get_playing_scene() const;
	Ref<RunPreset> get_running_preset() const;

	ProcessID has_child_process(ProcessID p_pid) const;
	void stop_child_process(ProcessID p_pid);
	ProcessID get_current_process() const;

	void set_movie_maker_enabled(bool p_enabled);
	bool is_movie_maker_enabled() const;

	void update_profiler_autostart_indicator();

	Button *get_pause_button() { return pause_button; }

	HBoxContainer *get_buttons_container();

	EditorRunBar();
	Error start_run_native(int p_platform, int p_device);
	void resume_running_preset();
};
