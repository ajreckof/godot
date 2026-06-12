/**************************************************************************/
/*  run_preset_button.cpp                                                 */
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

#include "run_preset_button.h"

#include "core/object/callable_mp.h"
#include "editor/editor_string_names.h"
#include "editor/run/run_preset.h"
#include "scene/gui/popup_menu.h"
#include "servers/display/display_server.h"
#include "servers/rendering/rendering_server.h"

void RunPresetButton::_on_button_pressed() {
	if (is_running()) {
		EditorRunBar::get_singleton()->resume_running_preset();
	} else if (preset->get_destination() == RunDestination::DESTINATION_REMOTE && (preset->get_remote_platform_id() == -1 || preset->get_remote_device_id() == -1 || preset->get_select_remote_platform_id() || preset->get_select_remote_device_id())) {
		show_popup();
	} else {
		// Otherwise, start running immediately with the preset's settings.
		EditorRunBar::get_singleton()->play_preset(preset);
	}
}

void RunPresetButton::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_POSTINITIALIZE: {
			connect("toggled", callable_mp(this, &RunPresetButton::_on_button_pressed).unbind(1));
			popup->connect("id_pressed", callable_mp(this, &RunPresetButton::_on_popup_id_pressed));
			EditorRunBar::get_singleton()->connect("play_pressed", callable_mp(this, &RunPresetButton::_update_button), CONNECT_DEFERRED);
			EditorRunBar::get_singleton()->connect("stop_pressed", callable_mp(this, &RunPresetButton::_update_button));
			break;
		}
		case NOTIFICATION_READY:
		case NOTIFICATION_THEME_CHANGED: {
			_update_button();
			break;
		}
		case NOTIFICATION_ENTER_TREE: {
			get_parent()->connect(SceneStringName(theme_changed), callable_mp(this, &RunPresetButton::_on_parent_theme_changed));
			_on_parent_theme_changed();
			break;
		}
	}
}

void RunPresetButton::_on_popup_id_pressed(int p_id) {
	preset->set_option(p_id);
	EditorRunBar::get_singleton()->play_preset(preset);
}
bool RunPresetButton::is_running() const {
	return EditorRunBar::get_singleton()->get_running_preset() == preset;
}

void RunPresetButton::_update_button() {
	if (preset.is_null()) {
		return;
	}
	if (!is_ready()) {
		return;
	}
	set_visible(preset->is_pinned());
	set_pressed_no_signal(is_running());
	if (is_running()) {
		set_button_icon(get_theme_icon(SNAME("Reload"), EditorStringName(EditorIcons)));
	} else {
		set_button_icon(preset->get_icon());
	}
	set_tooltip_text(preset->get_name());
}

void RunPresetButton::set_preset(const Ref<RunPreset> &p_preset) {
	if (preset == p_preset) {
		return;
	}
	if (preset.is_valid()) {
		preset->disconnect("changed", callable_mp(this, &RunPresetButton::_update_button));
		preset->disconnect("changed", callable_mp(this, &RunPresetButton::_update_popup));
	}
	preset = p_preset;
	if (preset.is_null()) {
		set_disabled(true);
	} else {
		preset->connect("changed", callable_mp(this, &RunPresetButton::_update_button));
		preset->connect("changed", callable_mp(this, &RunPresetButton::_update_popup));
		set_disabled(false);
		_update_button();
		_update_popup();
	}
}

void RunPresetButton::_update_popup() {
	popup->clear();
	if (preset.is_null()) {
		return;
	}
	for (RunPresetOptions option : preset->get_options()) {
		popup->add_icon_item(option.icon, option.name, option.id);
	}
}

void RunPresetButton::_on_parent_theme_changed() {
	add_theme_constant_override("icon_max_width", get_theme_constant("class_icon_size", EditorStringName(Editor)));
}

void RunPresetButton::show_popup() {
	if (!get_viewport()) {
		return;
	}

	Rect2 rect = get_screen_rect();
	rect.position.y += rect.size.height;
	if (get_viewport()->is_embedding_subwindows() && popup->get_force_native()) {
		Transform2D xform = get_viewport()->get_popup_base_transform_native();
		rect = xform.xform(rect);
	}
	Rect2i scr_usable = DisplayServer::get_singleton()->screen_get_usable_rect(get_window()->get_current_screen());
	Size2i max_size;
	if (scr_usable.has_area()) {
		real_t max_h = scr_usable.get_end().y - rect.position.y;
		if (max_h >= 4 * rect.size.height) {
			max_size = Size2(RS::get_singleton()->get_maximum_viewport_size().width, max_h);
		}
	}
	popup->set_max_size(max_size);
	if (is_layout_rtl()) {
		rect.position.x += rect.size.width - popup->get_size().width;
	}
	popup->set_position(rect.position);

	// If not triggered by the mouse, start the popup with its first enabled item focused.
	if (!_was_pressed_by_mouse()) {
		for (int i = 0; i < popup->get_item_count(); i++) {
			if (!popup->is_item_disabled(i)) {
				popup->set_focused_item(i);
				break;
			}
		}
	}

	popup->popup();
}

RunPresetButton::RunPresetButton() {
	popup = memnew(PopupMenu);
	add_child(popup);
	popup->hide();

	set_toggle_mode(true);
	set_theme_type_variation(SceneStringName(FlatButton));
	set_process(true);
}
