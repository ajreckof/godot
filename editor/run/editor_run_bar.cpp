/**************************************************************************/
/*  editor_run_bar.cpp                                                    */
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

#include "editor_run_bar.h"

#include "core/config/engine.h"
#include "core/config/project_settings.h"
#include "core/object/callable_mp.h"
#include "editor/debugger/editor_debugger_node.h"
#include "editor/debugger/script_editor_debugger.h"
#include "editor/editor_node.h"
#include "editor/editor_string_names.h"
#include "editor/export/editor_export.h"
#include "editor/export/editor_export_platform.h"
#include "editor/export/editor_export_preset.h"
#include "editor/gui/editor_bottom_panel.h"
#include "editor/gui/editor_quick_open_dialog.h"
#include "editor/gui/editor_toaster.h"
#include "editor/run/editor_run.h"
#include "editor/run/game_view_plugin.h"
#include "editor/run/run_preset.h"
#include "editor/run/run_preset_button.h"
#include "editor/run/run_preset_manager.h"
#include "editor/settings/editor_command_palette.h"
#include "editor/settings/editor_settings.h"
#include "editor/settings/project_settings_editor.h"
#include "editor/themes/editor_scale.h"
#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/menu_button.h"
#include "scene/gui/panel_container.h"
#include "scene/gui/rich_text_label.h"
#include "scene/main/scene_tree.h"
#include "servers/display/display_server.h"

#ifndef XR_DISABLED
#include "servers/xr/xr_server.h"
#endif // XR_DISABLED

EditorRunBar *EditorRunBar::singleton = nullptr;

void EditorRunBar::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_POSTINITIALIZE: {
			_reset_play_buttons();
		} break;

		case NOTIFICATION_READY: {
			if (Engine::get_singleton()->is_recovery_mode_hint()) {
				recovery_mode_show_dialog();
			}

			Dictionary current_preset_data = EditorSettings::get_singleton()->get_project_metadata("editor_run_bar", "current_preset", Dictionary());
			current_preset = RunPreset::from_dict(current_preset_data);
			if (DisplayServer::get_singleton()->has_feature(DisplayServerEnums::FEATURE_WINDOW_EMBEDDING)) {
				int game_mode = EDITOR_GET("run/window_placement/game_embed_mode");
				switch (game_mode) {
					case -1: { // Disabled.
						current_preset->set_destination(DESTINATION_FLOATING_WINDOW);
						current_preset->set_show_toolbar(false);
					} break;
					case 1: { // Embed.
						current_preset->set_destination(DESTINATION_EMBEDDED_IN_EDITOR);
						current_preset->set_show_toolbar(true);
					} break;
					case 2: { // Floating.
						current_preset->set_destination(DESTINATION_FLOATING_WINDOW);
						current_preset->set_show_toolbar(true);
					} break;
					default: {
						// nothing to do here, we keep the value that was saved last time.
					} break;
				}
			} else {
				current_preset->set_destination(DESTINATION_FLOATING_WINDOW);
				current_preset->set_show_toolbar(false);
			}

			_generate_popup_menu();
			_generate_presets_buttons();
			set_process(true);
		} break;

		case NOTIFICATION_THEME_CHANGED: {
			if (Engine::get_singleton()->is_recovery_mode_hint()) {
				main_panel->add_theme_style_override(SceneStringName(panel), get_theme_stylebox(SNAME("LaunchPadRecoveryMode"), EditorStringName(EditorStyles)));
				recovery_mode_panel->add_theme_style_override(SceneStringName(panel), get_theme_stylebox(SNAME("RecoveryModeButton"), EditorStringName(EditorStyles)));
				recovery_mode_button->add_theme_style_override("hover", get_theme_stylebox(SNAME("RecoveryModeButton"), EditorStringName(EditorStyles)));

				recovery_mode_button->set_button_icon(get_editor_theme_icon(SNAME("NodeWarning")));
				recovery_mode_reload_button->set_button_icon(get_editor_theme_icon(SNAME("Reload")));

				recovery_mode_button->begin_bulk_theme_override();
				recovery_mode_button->add_theme_color_override("icon_normal_color", Color(0.3, 0.3, 0.3, 1));
				recovery_mode_button->add_theme_color_override("icon_pressed_color", Color(0.4, 0.4, 0.4, 1));
				recovery_mode_button->add_theme_color_override("icon_hover_color", Color(0.6, 0.6, 0.6, 1));
				Color dark_color = get_theme_color("recovery_mode_text_color", EditorStringName(Editor));
				recovery_mode_button->add_theme_color_override(SceneStringName(font_color), dark_color);
				recovery_mode_button->add_theme_color_override("font_pressed_color", dark_color.lightened(0.2));
				recovery_mode_button->add_theme_color_override("font_hover_color", dark_color.lightened(0.4));
				recovery_mode_button->add_theme_color_override("font_hover_pressed_color", dark_color.lightened(0.2));
				recovery_mode_button->end_bulk_theme_override();

				return;
			}

			_update_play_buttons();
			profiler_autostart_indicator->set_button_icon(get_editor_theme_icon(SNAME("ProfilerAutostartWarning")));
			pause_button->set_button_icon(get_editor_theme_icon(SNAME("Pause")));
			stop_button->set_button_icon(get_editor_theme_icon(SNAME("Stop")));

			presets_menu_button->set_button_icon(get_editor_theme_icon(SNAME("GuiTabMenuHl")));
			presets_menu_button->get_popup()->add_theme_constant_override("icon_max_width", get_theme_constant("class_icon_size", EditorStringName(Editor)));

		} break;

		case NOTIFICATION_PROCESS: {
			bool changed = EditorExport::get_singleton()->poll_export_platforms();

			if (changed) {
				_generate_popup_menu();
				for (Ref<RunPreset> preset : run_preset_manager_dialog->get_presets()) {
					preset->update_options();
				}
				_update_presets_menu_button();
			}
		} break;
	}
}

