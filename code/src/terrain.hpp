#pragma once

#include "vcl/vcl.hpp"

vcl::mesh initialize_ice_pack();
vcl::mesh tore_with_texture();
vcl::mesh disc_with_texture(float radius, float height);
vcl::mesh create_ice();
std::vector<vcl::mesh_drawable> generate_ice_list(int unsigned N);
float dist(vcl::vec3 p, vcl::vec3 q);
std::vector<vcl::vec3> generate_positions_on_terrain(int unsigned N);
std::vector<vcl::vec3> generate_tree_positions(int unsigned N);
