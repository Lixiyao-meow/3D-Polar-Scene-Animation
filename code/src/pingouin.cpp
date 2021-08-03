
#include "water.hpp"
#include "pingouin.hpp"

using namespace vcl;

hierarchy_mesh_drawable initialize_pingouin()
{
	hierarchy_mesh_drawable hierarchy;
	// Definition of the elements of the hierarchy
	// ------------------------------------------- //
	vec3 const parameters_body0 = { 0.35f,0.35f,0.5f };
	vec3 const parameters_body1 = { 0.28f,0.28f,0.4f };
	float const radius_head = 0.23f;
	float const eye_white_radius = 0.09f;
	float const eye_black_radius = 0.07f;
	vec3 const parameters_beak = { 0.06f,0.1f,0.02f };
	vec3 const parameters_wing = { 0.04f,0.15f,0.3f };
	vec3 const parameters_feet = { 0.1f,0.2f,0.04f };

	mesh_drawable body0 = mesh_drawable(mesh_primitive_ellipsoid(parameters_body0, { 0,0,0.8f }, 80, 80));
	body0.shading.color = { 0.3f,0.3f,0.3f };
	mesh_drawable body1 = mesh_drawable(mesh_primitive_ellipsoid(parameters_body1, { 0,0,0.8f }, 80, 80));
	mesh_drawable head = mesh_drawable(mesh_primitive_sphere(radius_head, { 0,0,0.8f }, 40, 40));
	head.shading.color = { 0.3f,0.3f,0.3f };
	mesh_drawable eye_white = mesh_drawable(mesh_primitive_sphere(eye_white_radius, { 0,0,0.8f }, 40, 40));
	mesh_drawable eye_black = mesh_drawable(mesh_primitive_sphere(eye_black_radius, { 0,0,0.8f }, 40, 40));
	eye_black.shading.color = { 0.3f,0.3f,0.3f };
	mesh_drawable beak = mesh_drawable(mesh_primitive_ellipsoid(parameters_beak, { 0,0,0.8f }, 40, 40));
	beak.shading.color = { 0.93f,0.68f,0.05f };
	mesh_drawable wing = mesh_drawable(mesh_primitive_ellipsoid(parameters_wing, { 0,0, -0.3f }, 60, 60));
	wing.shading.color = { 0.3f,0.3f,0.3f };
	mesh_drawable feet = mesh_drawable(mesh_primitive_ellipsoid(parameters_feet, { 0,0.2f,0.8f }, 40, 40));
	feet.shading.color = { 0.93f,0.68f,0.05f };

    // Build the hierarchy: The root of the hierarchy is the body
	hierarchy.add(body0, "body0");
	hierarchy.add(body1, "body1", "body0", vec3{ 0,0.1f,0 });
	hierarchy.add(head, "head", "body0", vec3{ 0,0,0.6f });
	hierarchy.add(eye_white, "eye_white_left", "head", vec3{ 0.06f,0.14f,0.05f });
	hierarchy.add(eye_white, "eye_white_right", "head", vec3{ -0.06f,0.14f,0.05f });
	hierarchy.add(eye_black, "eye_black_left", "eye_white_left", vec3{ 0.01f,0.022f,0.008f });
	hierarchy.add(eye_black, "eye_black_right", "eye_white_right", vec3{ -0.01f,0.022f,0.008f });
	hierarchy.add(beak, "beak_up", "head", vec3{ 0,0.2f,-0.02f });
	hierarchy.add(beak, "beak_down", "head", vec3{ 0,0.2f,-0.03f });
	hierarchy.add(wing, "wing_left", "body0", vec3{ -0.28f, 0, 1.1f });
	hierarchy.add(wing, "wing_right", "body0", vec3{ 0.28f, 0, 1.1f });
	hierarchy.add(feet, "feet_left", "body0", vec3{ 0.15f,-0.15f,-0.46f });
	hierarchy.add(feet, "feet_right", "body0", vec3{ -0.15f,-0.15,-0.46f });

	return hierarchy;
}

vec3 const interpolation(float t, buffer<vec3> const& key_positions, buffer<float> const& key_times)
{
    // Find idx such that key_times[idx] < t < key_times[idx+1]
    int const idx = find_index_of_interval(t, key_times);

    // Parameters used to compute the spline interpolation
    float const t0 = key_times[idx-1]; // = t_{i-1}
    float const t1 = key_times[idx]; // = t_{i}
    float const t2 = key_times[idx+1]; // = t_{i+1}
    float const t3 = key_times[idx+2]; // = t_{i+2}

    vec3 const& p0 = key_positions[idx-1]; // = p_{i-1}
    vec3 const& p1 = key_positions[idx]; // = p_{i}
    vec3 const& p2 = key_positions[idx+1]; // = p_{i+1}
    vec3 const& p3 = key_positions[idx+2]; // = p_{i+2}

    float const K = 0.5;
    vec3 const p = cardinal_spline_interpolation(t,t0,t1,t2,t3,p0,p1,p2,p3,K);

    return p;
}

vec3 cardinal_spline_interpolation(float t, float t0, float t1, float t2, float t3, vec3 const& p0, vec3 const& p1, vec3 const& p2, vec3 const& p3, float K)
{
    float const s = (t-t1)/(t2-t1);
    vec3 const d0 = 2*K*(p2-p0)/(t2-t0);
    vec3 const d1 = 2*K*(p3-p1)/(t3-t1);
    vec3 const p = (2*pow(s,3)-3*pow(s,2)+1)*p1 + (pow(s,3)-2*pow(s,2)+s)*d0 + (-2*pow(s,3)+3*pow(s,2))*p2 + (pow(s,3)-pow(s,2))*d1;
    return p;
}

vec3 rotation_axe(float t, buffer<vec3> const& key_positions, buffer<float> const& key_times){
    int const idx = find_index_of_interval(t, key_times);
    vec3 const& p1 = normalize(key_positions[idx+1]-key_positions[idx]);

    return p1;
}

size_t find_index_of_interval(float t, buffer<float> const& intervals)
{
    size_t const N = intervals.size();
    bool error = false;
    if (intervals.size() < 2) {
        std::cout<<"Error: Intervals should have at least two values; current size="<<intervals.size()<<std::endl;
        error = true;
    }
    if (N>0 && t < intervals[0]) {
        std::cout<<"Error: current time t is smaller than the first time of the interval"<<std::endl;
        error = true;
    }
    if(N>0 && t > intervals[N-1]) {
        std::cout<<"Error: current time t is greater than the last time of the interval"<<std::endl;
        error = true;
    }
    if (error == true) {
        std::string const error_str = "Error trying to find interval for t="+str(t)+" within values: ["+str(intervals)+"]";
        error_vcl( error_str );
    }


    size_t k=0;
    while( intervals[k+1]<t )
        ++k;
    return k;
}