void EditorRunBar::_reset_play_buttons() {
	if (Engine::get_singleton()->is_recovery_mode_hint()) {
		return;
	}

	play_button->set_pressed(false);
	play_button->set_button_icon(get_editor_theme_icon(SNAME("MainPlay")));
	play_button->set_tooltip_text(TTRC("Run the project's main scene."));
}

void EditorRunBar::_update_play_buttons() {
	if (Engine::get_singleton()->is_recovery_mode_hint()) {
		return;
	}

	_reset_play_buttons();
	if (!is_playing() || current_preset != running_preset) {
		return;
	}

	play_button->set_pressed(true);
	play_button->set_button_icon(get_editor_theme_icon(SNAME("Reload")));
}

void EditorRunBar::_on_presets_menu_item_pressed(int p_id) {
	Vector<Ref<RunPreset>> presets = run_preset_manager_dialog->get_presets();
	if (p_id >= 0 && p_id < presets.size()) {
		play_preset(presets[p_id]);
		return;
	} else if (p_id == presets.size()) {
		// "Manage Presets..." is always the last item in the menu.
		run_preset_manager_dialog->popup_centered_ratio(0.6);
		return;
	}
}

void EditorRunBar::_on_presets_submenu_item_pressed(int p_option_id, int p_preset_id) {
	run_preset_manager_dialog->get_presets()[p_preset_id]->set_option(p_option_id);
	_on_presets_menu_item_pressed(p_preset_id);
}

void EditorRunBar::_run_scene(const String &p_scene_path, const Vector<String> &p_run_args) {
	ERR_FAIL_COND_MSG(p_scene_path.is_empty(), "Attempting to run a custom scene with an empty path.");

	if (editor_run.get_status() == EditorRun::STATUS_PLAY) {
		return;
	}

	if (!EditorNode::get_singleton()->validate_custom_directory()) {
		return;
	}

	_reset_play_buttons();

	String resource_path = ProjectSettings::get_singleton()->get_resource_path();
	if (!resource_path.is_empty()) {
		String project_file_path = resource_path.path_join("project.godot");
		if (!FileAccess::exists(project_file_path)) {
			// TODO: Try to recover the "project.godot" file using ProjectSettings::get_singleton()->save()
			EditorNode::get_singleton()->show_warning(
					TTRC("Failed to run the project because the project.godot file is missing."),
					TTRC("Error!"));
			return;
		}
	}

	String write_movie_file;
	if (is_movie_maker_enabled()) {
		if (running_preset->get_mode() == RUN_CURRENT) {
			Node *scene_root = nullptr;
			if (p_scene_path.is_empty()) {
				scene_root = get_tree()->get_edited_scene_root();
			} else {
				int scene_index = EditorNode::get_editor_data().get_edited_scene_from_path(p_scene_path);
				if (scene_index >= 0) {
					scene_root = EditorNode::get_editor_data().get_edited_scene_root(scene_index);
				}
			}

			if (scene_root && scene_root->has_meta("movie_file")) {
				// If the scene file has a movie_file metadata set, use this as file.
				// Quick workaround if you want to have multiple scenes that write to
				// multiple movies.
				write_movie_file = scene_root->get_meta("movie_file");
			}
		}

		if (write_movie_file.is_empty()) {
			write_movie_file = GLOBAL_GET("editor/movie_writer/movie_file");
		}

		if (write_movie_file.is_empty()) {
			// TODO: Provide options to directly resolve the issue with a custom dialog.
			EditorNode::get_singleton()->show_warning(TTR("Movie Maker mode is enabled, but no movie file path has been specified.\nA default movie file path can be specified in the project settings under the Editor > Movie Writer category.\nAlternatively, for running single scenes, a `movie_file` string metadata can be added to the root node,\nspecifying the path to a movie file that will be used when recording that scene."));
			return;
		}
	}

	EditorNode::get_singleton()->try_autosave();
	if (!EditorNode::get_singleton()->call_build()) {
		return;
	}

	Vector<String> args = p_run_args;
	EditorNode::get_singleton()->call_run_scene(p_scene_path, args);

	// Use the existing URI, in case it is overridden by the CLI.
	String uri = EditorDebuggerNode::get_singleton()->get_server_uri();
	if (uri.is_empty()) {
		uri = "tcp://";
	}
	EditorDebuggerNode::get_singleton()->start(uri);
	Error error = editor_run.run(p_scene_path, write_movie_file, args);
	if (error != OK) {
		EditorDebuggerNode::get_singleton()->stop();
		EditorNode::get_singleton()->show_warning(TTR("Could not start subprocess(es)!"), TTR("OK"));
		return;
	}

	_update_play_buttons();
	stop_button->set_disabled(false);

	emit_signal(SNAME("play_pressed"));
}

void EditorRunBar::_confirm_run_native() {
	run_native_confirmed = true;
	resume_running_preset();
}

