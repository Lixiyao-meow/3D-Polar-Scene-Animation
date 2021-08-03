#pragma once

#include "vcl/vcl.hpp"

vcl::hierarchy_mesh_drawable initialize_pingouin();

vcl::vec3 const interpolation(float t, vcl::buffer<vcl::vec3> const& key_positions, vcl::buffer<float> const& key_times);

vcl::vec3 cardinal_spline_interpolation(float t, float t0, float t1, float t2, float t3, vcl::vec3 const& p0, vcl::vec3 const& p1, vcl::vec3 const& p2, vcl::vec3 const& p3, float K);

// Find the index k such that intervals[k] < t < intervals[k+1]
size_t find_index_of_interval(float t, vcl::buffer<float> const& intervals);

//funtions for pingouin's turning angle
float rotation_angle(float t, vcl::buffer<vcl::vec3> const& key_positions, vcl::buffer<float> const& key_times);

vcl::vec3 rotation_axe(float t, vcl::buffer<vcl::vec3> const& key_positions, vcl::buffer<float> const& key_times);

vcl::vec3 lissage_rotation_axe(float t, vcl::buffer<vcl::vec3> const& key_positions, vcl::buffer<float> const& key_times);
