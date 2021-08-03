
#include "water.hpp"

using namespace vcl;


mesh initialize_water()
{
	int const water_sample = 180;
	mesh water = mesh_primitive_grid({ -20, -20, 0.02f}, { 20, -20, 0.02f }, { 20, 20, 0.02f }, { -20, 20, 0.02f }, water_sample, water_sample);
	return water;
}

void update_water(mesh& water, mesh_drawable& water_visual, float t)
{
	// Number of samples in each direction (assuming a square grid)
	int const N = std::sqrt(water.position.size());

	// Recompute the new vertices
	for (int ku = 0; ku < N; ++ku) {
		for (int kv = 0; kv < N; ++kv) {

			// Compute local parametric coordinates (u,v) \in [0,1]
			const float u = ku / (N - 1.0f);
			const float v = kv / (N - 1.0f);

			int const idx = ku * N + kv;

            water.position[idx].z = 0.1f * (std::sin(20 * u + 23 * v + 2 * 3.14 * t)) * 1.5f;
		}
	}

	// Update the normal of the mesh structure
	water.compute_normal();

	// Update step: Allows to update a mesh_drawable without creating a new one
	water_visual.update_position(water.position);
	water_visual.update_normal(water.normal);
}