void EditorRunBar::_profiler_autostart_indicator_pressed() {
	// Switch to the first profiler tab in the bottom panel.
	EditorDebuggerNode::get_singleton()->make_visible();

	if (EditorSettings::get_singleton()->get_project_metadata("debug_options", "autostart_profiler", false)) {
		EditorDebuggerNode::get_singleton()->get_current_debugger()->switch_to_debugger(3);
	} else if (EditorSettings::get_singleton()->get_project_metadata("debug_options", "autostart_visual_profiler", false)) {
		EditorDebuggerNode::get_singleton()->get_current_debugger()->switch_to_debugger(4);
	} else {
		// Switch to the network profiler tab.
		EditorDebuggerNode::get_singleton()->get_current_debugger()->switch_to_debugger(8);
	}
}

void EditorRunBar::_generate_popup_menu() {
	main_play_menu_button->set_text(current_preset->get_name());

	main_play_popup->clear();

	main_play_popup->add_separator(TTR("Run Scene"), PLAY_POPUP_RUN_SCENE_SEPARATOR);

	main_play_popup->add_radio_check_item(TTR("Main Scene"), PLAY_POPUP_RUN_MAIN);
	main_play_popup->set_item_checked(-1, current_preset->get_mode() == RunMode::RUN_MAIN);

	main_play_popup->add_radio_check_item(TTR("Current Scene"), PLAY_POPUP_RUN_CURRENT);
	main_play_popup->set_item_checked(-1, current_preset->get_mode() == RunMode::RUN_CURRENT);
	main_play_popup->set_item_disabled(-1, current_preset->get_destination() == DESTINATION_REMOTE);

	for (int i = 0; i < last_runned_scenes.size(); i++) {
		main_play_popup->add_radio_check_item(last_runned_scenes[i].get_file(), (i << PLAY_POPUP_EXTRA_INFO) + PLAY_POPUP_SELECTED_SCENE); // +1 to account for the "Select Scene..." item.
		main_play_popup->set_item_tooltip(-1, last_runned_scenes[i]);
		main_play_popup->set_item_checked(-1, current_preset->get_mode() == RunMode::RUN_CUSTOM && current_preset->get_custom_scene_path() == last_runned_scenes[i]);
		main_play_popup->set_item_disabled(-1, current_preset->get_destination() == DESTINATION_REMOTE);
	}
	main_play_popup->add_item(TTR("Select Scene..."), PLAY_POPUP_RUN_SELECT_SCENE);
	main_play_popup->set_item_disabled(-1, current_preset->get_destination() == DESTINATION_REMOTE);

	main_play_popup->add_separator(TTR("Run Destination"), PLAY_POPUP_RUN_DESTINATION_SEPARATOR);

	main_play_popup->add_radio_check_item(TTR("Floating Window"), PLAY_POPUP_RUN_DESTINATION_FLOATING_WINDOW);
	main_play_popup->set_item_checked(-1, current_preset->get_destination() == RunDestination::DESTINATION_FLOATING_WINDOW);

	main_play_popup->add_radio_check_item(TTR("Embedded in Editor"), PLAY_POPUP_RUN_DESTINATION_EMBEDDED_IN_EDITOR);
	main_play_popup->set_item_checked(-1, current_preset->get_destination() == RunDestination::DESTINATION_EMBEDDED_IN_EDITOR);

	int device_shortcut_id = 1;
	LocalVector<int> platform_idx_with_options;
	for (int i = 0; i < EditorExport::get_singleton()->get_export_platform_count(); i++) {
		Ref<EditorExportPlatform> eep = EditorExport::get_singleton()->get_export_platform(i);
		Ref<EditorExportPreset> preset = EditorExport::get_singleton()->get_runnable_preset_for_platform(eep);
		if (preset.is_null()) {
			continue;
		}
		const int device_count = MIN(eep->get_options_count(), 9000);
		bool has_options = false;
		if (device_count > 0) {
			for (int j = 0; j < device_count; j++) {
				if (eep->is_option_runnable(j)) {
					main_play_popup->add_icon_radio_check_item(eep->get_option_icon(j), eep->get_option_label(j), (EditorExport::encode_platform_device_id(i, j) << PLAY_POPUP_EXTRA_INFO) + PLAY_POPUP_RUN_DESTINATION_REMOTE);
					main_play_popup->set_item_checked(-1, current_preset->get_destination() == DESTINATION_REMOTE && current_preset->get_remote_platform_id() == i && current_preset->get_remote_device_id() == j);

					if (device_shortcut_id <= 4) {
						// Assign shortcuts for the first 4 devices added in the list.
						main_play_popup->set_item_shortcut(-1, ED_GET_SHORTCUT(vformat("remote_deploy/deploy_to_device_%d", device_shortcut_id)), true);
						device_shortcut_id += 1;
					}
					main_play_popup->set_item_tooltip(-1, eep->get_option_tooltip(j));
				} else {
					has_options = true;
				}
			}
			if (has_options) {
				platform_idx_with_options.push_back(i);
			}
		}
	}

	main_play_popup->add_separator(TTRC("Run Options"), PLAY_POPUP_RUN_OPTIONS_SEPARATOR);

	main_play_popup->add_check_item(TTRC("Show Toolbar"), PLAY_POPUP_RUN_OPTIONS_SHOW_TOOLBAR);
	main_play_popup->set_item_disabled(-1, current_preset->get_destination() != DESTINATION_FLOATING_WINDOW);
	main_play_popup->set_item_checked(-1, current_preset->get_show_toolbar());
#ifndef XR_DISABLED
	if (XRServer::get_xr_mode() == XRServer::XRMODE_ON ||
			(XRServer::get_xr_mode() == XRServer::XRMODE_DEFAULT && GLOBAL_GET("xr/openxr/enabled"))) {
		main_play_popup->add_check_item(TTRC("XR Mode Enabled"), PLAY_POPUP_RUN_OPTIONS_RUN_XR_ENABLED);
		main_play_popup->set_item_checked(-1, current_preset->get_run_xr_enabled());
	}
#endif
	main_play_popup->add_check_item(TTRC("Movie Maker Mode Enabled"), PLAY_POPUP_RUN_OPTIONS_MOVIE_MAKER_ENABLED);
	main_play_popup->set_item_checked(-1, is_movie_maker_enabled());
	main_play_popup->set_item_disabled(-1, current_preset->get_destination() == DESTINATION_REMOTE);
	main_play_popup->add_item(TTRC("Movie Maker Options..."), PLAY_POPUP_RUN_OPTIONS_MOVIE_MAKER_OPTIONS);

	for (int platform_idx : platform_idx_with_options) {
		Ref<EditorExportPlatform> eep = EditorExport::get_singleton()->get_export_platform(platform_idx);
		const int device_count = MIN(eep->get_options_count(), 9000);
		for (int j = 0; j < device_count; j++) {
			if (!eep->is_option_runnable(j)) {
				main_play_popup->add_icon_item(eep->get_option_icon(j), eep->get_name() + " : " + eep->get_option_label(j), (EditorExport::encode_platform_device_id(platform_idx, j) << PLAY_POPUP_EXTRA_INFO) + PLAY_POPUP_RUN_DESTINATION_REMOTE);
				main_play_popup->set_item_tooltip(-1, eep->get_option_tooltip(j));
			}
		}
		main_play_popup->set_item_disabled(main_play_popup->get_item_index((EditorExport::encode_platform_device_id(platform_idx, 0) << PLAY_POPUP_EXTRA_INFO) + PLAY_POPUP_RUN_DESTINATION_REMOTE), false);
	}

	// Save the current present in project metadata, anytime there is a modification of it there should be a regeneration anyway.
	EditorSettings::get_singleton()->set_project_metadata("editor_run_bar", "current_preset", current_preset->to_dict());
}

