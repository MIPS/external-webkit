/*
 * Copyright (c) 2010 The Chromium Authors. All rights reserved.
 * Copyright 2010, The Android Open Source Project
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "FormFieldAndroid.h"

#include "ChromiumIncludes.h"
#include "Element.h"
#include "HTMLFormControlElement.h"
#include "HTMLInputElement.h"
#include "HTMLNames.h"
#include "HTMLOptionElement.h"
#include "HTMLSelectElement.h"
#include "StringUtils.h"
#include <wtf/Vector.h>

using WebCore::Element;
using WebCore::HTMLFormControlElement;
using WebCore::HTMLInputElement;
using WebCore::HTMLOptionElement;
using WebCore::HTMLSelectElement;

using namespace WebCore::HTMLNames;

// TODO: This file is taken from chromium/webkit/glue/form_field.cc and
// customised to use WebCore types rather than WebKit API types. It would
// be nice and would ease future merge pain if the two could be combined.

namespace webkit_glue {

FormField::FormField()
    : max_length(0),
      is_autofilled(false) {
}

// TODO: This constructor should probably be deprecated and the
// functionality moved to FormManager.
FormField::FormField(const HTMLFormControlElement& element)
    : max_length(0),
      is_autofilled(false) {
    name = nameForAutofill(element);

    // TODO: Extract the field label.  For now we just use the field
    // name.
    label = name;

    form_control_type = formControlType(element);
    if (form_control_type == kText) {
        const HTMLInputElement& input_element = static_cast<const HTMLInputElement&>(element);
        value = WTFStringToString16(input_element.value());
        max_length = input_element.size();
        is_autofilled = input_element.isAutofilled();
    } else if (form_control_type == kSelectOne) {
        const HTMLSelectElement& const_select_element = static_cast<const HTMLSelectElement&>(element);
        HTMLSelectElement& select_element = const_cast<HTMLSelectElement&>(const_select_element);
        value = WTFStringToString16(select_element.value());

        // For select-one elements copy option strings.
        WTF::Vector<Element*> list_items = select_element.listItems();
        option_strings.reserve(list_items.size());
        for (size_t i = 0; i < list_items.size(); ++i) {
            if (list_items[i]->hasTagName(optionTag))
                option_strings.push_back(WTFStringToString16(static_cast<HTMLOptionElement*>(list_items[i])->value()));
        }
    }

    TrimWhitespace(value, TRIM_LEADING, &value);
}

FormField::FormField(const string16& _label, const string16& _name, const string16& _value, const string16& _form_control_type, int _max_length, bool _is_autofilled)
    : label(_label),
      name(_name),
      value(_value),
      form_control_type(_form_control_type),
      max_length(_max_length),
      is_autofilled(_is_autofilled) {
}

FormField::~FormField() {
}

bool FormField::operator==(const FormField& field) const {
    // A FormField stores a value, but the value is not part of the identity of
    // the field, so we don't want to compare the values.
    return (label == field.label &&
            name == field.name &&
            form_control_type == field.form_control_type &&
            max_length == field.max_length);
}

bool FormField::operator!=(const FormField& field) const {
    return !operator==(field);
}

bool FormField::StrictlyEqualsHack(const FormField& field) const {
    return (label == field.label &&
            name == field.name &&
            value == field.value &&
            form_control_type == field.form_control_type &&
            max_length == field.max_length);
}

std::ostream& operator<<(std::ostream& os, const FormField& field) {
    return os
            << UTF16ToUTF8(field.label)
            << " "
            << UTF16ToUTF8(field.name)
            << " "
            << UTF16ToUTF8(field.value)
            << " "
            << UTF16ToUTF8(field.form_control_type)
            << " "
            << field.max_length;
}

}  // namespace webkit_glue
