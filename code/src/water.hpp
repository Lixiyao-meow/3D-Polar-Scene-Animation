#pragma once

#include "vcl/vcl.hpp"

vcl::mesh initialize_water();
void update_water(vcl::mesh& water, vcl::mesh_drawable& water_visual, float t);