void EditorRunBar::_generate_presets_buttons() {
	preset_hbox->remove_child(presets_menu_button);

	for (int i = 0; i < preset_hbox->get_child_count(); i++) {
		preset_hbox->get_child(i)->queue_free();
	}

	RunPresetButton *preset_button = nullptr;
	for (Ref<RunPreset> preset : run_preset_manager_dialog->get_presets()) {
		preset_button = memnew(RunPresetButton);
		preset_button->set_preset(preset);
		preset_hbox->add_child(preset_button);
	}
	_update_presets_menu_button();
	preset_hbox->add_child(presets_menu_button);
}

void EditorRunBar::_update_presets_menu_button() {
	presets_menu_button->get_popup()->clear(true);
	for (Ref<RunPreset> preset : run_preset_manager_dialog->get_presets()) {
		if (preset->is_pinned()) {
			continue;
		}
		Vector<RunPresetOptions> options = preset->get_options();
		if (options.is_empty()) {
			presets_menu_button->get_popup()->add_icon_item(preset->get_icon(), preset->get_name());
		} else {
			PopupMenu *submenu = memnew(PopupMenu);
			for (const RunPresetOptions &option : options) {
				submenu->add_icon_item(option.icon, option.name, option.id);
			}
			submenu->connect(SceneStringName(id_pressed), callable_mp(this, &EditorRunBar::_on_presets_submenu_item_pressed).bind(presets_menu_button->get_popup()->get_item_count()));
			presets_menu_button->get_popup()->add_submenu_node_item(preset->get_name(), submenu);
			presets_menu_button->get_popup()->set_item_icon(-1, preset->get_icon());
		}
	}
	presets_menu_button->get_popup()->add_item(TTR("Manage Presets..."), run_preset_manager_dialog->get_presets().size());
}

void EditorRunBar::_on_popup_menu_id_pressed(int p_id) {
	switch (p_id) {
		case PLAY_POPUP_RUN_MAIN:
			current_preset->set_mode(RunMode::RUN_MAIN);
			break;
		case PLAY_POPUP_RUN_CURRENT:
			current_preset->set_mode(RunMode::RUN_CURRENT);
			break;
		case PLAY_POPUP_RUN_SELECT_SCENE:
			main_play_popup->hide();
			main_play_popup->set_flag(Window::FLAG_POPUP, false); // Prevent closing popup while selecting a scene.
			main_play_popup->show(); // Re-show the popup to apply the flag change immediately.
			EditorNode::get_singleton()->get_quick_open_dialog()->popup_dialog({ "PackedScene" }, callable_mp(this, &EditorRunBar::_selected_scene));
			break;
		case PLAY_POPUP_RUN_DESTINATION_FLOATING_WINDOW:
			current_preset->set_destination(RunDestination::DESTINATION_FLOATING_WINDOW);
			break;
		case PLAY_POPUP_RUN_DESTINATION_EMBEDDED_IN_EDITOR:
			current_preset->set_destination(RunDestination::DESTINATION_EMBEDDED_IN_EDITOR);
			current_preset->set_show_toolbar(true);
			break;
		case PLAY_POPUP_RUN_OPTIONS_SHOW_TOOLBAR:
			current_preset->set_show_toolbar(!current_preset->get_show_toolbar());
			break;
		case PLAY_POPUP_RUN_OPTIONS_RUN_XR_ENABLED:
			current_preset->set_run_xr_enabled(!current_preset->get_run_xr_enabled());
			break;
		case PLAY_POPUP_RUN_OPTIONS_MOVIE_MAKER_ENABLED:
			set_movie_maker_enabled(!is_movie_maker_enabled());
			break;
		case PLAY_POPUP_RUN_OPTIONS_MOVIE_MAKER_OPTIONS:
			ProjectSettingsEditor::get_singleton()->popup_project_settings(true);
			ProjectSettingsEditor::get_singleton()->set_general_page("editor/movie_writer");
			break;
		default:
			if (p_id & PLAY_POPUP_SELECTED_SCENE) {
				int scene_index = p_id >> PLAY_POPUP_EXTRA_INFO;
				_selected_scene(last_runned_scenes[scene_index]);
			} else if (p_id & PLAY_POPUP_RUN_DESTINATION_REMOTE) {
				int platform_idx = EditorExport::decode_platform_from_id(p_id >> PLAY_POPUP_EXTRA_INFO);
				int device_idx = EditorExport::decode_device_from_id(p_id >> PLAY_POPUP_EXTRA_INFO);
				if (EditorExport::get_singleton()->get_export_platform(platform_idx)->is_option_runnable(device_idx)) {
					current_preset->set_destination(RunDestination::DESTINATION_REMOTE);
					current_preset->set_show_toolbar(false);
					current_preset->set_mode(RunMode::RUN_MAIN); // Remote run only supports running the main scene for now, so switch to this mode if not already.
					current_preset->set_remote_platform_id_as_int(platform_idx);
					current_preset->set_remote_device_id_as_int(device_idx);
				} else {
					start_run_native(platform_idx, device_idx);
				}
			}

			break;
	}
	_generate_popup_menu();
}

