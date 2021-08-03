
#include "igloo.hpp"

using namespace vcl;

mesh cylinder_with_texture()
{
    float const r = 0.8f;
    float const h = 2.7f;

    // Number of samples of the terrain is N x N
    const unsigned int N = 40;

    mesh cylinder; // temporary terrain storage (CPU only)
    cylinder.position.resize(N * N);
    cylinder.uv.resize(N * N);

    // Fill terrain geometry
    for (unsigned int ku = 0; ku < N; ++ku)
    {
        for (unsigned int kv = 0; kv < N; ++kv)
        {
            // Compute local parametric coordinates (u,v) \in [0,1]
            const float u = ku / (N - 1.0f);
            const float v = kv / (N - 1.0f);

            // Compute the local surface function
            vec3 const p = { h * (v - 0.15),r * std::cos(2 * pi * u), r * std::sin(2 * pi * u) };
            vec2 const uv = { u,v };

            // Store vertex coordinates
            cylinder.position[kv + N * ku] = p;
            cylinder.uv[kv + N * ku] = { 3 * u,3 * v };
        }
    }

    // Generate triangle organization
    for (size_t ku = 0; ku < N - 1; ++ku)
    {
        for (size_t kv = 0; kv < N - 1; ++kv)
        {
            const unsigned int idx = kv + N * ku;

            const uint3 triangle_1 = { idx, idx + 1 + N, idx + 1 };
            const uint3 triangle_2 = { idx, idx + N, idx + 1 + N };

            cylinder.connectivity.push_back(triangle_1);
            cylinder.connectivity.push_back(triangle_2);
        }
    }

    cylinder.fill_empty_field();
    return cylinder;
}

mesh create_igloo() {
    float const radius = 1.9f;
    mesh sphere = mesh_primitive_sphere2(radius, {0,0,0} , 100, 100);
    mesh cylinder_cpu = cylinder_with_texture();
    sphere.push_back(cylinder_cpu);

    return sphere;
}

mesh_drawable create_door(vec3 p){
    mesh door = mesh_primitive_disc(0.8f,{0,0,0},{1,0,0},80);
    mesh_drawable door_texture = mesh_drawable(door);
    // Texture Igloo
    image_raw const im_door = image_load_png("assets/wooden.png");
    GLuint const texture_image_igloo_id = opengl_texture_to_gpu(im_door,
        GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
    door_texture.texture = texture_image_igloo_id;
    door_texture.transform.translate = { p+vec3{2,0,0}};
    door_texture.transform.rotate = rotation({ 1, 0, 0 }, -3.14f / 2);

    return door_texture;
}


