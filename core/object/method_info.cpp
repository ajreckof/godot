/**************************************************************************/
/*  method_info.cpp                                                       */
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

#include "method_info.h"
#include "core/variant/struct.h"
#include "core/variant/typed_array.h"

PropertyInfo MethodInfo::return_val::from_variant(const Variant &p_variant) {
	return PropertyInfo(Struct<PropertyInfo>(p_variant));
}

Variant MethodInfo::return_val::to_variant(const PropertyInfo &p_value) {
	return Struct<PropertyInfo>(p_value);
}

List<PropertyInfo> MethodInfo::arguments::from_variant(const Variant &p_variant) {
	return (List<PropertyInfo>)TypedArray<Struct<PropertyInfo>>(p_variant);
}

Variant MethodInfo::arguments::to_variant(const List<PropertyInfo> &p_value) {
	return TypedArray<Struct<PropertyInfo>>(&p_value);
}