void EditorRunBar::_selected_scene(const String p_scene_path) {
	current_preset->set_mode(RunMode::RUN_CUSTOM);
	current_preset->set_custom_scene_path(p_scene_path);
	last_runned_scenes.erase(p_scene_path);
	last_runned_scenes.insert(0, p_scene_path);
	if (last_runned_scenes.size() > MAX_CACHED_RUN_SCENES) {
		last_runned_scenes.resize(MAX_CACHED_RUN_SCENES);
	}
	main_play_popup->hide();
	main_play_popup->set_flag(PopupMenu::FLAG_POPUP, true); // Re-enable popup closing after selecting a scene.
	main_play_popup->show();
	_generate_popup_menu();
}

void EditorRunBar::_selected_running_scene(const String p_scene_path) {
	running_preset->set_running_scene_path(p_scene_path);
	resume_running_preset();
}

void EditorRunBar::recovery_mode_show_dialog() {
	recovery_mode_popup->popup_centered();
}

void EditorRunBar::recovery_mode_reload_project() {
	EditorNode::get_singleton()->trigger_menu_option(EditorNode::PROJECT_RELOAD_CURRENT_PROJECT, false);
}
void EditorRunBar::play_current_preset() {
	play_preset(current_preset);
}

void EditorRunBar::play_main_scene(const Vector<String> &p_play_args) {
}

void EditorRunBar::play_current_scene(const Vector<String> &p_play_args) {
}

void EditorRunBar::play_custom_scene(const String &p_scene_path, const Vector<String> &p_play_args) {
}

void EditorRunBar::play_preset(const Ref<RunPreset> p_preset) {
	if (p_preset->get_destination() == DESTINATION_REMOTE) {
		Error err = start_run_native(p_preset->get_remote_platform_id(), p_preset->get_remote_device_id());
		if (err == OK) {
			Ref<EditorExportPlatform> eep = EditorExport::get_singleton()->get_export_platform(p_preset->get_remote_platform_id());
			if (eep->is_option_runnable(p_preset->get_remote_device_id())) {
				running_preset = p_preset;
				emit_signal(SNAME("play_pressed"));
				stop_button->set_disabled(!eep->is_option_stoppable(p_preset->get_remote_device_id()));
			}
		}
		return;
	}
	if (editor_run.get_status() == EditorRun::STATUS_PLAY) {
		stop_playing();
	}
	running_preset = p_preset;

	if (p_preset->get_mode() == RUN_CUSTOM && p_preset->needs_selecting_custom_scene_path() && p_preset->get_custom_scene_path().is_empty()) {
		EditorNode::get_singleton()->get_quick_open_dialog()->popup_dialog(
				{ "PackedScene" },
				callable_mp(this, &EditorRunBar::_selected_running_scene));
		return;
	}
	resume_running_preset();
}

void EditorRunBar::resume_running_preset() {
	String run_filename;
	switch (running_preset->get_mode()) {
		case RUN_CUSTOM: {
			run_filename = ResourceUID::ensure_path(running_preset->get_custom_scene_path());
			run_custom_filename = run_filename;
		} break;

		case RUN_CURRENT: {
			Node *scene_root = get_tree()->get_edited_scene_root();
			if (!scene_root) {
				EditorNode::get_singleton()->show_warning(TTR("There is no defined scene to run."));
				return;
			}

			if (scene_root->get_scene_file_path().is_empty()) {
				EditorNode::get_singleton()->save_before_run();
				return;
			}

			run_filename = scene_root->get_scene_file_path();
			run_current_filename = run_filename;
		} break;

		default: {
			if (!EditorNode::get_singleton()->ensure_main_scene(false)) {
				return;
			}

			run_filename = GLOBAL_GET("application/run/main_scene");
		} break;
	}

	GameView::get_singleton()->set_embed_options(
			running_preset->get_show_toolbar(),
			running_preset->get_destination() == DESTINATION_FLOATING_WINDOW);

	run_current_filename = run_filename;

	Vector<String> play_args;
#ifndef XR_DISABLED
	if (XRServer::get_xr_mode() == XRServer::XRMODE_ON ||
			(XRServer::get_xr_mode() == XRServer::XRMODE_DEFAULT && GLOBAL_GET("xr/openxr/enabled"))) {
		if (running_preset->get_run_xr_enabled()) {
			// Play in regular mode, xr mode off.
			play_args.push_back("--xr-mode");
			play_args.push_back("on");
		} else {
			// Play in xr mode.
			play_args.push_back("--xr-mode");
			play_args.push_back("off");
		}
	}
#endif // XR_DISABLED

	_run_scene(run_filename, play_args);
}

