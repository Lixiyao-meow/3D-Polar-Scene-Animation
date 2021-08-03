#include "terrain.hpp"

using namespace vcl;

mesh tore_with_texture()
{
    float const a = 12.0f;
    float const b = 0.3f;

    // Number of samples of the terrain is N x N
    const unsigned int N = 50;

    mesh tore; // temporary terrain storage (CPU only)
    tore.position.resize(N * N);
    tore.uv.resize(N * N);

    // Fill terrain geometry
    for (unsigned int ku = 0; ku < N; ++ku)
    {
        for (unsigned int kv = 0; kv < N; ++kv)
        {
            // Compute local parametric coordinates (u,v) \in [0,1]
            const float u = ku / (N - 1.0f);
            const float v = kv / (N - 1.0f);

            // Compute the local surface function

            vec3 const p = {
                    (a + b * std::cos(2 * pi * u)) * std::cos(2 * pi * v),
                    (a + b * std::cos(2 * pi * u)) * std::sin(2 * pi * v),
                    b * std::sin(2 * pi * u) };


            // Store vertex coordinates
            tore.position[kv + N * ku] = p;
            tore.uv[kv+N*ku] = { std::sin(2 * pi * (1 / u)) * std::cos(2 * pi * (1 / (v))) , std::sin(2 * pi * (1 / (v))) * std::cos(2 * pi * (1 / u)) };
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

            tore.connectivity.push_back(triangle_1);
            tore.connectivity.push_back(triangle_2);
        }
    }

    tore.fill_empty_field();
    return tore;
}

mesh disc_with_texture(float radius, float height)
{
    float const r = radius;

    mesh disc;
    size_t N = 20;

    for (size_t k = 0; k < size_t(N); ++k)
    {
        float const u = k / (N - 1.0f);
        vec3 const p = r * vec3(std::cos(2 * pi * u), std::sin(2 * pi * u), 0.0f);
        disc.position.push_back(p + vec3(0.0f, 0.0f, height));
        disc.uv.push_back({ std::sin(pi * u / 2), std::cos(pi * u / 2) });
    }
    // middle point
    disc.position.push_back({ 0,0,0 });
    disc.uv.push_back({ 0, 0 });

    for (size_t k = 0; k < size_t(N - 1); ++k)
        disc.connectivity.push_back(uint3{ unsigned(N), unsigned(k), unsigned(k + 1) });

    disc.fill_empty_field();

    disc.color.fill({ 1.0f, 1.0f, 1.0f });
    return disc;
}

mesh initialize_ice_pack()
{
    int const iceberg_sample = 200;
    mesh surf = mesh_primitive_disc(12.0f, { 0, 0 ,0.3f }, { 0,0,1 }, iceberg_sample);
    surf.color.fill({ 1.0f, 1.0f, 1.0f });
    mesh ice = tore_with_texture();

    ice.push_back(surf);

    return ice;
}


mesh create_ice(){
    float h = 0.3f;
    float r = 3.0f;
    float x;
    float y;

    x = r*std::rand()/RAND_MAX;
    y = r*std::rand()/RAND_MAX;
    vec3 p000 = {x,y,0};
    vec3 p001 = {x,y,h};

    x = r*std::rand()/RAND_MAX;
    y = r*std::rand()/RAND_MAX;
    vec3 p100 = {-x,y,0};
    vec3 p101 = {-x,y,h};

    x = r*std::rand()/RAND_MAX;
    y = r*std::rand()/RAND_MAX;
    vec3 p110 = {-x,-y,0};
    vec3 p111 = {-x,-y,h};

    x = r*std::rand()/RAND_MAX;
    y = r*std::rand()/RAND_MAX;
    vec3 p010 = {x,-y,0};
    vec3 p011 = {x,-y,h};

    mesh shape;
    shape.push_back(mesh_primitive_quadrangle(p000, p100, p101, p001));
    shape.push_back(mesh_primitive_quadrangle(p100, p110, p111, p101));
    shape.push_back(mesh_primitive_quadrangle(p110, p010, p011, p111));
    shape.push_back(mesh_primitive_quadrangle(p010, p000, p001, p011));
    shape.push_back(mesh_primitive_quadrangle(p001, p101, p111, p011));
    shape.push_back(mesh_primitive_quadrangle(p100, p000, p010, p110));

    return shape;
}

//create a list of ice to matching the list of ice position
std::vector<vcl::mesh_drawable> generate_ice_list(int unsigned N){
    std::vector<vcl::mesh_drawable> list;
    mesh_drawable ice;
    for(int unsigned i=0; i<N; i++){
        ice = mesh_drawable(create_ice());
        list.push_back(ice);
    }
    return list;
}

float dist(vcl::vec3 p, vcl::vec3 q){
    float dist = (p[0]-q[0])*(p[0]-q[0]) + (p[1]-q[1])*(p[1]-q[1]);
    return sqrt(dist);
}

//  generate float ice position to avoid the iceberg, the island
//  and the superposition between them

std::vector<vcl::vec3> generate_positions_on_terrain(int unsigned N){
    std::vector<vcl::vec3> p;
    bool add = true;
    do{
        float x = rand_interval(-17,17);
        float y = rand_interval(-17,17);

        for(vcl::vec3 q: p){
            if( dist(q,{x,y,0}) < 6.0 )
                add = false;
        }
        vec3 center_ice_pack = {6,6,0.3f};
        vec3 center_icerberg = {-9,-9,0};
        if(dist(center_ice_pack,{x,y,0}) < 16 ||dist(center_icerberg,{x,y,0}) < 10)
            add = false;
        if(add == true)
            p.push_back({x,y,0});
        add = true;
    }while( p.size() < N );
    return p;
}

std::vector<vcl::vec3> generate_tree_positions(int unsigned N){
    std::vector<vcl::vec3> p;
    float h = 0.3f;
    bool add = true;
    do{
        float x = rand_interval(4,17);
        float y = rand_interval(4,17);

        for(vcl::vec3 q: p){
            if( dist(q,{x,y,0}) < 2.0 )
                add = false;
        }
        vec3 center_ice = {6,6,0.3f};
        vec3 center_igloo = {2,5,0.3f};
        if( dist(center_ice,{x,y,0}) > 12 || dist(center_igloo,{x,y,0}) < 10)
            add = false;
        if(add == true)
            p.push_back({x,y,h});
        add = true;
    }while( p.size() < N );
    return p;
}

