/**************************************************************************/
/*  run_preset_manager.h                                                  */
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

#include "scene/gui/dialogs.h"

class RunPreset;
class EditorInspector;
class ItemList;

class RunPresetManagerDialog : public AcceptDialog {
	GDCLASS(RunPresetManagerDialog, AcceptDialog);

private:
	Vector<Ref<RunPreset>> presets;
	ItemList *presets_list;
	TypedArray<Dictionary> default_presets;
	EditorInspector *preset_inspector;
	Dictionary new_preset;

	void _on_add_pressed();
	void _on_remove_pressed();
	void _on_restore_pressed();
	void _on_preset_selected(int p_index);
	void _on_preset_property_edited();
	void _update_preset_list();

	void save_presets();

protected:
	static void _bind_methods();
	void _notification(int p_what);

	void _on_parent_theme_changed();

public:
	Vector<Ref<RunPreset>> get_presets() const { return presets; }

	RunPresetManagerDialog();
};
