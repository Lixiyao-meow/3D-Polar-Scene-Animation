#include "vcl/vcl.hpp"
#include <iostream>

#include "terrain.hpp"
#include "water.hpp"
#include "iceberg.hpp"
#include "igloo.hpp"
#include "pingouin.hpp"

#include <list>

using namespace vcl;

struct gui_parameters {
	bool display_frame = false;
	bool display_wireframe = false;
	bool display_surface = true;
};

struct user_interaction_parameters {
	vec2 mouse_prev;
	timer_fps fps_record;
	mesh_drawable global_frame;
	gui_parameters gui;
	bool cursor_on_gui;
};
user_interaction_parameters user;

struct scene_environment
{
	camera_around_center camera;
	mat4 projection;
	vec3 light;
};
scene_environment scene;

// Structure of a particle to simulate snow
struct particle_structure
{
	vcl::vec3 p; // Position
	vcl::vec3 v; // Speed
};

void mouse_move_callback(GLFWwindow* window, double xpos, double ypos);
void window_size_callback(GLFWwindow* window, int width, int height);

void initialize_data();
void display_interface();
void display_terrain();
void display_frame();


int N = 10; // number of pack ice

buffer<vec3> key_positions; //penguin's position
buffer<vec3> key_positions_2;
buffer<float> key_times;
timer_interval timer;
float const dt = 0.01;

// iceberg
mesh terrain;
mesh_drawable iceberg;
perlin_noise_parameters parameters;

//ocean
mesh ocean;
mesh_drawable ocean_visual;

// ice
mesh_drawable ice_pack;
std::vector<vcl::vec3> ice_position;
std::vector<vcl::mesh_drawable> ice_list;

// igloo
mesh_drawable igloo;
mesh_drawable door;
mesh_drawable igloo2;
mesh_drawable door2;

// billboard tree
mesh_drawable billboard_tree;
std::vector<vcl::vec3> tree_position;

//penguins
hierarchy_mesh_drawable pingouin;
hierarchy_mesh_drawable pingouin_2;
hierarchy_mesh_drawable pingouin_3;
hierarchy_mesh_drawable pingouin_4;
hierarchy_mesh_drawable pingouin_5;

// snow
std::list<particle_structure> particles;
mesh_drawable sphere;
timer_event_periodic timer_event(0.005f);

// relief of the island
mesh relief;
mesh_drawable relief_visual;
perlin_noise_relief_parameters relief_parameters;

// cube under the ocean
mesh cube;
mesh_drawable cube_visual;

int main(int, char* argv[])
{
	std::cout << "Run " << argv[0] << std::endl;

	int const width = 1280, height = 1024;
	GLFWwindow* window = create_window(width, height);
	window_size_callback(window, width, height);
	std::cout << opengl_info_display() << std::endl;;

	imgui_init(window);
	glfwSetCursorPosCallback(window, mouse_move_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);
	
	std::cout<<"Initialize data ..."<<std::endl;
	initialize_data();

	std::cout<<"Start animation loop ..."<<std::endl;
	user.fps_record.start();
	glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(window))
	{
		scene.light = scene.camera.position();
		user.fps_record.update();
		
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);
		imgui_create_frame();
		if(user.fps_record.event) {
			std::string const title = "VCL Display - "+str(user.fps_record.fps)+" fps";
			glfwSetWindowTitle(window, title.c_str());
		}

		ImGui::Begin("GUI",NULL,ImGuiWindowFlags_AlwaysAutoResize);
		user.cursor_on_gui = ImGui::IsAnyWindowFocused();

		if(user.gui.display_frame) draw(user.global_frame, scene);

		display_interface();
		display_terrain();
		display_frame();

		ImGui::End();
		imgui_render_frame(window);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	imgui_cleanup();
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

void display_terrain()
{
    draw(iceberg, scene);
	draw(ocean_visual, scene);
    draw(ice_pack, scene);
    for (int i=0; i<N; i++){
        ice_list[i].transform.translate = ice_position[i];
        draw(ice_list[i], scene);
    }
	draw(igloo, scene);
    draw(door, scene);
    draw(igloo2, scene);
    draw(door2, scene);
	draw(relief_visual, scene);

	draw(cube_visual, scene);

    //draw billborad snow_pine
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(false);
    billboard_tree.transform.rotate = scene.camera.orientation();
    for (vcl::vec3 p : tree_position){
        billboard_tree.transform.translate = p;
        draw(billboard_tree, scene);
    }
    glDepthMask(true);
}