void EditorRunBar::stop_playing() {
	if (editor_run.get_status() == EditorRun::STATUS_STOP) {
		return;
	}

	if (running_preset.is_valid()) {
		if (running_preset->get_destination() == DESTINATION_REMOTE) {
			Ref<EditorExportPlatform> eep = EditorExport::get_singleton()->get_export_platform(running_preset->get_remote_platform_id());
			eep->stop();
		}
		running_preset->stop();
		running_preset = nullptr;
	}
	editor_run.stop();
	EditorDebuggerNode::get_singleton()->stop();

	run_custom_filename.clear();
	run_current_filename.clear();
	stop_button->set_pressed(false);
	stop_button->set_disabled(true);
	_reset_play_buttons();

	emit_signal(SNAME("stop_pressed"));
}

void EditorRunBar::notify_all_debug_sessions_exited() {
	if (running_preset.is_valid() && running_preset->get_destination() != DESTINATION_REMOTE) {
		stop_playing();
	}
}

bool EditorRunBar::is_playing() const {
	EditorRun::Status status = editor_run.get_status();
	return (status == EditorRun::STATUS_PLAY || status == EditorRun::STATUS_PAUSED);
}

String EditorRunBar::get_playing_scene() const {
	String run_filename = editor_run.get_running_scene();
	if (run_filename.is_empty() && is_playing()) {
		run_filename = GLOBAL_GET("application/run/main_scene"); // Must be the main scene then.
	}

	return run_filename;
}

Ref<RunPreset> EditorRunBar::get_running_preset() const {
	return running_preset;
}

ProcessID EditorRunBar::has_child_process(ProcessID p_pid) const {
	return editor_run.has_child_process(p_pid);
}

void EditorRunBar::stop_child_process(ProcessID p_pid) {
	if (!has_child_process(p_pid)) {
		return;
	}

	editor_run.stop_child_process(p_pid);
	if (!editor_run.get_child_process_count()) { // All children stopped. Closing.
		stop_playing();
	}
}

ProcessID EditorRunBar::get_current_process() const {
	return editor_run.get_current_process();
}

void EditorRunBar::set_movie_maker_enabled(bool p_enabled) {
	movie_maker_enabled = p_enabled;
	main_play_popup->set_item_checked(main_play_popup->get_item_index(PLAY_POPUP_RUN_OPTIONS_MOVIE_MAKER_ENABLED), p_enabled);
}

bool EditorRunBar::is_movie_maker_enabled() const {
	return movie_maker_enabled;
}

void EditorRunBar::update_profiler_autostart_indicator() {
	bool profiler_active = EditorSettings::get_singleton()->get_project_metadata("debug_options", "autostart_profiler", false);
	bool visual_profiler_active = EditorSettings::get_singleton()->get_project_metadata("debug_options", "autostart_visual_profiler", false);
	bool network_profiler_active = EditorSettings::get_singleton()->get_project_metadata("debug_options", "autostart_network_profiler", false);
	bool any_profiler_active = profiler_active | visual_profiler_active | network_profiler_active;
	any_profiler_active &= !Engine::get_singleton()->is_recovery_mode_hint();
	profiler_autostart_indicator->set_visible(any_profiler_active);
	if (any_profiler_active) {
		String tooltip = TTR("Autostart is enabled for the following profilers, which can have a performance impact:");
		if (profiler_active) {
			tooltip += "\n- " + TTR("Profiler");
		}
		if (visual_profiler_active) {
			tooltip += "\n- " + TTR("Visual Profiler");
		}
		if (network_profiler_active) {
			tooltip += "\n- " + TTR("Network Profiler");
		}
		tooltip += "\n\n" + TTR("Click to open the first profiler for which autostart is enabled.");
		profiler_autostart_indicator->set_tooltip_text(tooltip);
	}
}

HBoxContainer *EditorRunBar::get_buttons_container() {
	return main_hbox;
}

void EditorRunBar::_bind_methods() {
	ADD_SIGNAL(MethodInfo("play_pressed"));
	ADD_SIGNAL(MethodInfo("stop_pressed"));
}

