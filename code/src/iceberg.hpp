#pragma once

#include "vcl/vcl.hpp"

struct perlin_noise_parameters
{
    float persistency = 0.6f;
    float frequency_gain = 2.052f;
    int octave = 6;
    float terrain_height = 5.0f;
};

struct perlin_noise_relief_parameters
{
    float persistency = 0.6f;
    float frequency_gain = 2.0f;
    int octave = 5;
    float terrain_height = 0.5f;
};

vcl::mesh initialize_terrain();
void update_terrain(vcl::mesh& terrain, vcl::mesh_drawable& terrain_visual, perlin_noise_parameters const& parameters);
void update_relief(vcl::mesh& relief, vcl::mesh_drawable& relief_visual, perlin_noise_relief_parameters const& parameters);
