#ifndef ORBIT_RIBBON_CONSTANTS_H
#define ORBIT_RIBBON_CONSTANTS_H

// String describing the current version of Orbit Ribbon
#define APP_VERSION "prealpha"

// Maximum number of frames per second, and the base number of simulated steps per second
#define MAX_FPS 60

// Default coefficients for linear and angular damping on new GameObjs
#define DEFAULT_VEL_DAMP_COEF 0.15
#define DEFAULT_ANG_DAMP_COEF 0.15

// How far from -1, 0, or 1 where we consider an input axis to just be at those exact values
#define DEAD_ZONE 0.001

// Space separated list of key names to ignore in the input module because they cause problems
#define IGNORE_KEYS "[-] numlock"

#endif