Error EditorRunBar::start_run_native(int p_platform, int p_device) {
	if (!EditorNode::get_singleton()->ensure_main_scene(true)) {
		return OK;
	}

	Ref<EditorExportPlatform> eep = EditorExport::get_singleton()->get_export_platform(p_platform);
	ERR_FAIL_COND_V(eep.is_null(), ERR_UNAVAILABLE);

	Ref<EditorExportPreset> preset = EditorExport::get_singleton()->get_runnable_preset_for_platform(eep);
	if (preset.is_null()) {
		EditorNode::get_singleton()->show_warning(TTR("No runnable export preset found for this platform.\nPlease add a runnable preset in the Export menu or define an existing preset as runnable."));
		return ERR_UNAVAILABLE;
	}

	String architecture = eep->get_device_architecture(p_device);
	if (!run_native_confirmed && !architecture.is_empty()) {
		String preset_arch = "architectures/" + architecture;
		bool is_arch_enabled = preset->get(preset_arch);

		if (!is_arch_enabled) {
			run_native_confirm->set_text(vformat(TTR("Warning: The CPU architecture \"%s\" is not active in your export preset.\n\nRun \"Remote Deploy\" anyway?"), architecture));
			run_native_confirm->popup_centered();
			return OK;
		}
	}
	run_native_confirmed = false;

	preset->update_value_overrides();

	if (eep->is_option_runnable(p_device)) {
		EditorNode::get_singleton()->try_autosave();
		stop_playing();
		if (EditorSettings::get_singleton()->get_project_metadata("debug_options", "run_deploy_remote_debug", true)) {
			if (EditorNode::get_singleton()->call_build()) {
				EditorDebuggerNode::get_singleton()->start(preset->get_platform()->get_debug_protocol());
				editor_run.run_native_notify();
			}
		}
	}

	BitField<EditorExportPlatform::DebugFlags> flags = 0;

	bool deploy_debug_remote = EditorSettings::get_singleton()->get_project_metadata("debug_options", "run_deploy_remote_debug", true);
	bool deploy_dumb = EditorSettings::get_singleton()->get_project_metadata("debug_options", "run_file_server", false);
	bool debug_collisions = EditorSettings::get_singleton()->get_project_metadata("debug_options", "run_debug_collisions", false);
	bool debug_navigation = EditorSettings::get_singleton()->get_project_metadata("debug_options", "run_debug_navigation", false);

	if (deploy_debug_remote) {
		flags.set_flag(EditorExportPlatform::DEBUG_FLAG_REMOTE_DEBUG);
	}
	if (deploy_dumb) {
		flags.set_flag(EditorExportPlatform::DEBUG_FLAG_DUMB_CLIENT);
	}
	if (debug_collisions) {
		flags.set_flag(EditorExportPlatform::DEBUG_FLAG_VIEW_COLLISIONS);
	}
	if (debug_navigation) {
		flags.set_flag(EditorExportPlatform::DEBUG_FLAG_VIEW_NAVIGATION);
	}

	eep->clear_messages();
	Error err = eep->run(preset, p_device, flags);
	native_result_dialog_log->clear();
	if (eep->fill_log_messages(native_result_dialog_log, err)) {
		if (eep->get_worst_message_type() >= EditorExportPlatform::EXPORT_MESSAGE_ERROR) {
			native_result_dialog->popup_centered_ratio(0.5);
		}
	}
	return err;
}

