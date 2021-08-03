#include "iceberg.hpp"
#include "math.h"

using namespace vcl;

mesh initialize_terrain()
{
    int const terrain_sample = 900;
    mesh terrain = mesh_primitive_grid({-2, -2, 0},{-20, -2, 0},{-20, -20, 0},{-2, -20, 0},terrain_sample,terrain_sample);
    return terrain;
}

void update_terrain(mesh& terrain, mesh_drawable& terrain_visual, perlin_noise_parameters const& parameters)
{
    // Number of samples in each direction (assuming a square grid)
    int const N = std::sqrt(terrain.position.size());

    // Recompute the new vertices
    for (int ku = 0; ku < N; ++ku) {
        for (int kv = 0; kv < N; ++kv) {

            // Compute local parametric coordinates (u,v) \in [0,1]
            const float u = ku/(N-1.0f);
            const float v = kv/(N-1.0f);

            int const idx = ku*N+kv;

            // Compute the Perlin noise
            float const noise = noise_perlin({u, v}, parameters.octave, parameters.persistency, parameters.frequency_gain);

            // use the noise as height value
            vec2 const u0 = { 0.5f, 0.5f };
            float const h0 = 2.0f;
            float const sigma0 = 0.15f;
            float const d = norm(vec2(u, v) - u0) / sigma0;

            float const z_f = h0 * std::exp(-pow(d, 4));

            terrain.position[idx].z = parameters.terrain_height*noise * z_f - 0.5f;

            // use also the noise as color value
            terrain.color[idx] = 0.2f*vec3(0,0.1f,0.5f)+ 0.8f*noise*vec3(1,1,1);
        }
    }

    // Update the normal of the mesh structure
    terrain.compute_normal();

    // Update step: Allows to update a mesh_drawable without creating a new one
    terrain_visual.update_position(terrain.position);
    terrain_visual.update_normal(terrain.normal);
    terrain_visual.update_color(terrain.color);
}

void update_relief(mesh& relief, mesh_drawable& relief_visual, perlin_noise_relief_parameters const& relief_parameters)
{
    // Number of samples in each direction (assuming a square grid)
    int const N = std::sqrt(relief.position.size());

    // Recompute the new vertices
    for (int ku = 0; ku < N; ++ku) {
        for (int kv = 0; kv < N; ++kv) {

            // Compute local parametric coordinates (u,v) \in [0,1]
            const float u = ku / (N - 1.0f);
            const float v = kv / (N - 1.0f);

            int const idx = ku * N + kv;

            // Compute the Perlin noise
            float const noise = noise_perlin({ u, v }, relief_parameters.octave, relief_parameters.persistency, relief_parameters.frequency_gain);

            // use the noise as height value
            relief.position[idx].z = relief_parameters.terrain_height * noise - 0.3;
        }
    }

    // Update the normal of the mesh structure
    relief.compute_normal();

    // Update step: Allows to update a mesh_drawable without creating a new one
    relief_visual.update_position(relief.position);
    relief_visual.update_normal(relief.normal);
}
