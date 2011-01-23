/*
control_settings_menu_mode.h: Header for the mode classes for configuring controls.

Copyright 2011 David Simon <david.mike.simon@gmail.com>

This file is part of Orbit Ribbon.

Orbit Ribbon is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Orbit Ribbon is distributed in the hope that it will be awesome,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Orbit Ribbon.  If not, see http://www.gnu.org/licenses/
*/

#ifndef ORBIT_RIBBON_CONTROL_SETTINGS_MENU_MODE_H
#define ORBIT_RIBBON_CONTROL_SETTINGS_MENU_MODE_H

#include <boost/array.hpp>

#include "autoxsd/save.h"
#include "gui.h"
#include "input.h"
#include "mode.h"

struct ActionDesc {
  std::string name;
  bool can_set_kbd_mouse;
  bool can_set_gamepad;
  ActionDesc(const std::string& n, bool km, bool g) : name(n), can_set_kbd_mouse(km), can_set_gamepad(g) {}
  virtual ~ActionDesc() {}
};

struct AxisActionDesc : public ActionDesc {
  typedef ORSave::AxisBoundAction::value_type AAction;
  AAction action;
  AxisActionDesc(AAction a, const std::string& n, bool km = true, bool g = true) : ActionDesc(n, km, g), action(a) {}
};

struct ButtonActionDesc : public ActionDesc {
  typedef ORSave::ButtonBoundAction::value_type BAction;
  BAction action;
  ButtonActionDesc(BAction a, const std::string& n, bool km = true, bool g = true) : ActionDesc(n, km, g), action(a) {}
};

struct BindingDesc {
  const ActionDesc* action_desc;
  ORSave::InputDeviceNameType::value_type dev;
  BindingDesc(const ActionDesc& a, ORSave::InputDeviceNameType::value_type d) :
    action_desc(&a), dev(d)
  {}
};

class ControlSettingsMenuMode : public Mode {
  private:
    GUI::Grid _grid;
    bool _at_main_menu;

    std::map<GUI::Widget*, BindingDesc> _binding_widgets;

    void add_row(const ActionDesc& action, const Channel& chan);

    static const boost::array<AxisActionDesc, 6> AXIS_BOUND_ACTION_NAMES;
    static const boost::array<ButtonActionDesc, 5> BUTTON_BOUND_ACTION_NAMES;

  public:
    ControlSettingsMenuMode(bool at_main_menu);
    
    bool execute_after_lower_mode() { return !_at_main_menu; }
    bool simulation_disabled() { return true; }
    bool mouse_cursor_enabled() { return true; }

    bool handle_input();
    void draw_3d_far(bool top);
    void draw_2d(bool top);

    void now_at_top();
};

class RebindingDialogMenuMode : public Mode {
  protected:
    std::string _old_value;
    const BindingDesc* _binding_desc;

    std::string _title, _instruction;
    bool _axis_mode;

  public:
    RebindingDialogMenuMode(const std::string& old_value, const BindingDesc* binding_desc);
    bool execute_after_lower_mode() { return true; }
    bool simulation_disabled() { return true; }
    bool mouse_cursor_enabled() { return false; }

    bool handle_input();
    void draw_2d(bool top);
};

#endif