EditorRunBar::EditorRunBar() {
	singleton = this;

	outer_hbox = memnew(HBoxContainer);
	add_child(outer_hbox);

	// Use a button for the indicator since it comes with a background panel and pixel perfect centering of an icon.
	profiler_autostart_indicator = memnew(Button);
	profiler_autostart_indicator->set_tooltip_auto_translate_mode(AUTO_TRANSLATE_MODE_DISABLED);
	profiler_autostart_indicator->set_icon_alignment(HORIZONTAL_ALIGNMENT_CENTER);
	profiler_autostart_indicator->set_focus_mode(FOCUS_ACCESSIBILITY);
	profiler_autostart_indicator->set_theme_type_variation("ProfilerAutostartIndicator");
	profiler_autostart_indicator->connect(SceneStringName(pressed), callable_mp(this, &EditorRunBar::_profiler_autostart_indicator_pressed));
	outer_hbox->add_child(profiler_autostart_indicator);
	update_profiler_autostart_indicator();

	main_panel = memnew(PanelContainer);
	outer_hbox->add_child(main_panel);

	main_hbox = memnew(HBoxContainer);
	main_panel->add_child(main_hbox);

	if (Engine::get_singleton()->is_recovery_mode_hint()) {
		recovery_mode_popup = memnew(AcceptDialog);
		recovery_mode_popup->set_min_size(Size2(550, 70) * EDSCALE);
		recovery_mode_popup->set_title(TTR("Recovery Mode"));
		recovery_mode_popup->set_text(
				TTR("Godot opened the project in Recovery Mode, which is a special mode that can help recover projects that crash the engine upon initialization. The following features have been temporarily disabled:") +
				String::utf8("\n\n•  ") + TTR("Tool scripts") +
				String::utf8("\n•  ") + TTR("Editor plugins") +
				String::utf8("\n•  ") + TTR("GDExtension addons") +
				String::utf8("\n•  ") + TTR("Automatic scene restoring") +
				String::utf8("\n\n") + TTR("If the project cannot be opened outside of this mode, then it's very likely any of these components is preventing this project from launching. This mode is intended only for basic editing to troubleshoot such issues, and therefore it is not possible to run a project in this mode.") +
				String::utf8("\n\n") + TTR("To disable Recovery Mode, reload the project by pressing the Reload button next to the Recovery Mode banner, or by reopening the project normally."));
		recovery_mode_popup->set_autowrap(true);
		add_child(recovery_mode_popup);

		recovery_mode_reload_button = memnew(Button);
		main_hbox->add_child(recovery_mode_reload_button);
		recovery_mode_reload_button->set_theme_type_variation("RunBarButton");
		recovery_mode_reload_button->set_focus_mode(Control::FOCUS_ACCESSIBILITY);
		recovery_mode_reload_button->set_tooltip_text(TTR("Disable recovery mode and reload the project."));
		recovery_mode_reload_button->connect(SceneStringName(pressed), callable_mp(this, &EditorRunBar::recovery_mode_reload_project));

		recovery_mode_panel = memnew(PanelContainer);
		main_hbox->add_child(recovery_mode_panel);

		recovery_mode_button = memnew(Button);
		recovery_mode_panel->add_child(recovery_mode_button);
		recovery_mode_button->set_theme_type_variation("RunBarButton");
		recovery_mode_button->set_focus_mode(Control::FOCUS_ACCESSIBILITY);
		recovery_mode_button->set_text(TTR("Recovery Mode"));
		recovery_mode_button->set_tooltip_text(TTR("Recovery Mode is enabled. Click for more details."));
		recovery_mode_button->connect(SceneStringName(pressed), callable_mp(this, &EditorRunBar::recovery_mode_show_dialog));

		return;
	}

	// here needs to add the dropdown for options for main play button
	main_play_hbox = memnew(HBoxContainer);
	main_hbox->add_child(main_play_hbox);

	main_play_menu_button = memnew(MenuButton);
	main_play_hbox->add_child(main_play_menu_button);
	main_play_menu_button->set_text(TTR("Main Scene"));

	main_play_popup = main_play_menu_button->get_popup();
	main_play_popup->connect(SceneStringName(id_pressed), callable_mp(this, &EditorRunBar::_on_popup_menu_id_pressed));
	main_play_popup->set_hide_on_item_selection(false);
	main_play_popup->set_hide_on_checkable_item_selection(false);

	// here need to modify the play button to run what is selected above
	play_button = memnew(Button);
	main_play_hbox->add_child(play_button);
	play_button->set_theme_type_variation("RunBarButton");
	play_button->set_toggle_mode(true);
	play_button->set_focus_mode(Control::FOCUS_ACCESSIBILITY);
	ED_SHORTCUT_AND_COMMAND("editor/run_project", TTRC("Run Project"), Key::F5);
	ED_SHORTCUT_OVERRIDE("editor/run_project", "macos", KeyModifierMask::META | Key::B);
	play_button->set_shortcut(ED_GET_SHORTCUT("editor/run_project"));
	play_button->set_tooltip_text(TTRC("Run the project's main scene."));
	play_button->connect(SceneStringName(pressed), callable_mp(this, &EditorRunBar::play_current_preset));

	pause_button = memnew(Button);
	main_play_hbox->add_child(pause_button);
	pause_button->set_theme_type_variation("RunBarButton");
	pause_button->set_toggle_mode(true);
	pause_button->set_focus_mode(Control::FOCUS_ACCESSIBILITY);
	pause_button->set_tooltip_text(TTRC("Pause the running project's execution for debugging."));
	pause_button->set_disabled(true);

	ED_SHORTCUT("editor/pause_running_project", TTRC("Pause Running Project"), Key::F7);
	ED_SHORTCUT_OVERRIDE("editor/pause_running_project", "macos", KeyModifierMask::META | KeyModifierMask::CTRL | Key::Y);
	pause_button->set_shortcut(ED_GET_SHORTCUT("editor/pause_running_project"));

	stop_button = memnew(Button);
	main_play_hbox->add_child(stop_button);
	stop_button->set_theme_type_variation("RunBarButton");
	stop_button->set_focus_mode(Control::FOCUS_ACCESSIBILITY);
	stop_button->set_tooltip_text(TTRC("Stop the currently running project."));
	stop_button->set_disabled(true);
	stop_button->connect(SceneStringName(pressed), callable_mp(this, &EditorRunBar::stop_playing));

	ED_SHORTCUT("editor/stop_running_project", TTRC("Stop Running Project"), Key::F8);
	ED_SHORTCUT_OVERRIDE("editor/stop_running_project", "macos", KeyModifierMask::META | Key::PERIOD);
	stop_button->set_shortcut(ED_GET_SHORTCUT("editor/stop_running_project"));

	run_native_confirm = memnew(ConfirmationDialog);
	add_child(run_native_confirm);
	run_native_confirm->connect(SceneStringName(confirmed), callable_mp(this, &EditorRunBar::_confirm_run_native));

	native_result_dialog = memnew(AcceptDialog);
	native_result_dialog->set_title(TTR("Project Run"));
	native_result_dialog_log = memnew(RichTextLabel);
	native_result_dialog_log->set_custom_minimum_size(Size2(300, 80) * EDSCALE);
	native_result_dialog->add_child(native_result_dialog_log);
	add_child(native_result_dialog);

	ED_SHORTCUT("remote_deploy/deploy_to_device_1", TTRC("Deploy to First Device in List"), KeyModifierMask::SHIFT | Key::F5);
	ED_SHORTCUT_OVERRIDE("remote_deploy/deploy_to_device_1", "macos", KeyModifierMask::META | KeyModifierMask::SHIFT | Key::B);
	ED_SHORTCUT("remote_deploy/deploy_to_device_2", TTRC("Deploy to Second Device in List"));
	ED_SHORTCUT("remote_deploy/deploy_to_device_3", TTRC("Deploy to Third Device in List"));
	ED_SHORTCUT("remote_deploy/deploy_to_device_4", TTRC("Deploy to Fourth Device in List"));

	preset_hbox = memnew(HBoxContainer);
	main_hbox->add_child(preset_hbox);

	presets_menu_button = memnew(MenuButton);
	presets_menu_button->get_popup()->connect(SceneStringName(id_pressed), callable_mp(this, &EditorRunBar::_on_presets_menu_item_pressed));
	presets_menu_button->connect(SNAME("about_to_popup"), callable_mp(this, &EditorRunBar::_update_presets_menu_button));
	preset_hbox->add_child(presets_menu_button);

	run_preset_manager_dialog = memnew(RunPresetManagerDialog);
	run_preset_manager_dialog->connect("presets_changed", callable_mp(this, &EditorRunBar::_generate_presets_buttons), CONNECT_DEFERRED);
	add_child(run_preset_manager_dialog);
}
