/*
background.h: Header of the Background class
The Background class is responsible for outside lighting and for drawing the Smoke Ring and other distant background objects

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

#ifndef ORBIT_RIBBON_BACKGROUND_H
#define ORBIT_RIBBON_BACKGROUND_H

#include <boost/random.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

#include "autoxsd/orepkgdesc.h"
#include "geometry.h"

class MeshAnimation;

class Background {
  private:
    boost::shared_ptr<ORE1::SkySettingsType> _sky;
    boost::taus88 _random_gen;
    Vector _sky_offset;

    struct RandomStuffDensityRange {
      unsigned int segments;
      unsigned int total_count;
      float rad_min;
      float rad_max;
      float d_sigma;
      float y_sigma;

      RandomStuffDensityRange(unsigned int s, unsigned int tc, float rmin, float rmax, float ds, float ys)
        : segments(s), total_count(tc), rad_min(rmin), rad_max(rmax), d_sigma(ds), y_sigma(ys)
      {}
    };
    
    static std::vector<RandomStuffDensityRange> density_ranges;

  public:
    static void init();
    static void deinit();
    
    Background();

    void set_sky(const ORE1::SkySettingsType& sky);
    
    void draw_starbox();
    void draw_objects();
    Vector to_center_from_game_origin();
};

#endif
