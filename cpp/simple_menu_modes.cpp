/*
simple_menu_modes.cpp: Implementation for the various simple menu mode classes.
These handle the menu screens used to start the game, choose a level, etc.

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

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <cstring>

#include "simple_menu_modes.h"

#include "app.h"
#include "autoinfo/version.h"
#include "background.h"
#include "debug.h"
#include "display.h"
#include "font.h"
#include "gameplay_mode.h"
#include "gameobj.h"
#include "globals.h"

#include "gui.h"
#include "input.h"
#include "interpolation.h"
#include "interp_mode.h"
#include "options_menu_mode.h"
#include "ore.h"
#include "saving.h"

void SimpleMenuMode::add_entry(const std::string& name, const std::string& label) {
  boost::shared_ptr<GUI::Widget> button(new GUI::Button(label));
  _menu.add_widget(button);
  _entry_names[&*button] = name;
}

SimpleMenuMode::SimpleMenuMode(bool draw_background, int menu_width, int btn_height, int padding, Vector center_offset) :
  _draw_background(draw_background),
  _menu(menu_width, btn_height, padding, center_offset)
{}

bool SimpleMenuMode::handle_input() {
  _menu.process();
  
  if (Input::get_button_ch(ORSave::ButtonBoundAction::Cancel).matches_frame_events()) {
    handle_menu_selection("CANCEL");
  } else {
    BOOST_FOREACH(SDL_Event& e, Globals::frame_events) {
      if (e.type == SDL_USEREVENT && e.user.code == GUI::WIDGET_CLICKED) {
        std::map<GUI::Widget*, std::string>::iterator i = _entry_names.find((GUI::Widget*)(e.user.data1));
        if (i != _entry_names.end()) {
          handle_menu_selection(i->second);
        }
      }
    }
  }
  
  return true;
}

void SimpleMenuMode::draw_3d_far(bool top __attribute__ ((unused))) {
  if (_draw_background) {
    Globals::bg->draw_starbox();
    Globals::bg->draw_objects();
  }
}

void SimpleMenuMode::draw_2d(bool top) {
  if (top) {
    _menu.draw(execute_after_lower_mode());
  }
}

MainMenuMode::MainMenuMode() :
  SimpleMenuMode(true, 180, 22, 8, Vector(0, 0.3)),
  _title_tex(GLOOTexture::load("title.png"))
{
  add_entry("play", "Play");
  add_entry("credits", "Credits");
  add_entry("options", "Options");
  add_entry("quit", "Quit");
  
  _camera.pos = Point(0, -5e9, -7e10);
}

void MainMenuMode::handle_menu_selection(const std::string& item) {
  if (item == "play") {
    ;
    Globals::mode_stack->next_frame_push_mode(boost::shared_ptr<Mode>(
      new InterpolationMode<CosineInterpolator>(1000, boost::shared_ptr<Mode>(
      new AreaSelectMenuMode()
    ))));
  } else if (item == "options") {
    Globals::mode_stack->next_frame_push_mode(boost::shared_ptr<Mode>(new OptionsMenuMode(true)));
  } else if (item == "quit" or item == "CANCEL") {
    Globals::mode_stack->next_frame_pop_mode();
  }
}

void MainMenuMode::draw_2d(bool top __attribute__ ((unused))) {
  SimpleMenuMode::draw_2d(top);
  
  Point title_pos(
    Display::get_screen_width()/2 - _title_tex->get_width()/2,
    Display::get_screen_height()*0.25 - _title_tex->get_height()/2
  );
  _title_tex->draw_2d(title_pos); 

  float font_height = 13.0;
  float buf = 3;
  Globals::sys_font->draw(Point(buf, Display::get_screen_height() - font_height*1 - buf), font_height, std::string("Version: ") + APP_VERSION);
  Globals::sys_font->draw(Point(buf, Display::get_screen_height() - font_height*2 - buf), font_height, std::string("Compiled: ") + BUILD_DATE);
  if (std::strlen(COMMIT_HASH) > 0) {
    Globals::sys_font->draw(Point(buf, Display::get_screen_height() - font_height*3 - buf), font_height, std::string("Commit Hash: ") + COMMIT_HASH);
    Globals::sys_font->draw(Point(buf, Display::get_screen_height() - font_height*4 - buf), font_height, std::string("Commit Date: ") + COMMIT_DATE);
  }
}

AreaSelectMenuMode::AreaSelectMenuMode() : SimpleMenuMode(true, 300, 22, 8) {
  const ORE1::PkgDescType* desc = &Globals::ore->get_pkg_desc();
  unsigned int n = 1;
  for (ORE1::PkgDescType::area_const_iterator i = desc->area().begin(); i != desc->area().end(); ++i) {
    const std::string n_as_str = boost::lexical_cast<std::string>(n);
    const ORE1::AreaType* area = &(*i);
    add_entry(n_as_str, n_as_str + ". " + area->niceName());
    ++n;
  }
  
  add_entry("back", "Back");
  
  _camera.pos = Point(0, 1.7e11, 0);
  _camera.up = Point(1, 0, 0);
}

void AreaSelectMenuMode::handle_menu_selection(const std::string& item) {
  if (item == "back" or item == "CANCEL") {
    Globals::mode_stack->next_frame_pop_mode();
  } else {
    unsigned int area_num = boost::lexical_cast<unsigned int>(item);
    App::load_mission(area_num, 0);
    Globals::mode_stack->next_frame_push_mode(boost::shared_ptr<Mode>(
      new InterpolationMode<Reverser<ExponentialInterpolation<4>::Interp>::Reversed>(1000, boost::shared_ptr<Mode>(
      new MissionSelectMenuMode(area_num)
    ))));
  }
}

MissionSelectMenuMode::MissionSelectMenuMode(unsigned int area_num) : SimpleMenuMode(true, 450, 22, 8, Vector(0.1, 0.2)), _area_num(area_num) {
  const ORE1::PkgDescType* desc = &Globals::ore->get_pkg_desc();
  const ORE1::AreaType* area = &(desc->area()[area_num-1]);
  unsigned int n = 1;
  for (ORE1::AreaType::mission_const_iterator i = area->mission().begin(); i != area->mission().end(); ++i) {
    const std::string n_as_str = boost::lexical_cast<std::string>(n);
    const ORE1::MissionType* mission = &*i;
    add_entry(n_as_str, n_as_str + ". " + mission->niceName());
    ++n;
  }
  
  add_entry("back", "Back");
  
  _camera.up = Point(1, 0, 0);
}

void MissionSelectMenuMode::draw_3d_far(bool top) {
  GLOOPushedMatrix pm;
  Vector offset(Globals::bg->to_center_from_game_origin());
  if (top) {
    {
      GLOOPushedMatrix pm;
      glTranslatef(offset.x, offset.y, offset.z);
      SimpleMenuMode::draw_3d_far(top);
    }
    for (GOMap::iterator i = Globals::gameobjs.begin(); i != Globals::gameobjs.end(); ++i) {
      i->second->draw(false);
    }
  } else {
    SimpleMenuMode::draw_3d_far(top);
    glTranslatef(-offset.x, -offset.y, -offset.z);
    for (GOMap::iterator i = Globals::gameobjs.begin(); i != Globals::gameobjs.end(); ++i) {
      i->second->draw(false);
    }
  }
}

const GLOOCamera* MissionSelectMenuMode::get_camera(bool top) {
  _camera.pos = Point(0, 1e4, 0);
  if (top) {
    // This is where we transition from thinking of the star as being origin to thinking of game bubble as origin
    _camera.tgt = Point(0,0,0);
  } else {
    Vector offset(-Globals::bg->to_center_from_game_origin());
    _camera.pos += offset;
    _camera.tgt = offset;
  }
  return &_camera;
}

void MissionSelectMenuMode::handle_menu_selection(const std::string& item) {
  if (item == "back" or item == "CANCEL") {
    Globals::mode_stack->next_frame_pop_mode();
  } else {
    unsigned int mission_num = boost::lexical_cast<unsigned int>(item);
    App::load_mission(_area_num, mission_num);
    Globals::mode_stack->next_frame_push_mode(boost::shared_ptr<Mode>(
      new InterpolationMode<Reverser<ExponentialInterpolation<4>::Interp>::Reversed>(5000, boost::shared_ptr<Mode>(
      new GameplayMode()
    ))));
  }
}

PauseMenuMode::PauseMenuMode() : SimpleMenuMode(false, 150, 30, 15) {
  add_entry("resume", "Resume Game");
  add_entry("options", "Options");
  add_entry("quit", "Quit Mission");
}

bool PauseMenuMode::handle_input() {
  if (Input::get_button_ch(ORSave::ButtonBoundAction::Pause).matches_frame_events()) {
    handle_menu_selection("CANCEL");
    return true;
  } else {
    return SimpleMenuMode::handle_input();
  }
}

void PauseMenuMode::handle_menu_selection(const std::string& item) {
  if (item == "resume" or item == "CANCEL") {
    Globals::mode_stack->next_frame_pop_mode();
  } else if (item == "options") {
    Globals::mode_stack->next_frame_push_mode(boost::shared_ptr<Mode>(new OptionsMenuMode(false)));
  } else if (item == "quit") {
    // Pop both the PauseMenuMode and the GameplayMode underneath it
    Globals::mode_stack->next_frame_pop_mode();
    Globals::mode_stack->next_frame_pop_mode();
  }
}

PostMissionMenuMode::PostMissionMenuMode(bool won) : SimpleMenuMode(false, 400, 40, 30), _won(won) {
  add_entry("continue", won ? "Rios made it!" : "Didn't make it...");
}

void PostMissionMenuMode::handle_menu_selection(const std::string& item) {
  if (item == "continue") {
    // Pop both this mode and the GameplayMode, returning us to the pre-mission screen
    Globals::mode_stack->next_frame_pop_mode();
    Globals::mode_stack->next_frame_pop_mode();
  }
}
