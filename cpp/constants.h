/*
constants.h: Header defining game-wide constants

Copyright 2009 David Simon. You can reach me at david.mike.simon@gmail.com

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

#ifndef ORBIT_RIBBON_CONSTANTS_H
#define ORBIT_RIBBON_CONSTANTS_H

#include "geometry.h"

// String describing the current version of Orbit Ribbon
const char* const APP_VERSION = "prealpha";

// Maximum number of frames per second, and the base number of simulated steps per second
const unsigned int MAX_FPS = 60;

// How many ticks each frame must at least last
const unsigned int MIN_TICKS_PER_FRAME = 1000/MAX_FPS;

// Default coefficients for linear and angular damping on new GameObjs
const float DEFAULT_VEL_DAMP_COEF = 0.15;
const float DEFAULT_ANG_DAMP_COEF = 0.15;

// How far from -1, 0, or 1 where we consider an input axis to just be at those exact values
const float DEAD_ZONE = 0.001;

// Space separated list of key names to ignore in the input module because they cause problems
const char* const IGNORE_KEYS = "[-] numlock";

// How many bytes to load from ORE files in each chunk
const int ORE_CHUNK_SIZE = 4096;

// How many kilobytes should be allocated for the vertices buffer and faces buffer respectively
const int VERTICES_BUFFER_ALLOCATED_SIZE = 4096;
const int FACES_BUFFER_ALLOCATED_SIZE = 1024;

// Clipping distance for gameplay objects and background objects respectively
const float GAMEPLAY_CLIP_DIST = 50000;
const float SKY_CLIP_DIST = 1e12;

// Field-of-view in degrees
const float FOV = 45;

// Camera positioning relative to avatar's reference frame
const Vector GAMEPLAY_CAMERA_POS_OFFSET(0.0, 1.1, -6.0);
const Vector GAMEPLAY_CAMERA_TGT_OFFSET(0.0, 1.1, 0.0);
const Vector GAMEPLAY_CAMERA_UP_VECTOR(0.0, 1.0, 0.0);

// How many frames are analyzed in calculating performance info
const unsigned int PERF_FRAMES_WINDOW = 100;

#endif
