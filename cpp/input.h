/*
input.h: Header for input-management classes.
These classes are responsible for taking input from the keyboard and gamepad, and binding input to game actions.

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

#ifndef ORBIT_RIBBON_INPUT_H
#define ORBIT_RIBBON_INPUT_H

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <map>
#include <SDL/SDL.h>
#include <string>
#include <vector>

#include "autoxsd/save.h"

class Input;

const unsigned int CHANNEL_DESC_TYPE_KEYBOARD = 1;
const unsigned int CHANNEL_DESC_TYPE_MOUSE = 2;
const unsigned int CHANNEL_DESC_TYPE_GAMEPAD = 4;
const unsigned int CHANNEL_DESC_TYPE_ALL = CHANNEL_DESC_TYPE_KEYBOARD | CHANNEL_DESC_TYPE_MOUSE | CHANNEL_DESC_TYPE_GAMEPAD;

class Channel : boost::noncopyable {
  public:
    virtual bool is_on() const =0;
    virtual bool is_partially_on() const;
    virtual bool matches_frame_events() const =0;
    virtual float get_value() const =0;
    virtual void set_neutral();
    virtual bool is_null() const;
    virtual std::string desc(unsigned int desc_type = CHANNEL_DESC_TYPE_ALL) const =0;
};

class NullChannel : public Channel {
  private:
    NullChannel() {}
    
    friend class Input;
  
  public:
    bool is_on() const;
    bool matches_frame_events() const;
    float get_value() const;
    bool is_null() const;
    std::string desc(unsigned int desc_type = CHANNEL_DESC_TYPE_ALL) const;
};

class ChannelSource : boost::noncopyable {
  protected:
    std::vector<boost::shared_ptr<Channel> > _channels;
    
  public:
    virtual void update() =0;
    virtual void set_neutral();
    
    std::vector<boost::shared_ptr<Channel> >& channels() { return _channels; }
    const std::vector<boost::shared_ptr<Channel> >& channels() const { return _channels; }
};

class KeyChannel;
class Keyboard : public ChannelSource {
  private:
    friend class Input;
    friend class KeyChannel;
    
    Uint8* _sdl_key_state; // Pointer to an array from SDL which can be indexed by SDLKey
    
    Keyboard();
  
  public:
    void update();
    const boost::shared_ptr<Channel> key_channel(SDLKey key) const;
};

class KeyChannel : public Channel {
  private:
    friend class Keyboard;
    
    Keyboard* _kbd;
    SDLKey _key;
    Uint8 _neutral_state;
    
    KeyChannel(Keyboard* kbd, SDLKey key);
  
  public:
    bool is_on() const;
    bool matches_frame_events() const;
    float get_value() const;
    void set_neutral();
    std::string desc(unsigned int desc_type = CHANNEL_DESC_TYPE_ALL) const;
};

class MouseButtonChannel;
class MouseMovementChannel;
class Mouse : public ChannelSource {
  private:
    friend class Input;
    friend class MouseButtonChannel;
    friend class MouseMovementChannel;
    
    int _rel_x;
    int _rel_y;
    int _btn_mask;
    
    // Key is axis/button number, value is index into _channels
    std::map<Uint8, unsigned int> _axis_map;
    std::map<Uint8, unsigned int> _button_map;
  
    Mouse();
  
  public:
    void update();
    const boost::shared_ptr<Channel> button_channel(Uint8 btn) const;
    const boost::shared_ptr<Channel> movement_channel(Uint8 axis_num) const;
};

class MouseButtonChannel : public Channel {
  private:
    friend class Mouse;
    
    Mouse* _mouse;
    Uint8 _btn;
    bool _neutral_state;
    
    MouseButtonChannel(Mouse* mouse, Uint8 btn);
  
  public:
    bool is_on() const;
    bool matches_frame_events() const;
    float get_value() const;
    void set_neutral();
    std::string desc(unsigned int desc_type = CHANNEL_DESC_TYPE_ALL) const;
};

class MouseMovementChannel : public Channel {
  private:
    friend class Mouse;
    
    Mouse* _mouse;
    Uint8 _axis;
    
    MouseMovementChannel(Mouse* mouse, Uint8 axis);
  
  public:
    bool is_on() const;
    bool matches_frame_events() const;
    float get_value() const;
    std::string desc(unsigned int desc_type = CHANNEL_DESC_TYPE_ALL) const;
};

class GamepadAxisChannel;
class GamepadButtonChannel;
class GamepadManager : public ChannelSource {
  private:
    friend class Input;
    friend class GamepadAxisChannel;
    friend class GamepadButtonChannel;
    
    std::vector<SDL_Joystick*> _gamepads;
    
    // Outer key is joystick number, inner key is axis/button number, inner value is index into _channels
    std::map<Uint8, std::map<Uint8, unsigned int> > _gamepad_axis_map;
    std::map<Uint8, std::map<Uint8, unsigned int> > _gamepad_button_map;

    GamepadManager();

  public:
    ~GamepadManager();

    void update();
    unsigned int get_num_gamepads() const { return _gamepads.size(); }
    unsigned int get_num_axes(Uint8 gamepad_num) const;
    unsigned int get_num_buttons(Uint8 gamepad_num) const;
    std::string get_first_gamepad_name() const { return std::string(SDL_JoystickName(0)); }
    const boost::shared_ptr<Channel> axis_channel(Uint8 gamepad_num, Uint8 axis_num) const;
    const boost::shared_ptr<Channel> button_channel(Uint8 gamepad_num, Uint8 button_num) const;
};

class GamepadAxisChannel : public Channel {
  private:
    friend class GamepadManager;
    
    GamepadManager* _gamepad_man;
    Uint8 _gamepad;
    Uint8 _axis;
    float _neutral_value;
    float _last_pseudo_frame_event_value;
    
    GamepadAxisChannel(GamepadManager* gamepad_man, Uint8 gamepad, Uint8 axis);
  
  public:
    bool is_on() const;
    bool matches_frame_events() const;
    float get_value() const;
    void set_neutral();
    std::string desc(unsigned int desc_type = CHANNEL_DESC_TYPE_ALL) const;
};

class GamepadButtonChannel : public Channel {
  private:
    friend class GamepadManager;
    
    GamepadManager* _gamepad_man;
    Uint8 _gamepad;
    Uint8 _button;
    Uint8 _neutral_state;
    
    GamepadButtonChannel(GamepadManager* gamepad_man, Uint8 gamepad, Uint8 button);
  
  public:
    bool is_on() const;
    bool matches_frame_events() const;
    float get_value() const;
    void set_neutral();
    std::string desc(unsigned int desc_type = CHANNEL_DESC_TYPE_ALL) const;
};

class InvertAxisChannel : public Channel {
  private:
    boost::shared_ptr<Channel> _chn;

  public:
    InvertAxisChannel(const boost::shared_ptr<Channel>& chn);

    bool is_on() const;
    bool matches_frame_events() const;
    float get_value() const;
    void set_neutral();
    std::string desc(unsigned int desc_type = CHANNEL_DESC_TYPE_ALL) const;
};

class PseudoAxisChannel : public Channel {
  private:
    boost::shared_ptr<Channel> _neg;
    boost::shared_ptr<Channel> _pos;
    bool _neg_invert, _pos_invert;
  
  public:
    PseudoAxisChannel(const boost::shared_ptr<Channel>& neg, const boost::shared_ptr<Channel>& pos, bool neg_invert, bool pos_invert);
    
    bool is_on() const;
    bool matches_frame_events() const;
    float get_value() const;
    void set_neutral();
    std::string desc(unsigned int desc_type = CHANNEL_DESC_TYPE_ALL) const;
};

class PseudoButtonChannel : public Channel {
  private:
    boost::shared_ptr<Channel> _chn;
  
  public:
    PseudoButtonChannel(const boost::shared_ptr<Channel>& chn);
    
    bool is_on() const;
    bool matches_frame_events() const;
    float get_value() const;
    void set_neutral();
    std::string desc(unsigned int desc_type = CHANNEL_DESC_TYPE_ALL) const;
};

class MultiChannel : public Channel {
  protected:
    std::vector<boost::shared_ptr<Channel> > _channels;
  
  public:
    void add_channel(const boost::shared_ptr<Channel>& ch) { _channels.push_back(ch); }
    
    bool is_on() const =0;
    bool is_partially_on() const =0;
    float get_value() const =0;
    void set_neutral();
    std::string desc(unsigned int desc_type = CHANNEL_DESC_TYPE_ALL) const =0;
};

class MultiOrChannel : public MultiChannel {
  public:
    bool is_on() const;
    bool matches_frame_events() const;
    bool is_partially_on() const;
    float get_value() const;
    std::string desc(unsigned int desc_type = CHANNEL_DESC_TYPE_ALL) const;
};

class MultiAndChannel : public MultiChannel {
  public:
    bool is_on() const;
    bool matches_frame_events() const;
    bool is_partially_on() const;
    float get_value() const;
    std::string desc(unsigned int desc_type = CHANNEL_DESC_TYPE_ALL) const;
};

class App;

class Input {
  private:
    static boost::shared_ptr<Channel> _null_channel;
    
    static boost::shared_ptr<Keyboard> _kbd;
    static boost::shared_ptr<Mouse> _mouse;
    static boost::shared_ptr<GamepadManager> _gp_man;
    static std::vector<ChannelSource*> _sources;
    
    static std::map<ORSave::AxisBoundAction::value_type, boost::shared_ptr<Channel> > _axis_action_map;
    static std::map<ORSave::ButtonBoundAction::value_type, boost::shared_ptr<Channel> > _button_action_map;
    static boost::scoped_ptr<ORSave::PresetListType> _preset_list;
    
    static void init();
    static void deinit();

    static void update();
    static void set_neutral();
    static boost::shared_ptr<Channel> xml_to_channel(const ORSave::BoundInputType& i);
    
    friend class App;
  
  public:
    static const float DEAD_ZONE;

    static boost::shared_ptr<Channel> get_null_channel() { return _null_channel; }
    static const GamepadManager& get_gamepad_manager() { return *_gp_man; }

    static const ORSave::PresetType& get_preset(const std::string& name);
    static const ORSave::PresetListType& get_preset_list() { return *_preset_list; }

    static void load_config_presets();
    static void set_channels_from_config();
    static const Channel& get_axis_ch(ORSave::AxisBoundAction::value_type action);
    static const Channel& get_button_ch(ORSave::ButtonBoundAction::value_type action);
};

#endif