void initialize_data()
{
	// Basic setups of shaders and camera
	GLuint const shader_mesh = opengl_create_shader_program(opengl_shader_preset("mesh_vertex"), opengl_shader_preset("mesh_fragment"));
	mesh_drawable::default_shader = shader_mesh;
	mesh_drawable::default_texture = opengl_texture_to_gpu(image_raw{1,1,image_color_type::rgba,{255,255,255,255}});

	user.global_frame = mesh_drawable(mesh_primitive_frame());
	user.gui.display_frame = false;
	scene.camera.distance_to_center = 2.5f;
	scene.camera.look_at({-3,1,2}, {0,0,0.5}, {0,0,1});


    // Create icerberg
	terrain = initialize_terrain();
    iceberg = mesh_drawable(terrain);
    update_terrain(terrain, iceberg, parameters);

	// Create ocean
	ocean = initialize_water();
	ocean_visual = mesh_drawable(ocean);
	image_raw const im = image_load_png("assets/mer.png");
	GLuint const texture_image_id = opengl_texture_to_gpu(im,
		GL_CLAMP_TO_EDGE,
		GL_CLAMP_TO_EDGE);
	ocean_visual.texture = texture_image_id;

    // Create pack ice
    ice_pack = mesh_drawable(initialize_ice_pack());
    ice_pack.transform.translate = {6,6,0};
	//create ice pack texture
	image_raw const im_snow = image_load_png("assets/snow.png");
	GLuint const texture_snow_pack = opengl_texture_to_gpu(im_snow,
		GL_CLAMP_TO_EDGE,
		GL_CLAMP_TO_EDGE);
	ice_pack.texture = texture_snow_pack;

    //create the tiny ice packs
    ice_list = generate_ice_list(N);
    ice_position = generate_positions_on_terrain(N);
    image_raw const im_ice0 = image_load_png("assets/ice_pack.png");
    GLuint const texture_ice_pack = opengl_texture_to_gpu(im_ice0,
        GL_CLAMP_TO_EDGE,
        GL_CLAMP_TO_EDGE);
    for(int i=0; i<N; i++){
        ice_list[i].texture = texture_ice_pack;
    }

    // Create igloo
    igloo = mesh_drawable(create_igloo());
    image_raw const im_igloo = image_load_png("assets/igloo.png");
    GLuint const texture_image_igloo_id = opengl_texture_to_gpu(im_igloo,
        GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
    igloo.texture = texture_image_igloo_id;
    igloo.transform.translate = { -0.5f,9,0.3f };
    //create igloo door
    door = create_door({ -0.5f, 9, 0.3f });

    // Create igloo2
    igloo2 = mesh_drawable(create_igloo());
    igloo2.texture = texture_image_igloo_id;
    igloo2.transform.translate = {5,2,0.3f };
    igloo2.transform.rotate = rotation({0,0,1}, 3.14f/4);
    door2 = create_door({3,3,0.3f });
    door2.transform.translate = { 6.4, 3.4, 0.3f };
    door2.transform.rotate = rotation({0,0,1}, 3.14f/4);

    // Create pingouin and pingouin moving position
	pingouin = initialize_pingouin();
	pingouin_2 = initialize_pingouin();
	pingouin_3 = initialize_pingouin();
	pingouin_4 = initialize_pingouin();
	pingouin_5 = initialize_pingouin();

    key_positions = {{4,12.5,0},{4.5,15,0},{3.5,16.5,0}, {1.5,16.5,0}, {0,14.5,0}, {0.5,12.5,0}, {2.5,12,0}, {4,12.5,0}, {4.5,15,0}, {3.5,16.5,0} };
    key_positions_2 = { {6.5,10.5,0}, {7.5,9,0}, {7,6.5,0}, {5.5,6,0}, {3.5,6.5,0}, {3,8.5,0}, {3.5,10.5,0}, {6.5,10.5,0}, {7.5,9,0},{7,6.5,0} };
    key_times = {0.0f, 2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.0f, 14.0f, 16.0f, 18.0f};

	// Set timer bounds
    size_t const N = key_times.size();
    timer.t_min = key_times[1];
    timer.t_max = key_times[N-2]-dt;
    timer.t = timer.t_min;

    //create snow pines
    billboard_tree = mesh_drawable(mesh_primitive_quadrangle({1,0,0},{-1,0,0},{-1,2,0},{1,2,0}));
    billboard_tree.transform.scale = 1.5f;
    billboard_tree.texture = opengl_texture_to_gpu(image_load_png("assets/snow_pine.png"));
    tree_position = generate_tree_positions(10);

	// Create snow
    float const r = 0.1f;
	sphere = mesh_drawable(mesh_primitive_sphere(r));
	//sphere.shading.color = { 0.5f,0.5f,1.0f };
	sphere.texture = texture_snow_pack;

	// Create relief
    relief = mesh_primitive_grid({ 15, 13.5, 0 }, { -3, 13.5, 0 }, { -3, -1.5 , 0 }, { 15, -1.5, 0 }, 500, 500);
    relief_visual = mesh_drawable(relief);
	relief_visual.texture = texture_snow_pack;
    update_relief(relief, relief_visual, relief_parameters);

	// Create cube
	cube = mesh_primitive_cubic_grid({ -20, -20, -0.2f }, { 20, -20, -0.2f }, { 20, 20, -0.2f }, { -20, 20, -0.2f },
									 { -20, -20, -2 }, { 20, -20, -2 }, { 20, 20, -2 }, { -20, 20, -2 },
									 180, 180);
	cube_visual = mesh_drawable(cube);
	cube_visual.texture = texture_image_id;
}



void display_interface()
{
	ImGui::SliderFloat("Time", &timer.t, timer.t_min, timer.t_max);
	ImGui::SliderFloat("Time scale", &timer.scale, 0.0f, 2.0f);
}

void display_frame()
{
	// Update the current time
	timer.update();
	float const t = timer.t;


	/** *************************************************************  **/
	/** Compute the (animated) transformations applied to the elements **/
	/** *************************************************************  **/
    // Compute the interpolated position for penguin1
    vec3 p = interpolation(t, key_positions, key_times);
    vec3 p1 = interpolation(t+dt, key_positions, key_times);
    vec3 axe = normalize(p1-p);

	// first penguin
    pingouin["body0"].transform.translate = p;
    pingouin["body0"].transform.rotate = rotation_between_vector({0,1,0}, axe);
    pingouin["wing_left"].transform.rotate = rotation({ 0,1,0 }, 0.2f * (2 + std::sin(2 * 3.14f * t)));
	pingouin["wing_right"].transform.rotate = rotation({ 0,1,0 }, -0.2f * (2 + std::sin(2 * 3.14f * t)));
	pingouin["feet_left"].transform.rotate = rotation({ 1,0,0 }, 0.1f * (std::sin(2 * 3.14f * t)));
	pingouin["feet_right"].transform.rotate = rotation({ 1,0,0 }, -0.1f * (std::sin(2 * 3.14f * t)));

	// second penguin
    pingouin_2["body0"].transform.translate = {-2, 12, 0};
	pingouin_2["body0"].transform.rotate = rotation({ 0, 0, 1 }, -3.14f / 2);
    pingouin_2["wing_left"].transform.rotate = rotation({ 0,1,0 }, 0.2f * (2 + std::sin(2 * 3.14f * t)));
    pingouin_2["wing_right"].transform.rotate = rotation({ 0,1,0 }, -0.2f * (2 + std::sin(2 * 3.14f * t)));

    // third penguin
    p = interpolation(t, key_positions_2, key_times);
    p1 = interpolation(t+dt, key_positions_2, key_times);
    axe = normalize(p1-p);

	pingouin_3["body0"].transform.translate = {9, 10, 0};
    pingouin_3["body0"].transform.translate = p;
	pingouin_3["body0"].transform.rotate = rotation_between_vector({ 0,1,0 }, axe);
	pingouin_3["wing_left"].transform.rotate = rotation({ 0,1,0 }, 0.2f * (2 + std::sin(2 * 3.14f * t)));
	pingouin_3["wing_right"].transform.rotate = rotation({ 0,1,0 }, -0.2f * (2 + std::sin(2 * 3.14f * t)));
	pingouin_3["feet_left"].transform.rotate = rotation({ 1,0,0 }, 0.1f * (std::sin(2 * 3.14f * t)));
	pingouin_3["feet_right"].transform.rotate = rotation({ 1,0,0 }, -0.1f * (std::sin(2 * 3.14f * t)));

	// fourth penguin
    pingouin_4["body0"].transform.translate = { 1, 6, 0 };
    pingouin_4["body0"].transform.rotate = rotation({ 0, 0, 1 }, -3.14f / 4);
    pingouin_4["wing_left"].transform.rotate = rotation({ 0,1,0 }, 0.2f * (2 + std::sin(2 * 3.14f * t)));
    pingouin_4["wing_right"].transform.rotate = rotation({ 0,1,0 }, -0.2f * (2 + std::sin(2 * 3.14f * t)));

	// fifth penguin
	pingouin_5["body0"].transform.translate = { 9, 2, 0 };
	pingouin_5["body0"].transform.rotate = rotation({ 0, 0, 1 }, 3.14f / 4);
	pingouin_5["wing_left"].transform.rotate = rotation({ 0,1,0 }, 0.2f * (2 + std::sin(2 * 3.14f * t)));
	pingouin_5["wing_right"].transform.rotate = rotation({ 0,1,0 }, -0.2f * (2 + std::sin(2 * 3.14f * t)));

	// update the global coordinates
	pingouin.update_local_to_global_coordinates();
	pingouin_2.update_local_to_global_coordinates();
	pingouin_3.update_local_to_global_coordinates();
	pingouin_4.update_local_to_global_coordinates();
	pingouin_5.update_local_to_global_coordinates();

	// display the hierarchy
	if (user.gui.display_surface) {
		draw(pingouin, scene);
		draw(pingouin_2, scene);
		draw(pingouin_3, scene);
		draw(pingouin_4, scene);
		draw(pingouin_5, scene);
	}

	// Snow
	float const dt = timer_event.update();

	bool const new_particle = timer_event.event;
	if (new_particle == true) {
		float const x0 = rand_interval(-20.0f, 20.0f);
		float const y0 = rand_interval(-20.0f, 20.0f);
		vec3 const p0 = { x0,y0, 20 };
		const vec3 v0 = vec3(0, 0, 0);

		particles.push_back({ p0,v0 });
	}

	// Evolve position of particles
	const vec3 g = { 0.0f,0.0f,-9.81f };
	for (particle_structure& particle : particles)
	{
		const float m = 0.01f; // particle mass

		vec3& p = particle.p;
		vec3& v = particle.v;

		const vec3 F = m * g;

		// Numerical integration
		v = v + dt * F / (3 * m);
		p = p + dt * v;
	}


	// Remove particles that are too low
	for (auto it = particles.begin(); it != particles.end(); ) {
		if (it->p.z < 0.02)
			it = particles.erase(it);
		if (it != particles.end())
			++it;
	}

	// Display particles
	for (particle_structure& particle : particles)
	{
		sphere.transform.translate = particle.p;
		draw(sphere, scene);
	}

	// Create wave
	update_water(ocean, ocean_visual, t);
}

void window_size_callback(GLFWwindow* , int width, int height)
{
	glViewport(0, 0, width, height);
	float const aspect = width / static_cast<float>(height);
	scene.projection = projection_perspective(50.0f*pi/180.0f, aspect, 0.1f, 100.0f);
}

void mouse_move_callback(GLFWwindow* window, double xpos, double ypos)
{
	vec2 const  p1 = glfw_get_mouse_cursor(window, xpos, ypos);
	vec2 const& p0 = user.mouse_prev;
	glfw_state state = glfw_current_state(window);

	auto& camera = scene.camera;
	if(!user.cursor_on_gui){
		if(state.mouse_click_left && !state.key_ctrl)
			scene.camera.manipulator_rotate_trackball(p0, p1);
		if(state.mouse_click_left && state.key_ctrl)
			camera.manipulator_translate_in_plane(p1-p0);
		if(state.mouse_click_right)
			camera.manipulator_scale_distance_to_center( (p1-p0).y );
	}

	user.mouse_prev = p1;
}

void opengl_uniform(GLuint shader, scene_environment const& current_scene)
{
	opengl_uniform(shader, "projection", current_scene.projection);
	opengl_uniform(shader, "view", current_scene.camera.matrix_view());
	opengl_uniform(shader, "light", current_scene.light, false);
}



