// icp.cpp 
// Author: JJ

#include <string>
// C++ 
#include <cstdlib>
#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>
#include <thread>
#include <opencv2\opencv.hpp>


// OpenCV 

// OpenGL Extension Wrangler
#include <glew.h> 
#include <wglew.h> //WGLEW = Windows GL Extension Wrangler (change for different platform) 
// GLFW toolkit
#include <glfw3.h>
// OpenGL math
#define GLM_FORCE_INLINE // favor speed over size
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
// Lua scripting
#include "lua_engine.h"
#include "OBJloader.h"

using namespace std;
using namespace cv;

class mesh
{
public:

	int type;
	vector< glm::vec3 > vertices;
	vector< glm::vec4 > colors;
	vector< glm::vec2 > uvs;
	vector< glm::vec3 > normals; // Won't be used at the moment.
	GLenum primitive_type = GL_POINTS;
	bool normals_used;
	bool textures_used;
	bool colors_used = true;

	glm::vec3 origin;
	glm::vec3 direction;
	glm::vec3 right;
	glm::vec3 up;
	glm::vec3 size;
	float horizontalAngle;
	float verticalAngle;
};


mesh cube;
mesh skybox_t;
mesh skybox_m;
mesh skybox_b;
mesh barrierl;
mesh barrierr;
mesh ball;

GLfloat color_white[] = { 1.0, 1.0, 1.0, 1.0 };
GLuint texture_id;
Mat image;
mesh player;
mesh & camera = player;

float global_Xangle = 0;
float global_Yangle = 0;

double mouse_x, mouse_y;
float speed = 300.0f;
float mouseSpeed = 0.05f;
double lastTime = 0;
float lastX, lastY;
bool firstMouse = true;
int score = 0;

float ball_min = -1500;

float barier_left = 0;
float barier_right = 1000;
float barier_up = 1000;
float barier_down = 0; 

int barrierl_direction = 1;
int barrierr_direction = 1;


bool close = false;


typedef struct s_globals {
	GLFWwindow* window;
	VideoCapture capture;
	int height;
	int width;
	double app_start_time;
	bool fullscreen;
} s_globals;

static mutex my_mutex;
int global_x, global_y;
static const int num_threads = 1;
vector< glm::vec3 > vertices;
vector< glm::vec4 > colors;
vector< glm::vec2 > uvs;
vector< glm::vec3 > normals; // Won't be used at the moment.

float ball_jump_max = 400.0f;
float ball_jump_min = -501.0f;
int ball_direction = -1;

// the only global variable - encapsulates all
s_globals globals = { NULL, };
Mat frame;

bool key_s, key_a, key_d, key_w, key_left, key_right, key_up, key_down, key_esc, key_f = false;

mesh board[10][10];


//forward declaration
void finalize(int code);

void draw_all();

void gen_board();

//=====================================================================================================

static void error_callback(int error, const char* description)
{
	cerr << "Error: " << description << endl;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS) {
		switch (toupper(key)) {
		case GLFW_KEY_UP:
			key_up = true;
			break;
		case GLFW_KEY_DOWN:
			key_down = true;
			break;
		case GLFW_KEY_LEFT:
			key_left = true;
			break;
		case GLFW_KEY_RIGHT:
			key_right = true;
			break;
		case GLFW_KEY_W:
			key_w = true;
			break;
		case GLFW_KEY_S:
			key_s = true;
			break;
		case GLFW_KEY_A:
			key_a = true;
			break;
		case GLFW_KEY_D:
			key_d = true;
			break;
		case GLFW_KEY_ESCAPE:
			key_esc = true;
			break;
		case GLFW_KEY_F:
			key_f = true;
			break;
		}

	}
	if (action == GLFW_RELEASE) {
		switch (toupper(key)) {
		case GLFW_KEY_UP:
			key_up = false;
			break;
		case GLFW_KEY_DOWN:
			key_down = false;
			break;
		case GLFW_KEY_LEFT:
			key_left = false;
			break;
		case GLFW_KEY_RIGHT:
			key_right = false;
			break;
		case GLFW_KEY_W:
			key_w = false;
			break;
		case GLFW_KEY_S:
			key_s = false;
			break;
		case GLFW_KEY_A:
			key_a = false;
			break;
		case GLFW_KEY_D:
			key_d = false;
			break;
		case GLFW_KEY_ESCAPE:
			key_esc = false;
			break;
		case GLFW_KEY_F:
			key_f = false;
			break;
		}

	}
		
}

static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	mouse_x = xpos;
	mouse_y = ypos;

}

static void fbsize_callback(GLFWwindow* window, int width, int height)
{
	globals.width = width;
	globals.height = height;

	glMatrixMode(GL_PROJECTION);				// set projection matrix for following transformations
	glm::mat4 projectionMatrix = glm::perspective(
		glm::radians(90.0f), // The vertical Field of View, in radians: the amount of "zoom". Think "camera lens". Usually between 90° (extra wide) and 30° (quite zoomed in)
		(float)width / (float)height,       // Aspect Ratio. Depends on the size of your window.
		0.1f,              // Near clipping plane. Keep as big as possible, or you'll get precision issues.
		20000.0f             // Far clipping plane. Keep as little as possible.
	);
	glLoadMatrixf(glm::value_ptr(projectionMatrix));
	glViewport(0, 0, width, height);			// set visible area
	
	//glLoadIdentity();							// clear all transformations

	//glOrtho(0, width, 0, height, -1000, 1000);  // set Orthographic projection

	//glScalef(1, -1, 1);							// invert Y-axis, so that values decrease in downwards direction to be same as OS coordinates

	//glTranslatef(0, -height, 0);				// shift the origin to bottom left corner 
	//glViewport(0, 0, width, height);
	cout << width << " " << height << endl;
}

static void init_lights(void)
{
	const glm::vec3 dark_grey(0.3f, 0.3f, 0.3f);

	//lighting model setup
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, glm::value_ptr(dark_grey));  //colour of default ambient light
	glShadeModel(GL_SMOOTH);   //Gouraud shading
	glEnable(GL_NORMALIZE);    //normalisation of EVERYTHING! Slower, but safe. 
	glEnable(GL_LIGHTING);

	// light above gaming board - white
	glEnable(GL_LIGHT0);
	GLfloat ambientLight[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	GLfloat diffuseLight[] = { 0.8f, 0.8f, 0.8, 1.0f };
	GLfloat specularLight[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	GLfloat position[] = { 500.0f, 0.0f, 500.0f, 1.0f };

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	// light below gaming board = "underglow" - red
	glEnable(GL_LIGHT1);
	GLfloat diffuseLight1[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	GLfloat specularLight1[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	GLfloat position1[] = { 500.0f, -400.0f, 500.0f, 0.0f };

	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseLight1);
	glLightfv(GL_LIGHT1, GL_SPECULAR, specularLight1);
	glLightfv(GL_LIGHT1, GL_POSITION, position1);

	//more lighting setup...
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
}

static void init(void)
{
	//
	// GLFW init.
	//

	// set error callback first
	glfwSetErrorCallback(error_callback);

	//initialize GLFW library
	int glfw_ret = glfwInit();
	if (!glfw_ret)
	{
		cout << "GLFW init err" << endl;
		finalize(EXIT_FAILURE);
	}

	// Try to obtain OpenGL 2.0 context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // only new functions
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE); // only old functions (for old tutorials etc.)
	
	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	globals.window = glfwCreateWindow(640, 480, "OpenGL context", NULL, NULL);
	if (!globals.window)
	{
		cerr << "GLFW window creation error." << endl;
		finalize(EXIT_FAILURE);
	}

	// Get some GLFW info.
	{
		int major, minor, revision;
		glfwGetVersion(&major, &minor, &revision);
		cout << "Running GLFW " << major << '.' << minor << '.' << revision << endl;
		cout << "Compiled against GLFW " << GLFW_VERSION_MAJOR << '.' << GLFW_VERSION_MINOR << '.' << GLFW_VERSION_REVISION << endl;
	}

	glfwSetInputMode(globals.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// Set more GLFW
	glfwSetKeyCallback(globals.window, key_callback);							// Key callback function (onKeyPress).
	glfwSetCursorPosCallback(globals.window, cursor_pos_callback);
	glfwSetWindowSizeCallback(globals.window, fbsize_callback);				// On window resize callback.

	glfwMakeContextCurrent(globals.window);										// Set current window.
	glfwGetFramebufferSize(globals.window, &globals.width, &globals.height);	// Get window size.
	globals.app_start_time = glfwGetTime();										// Get start time.
	glfwSwapInterval(1);														// Set V-Sync ON.

	
	//initial GL window size setting - call callback manually
	fbsize_callback(globals.window, globals.width, globals.height);				//initial GL window size setting - call callback manually


	//
	// Initialize all valid GL extensions with GLEW.
	// Usable AFTER creating GL context!
	//
	{
		GLenum glew_ret;
		glew_ret = glewInit();
		if (glew_ret != GLEW_OK)
		{
			cerr << "WGLEW failed with error: " << glewGetErrorString(glew_ret) << endl;
			finalize(EXIT_FAILURE);
		}
		else
		{
			cout << "GLEW successfully initialized to version: " << glewGetString(GLEW_VERSION) << endl;
		}

		// Platform specific. (Change to GLXEW or ELGEW if necessary.)
		glew_ret = wglewInit();
		if (glew_ret != GLEW_OK)
		{
			cerr << "WGLEW failed with error: " << glewGetErrorString(glew_ret) << endl;
			finalize(EXIT_FAILURE);
		}
		else
		{
			cout << "WGLEW successfully initialized platform specific functions." << endl;
		}

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_POLYGON_SMOOTH); //antialiasing
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_NORMALIZE);

		glPolygonMode(GL_FRONT, GL_FILL);           // mode for model draw...
		glPolygonMode(GL_BACK, GL_FILL);            // both fron and back (necessary for transparent objects)
		glDisable(GL_CULL_FACE);                    // no polygon removal

		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // needed for texture mapping 

		glGenTextures(1, &texture_id);
		glBindTexture(GL_TEXTURE_2D, texture_id);

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // tiling in S axis
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // tiling in T axis 

		image = cv::imread("tex.png"); // .JPG, .TIFF, ...
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.cols, image.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, image.data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);



		//
		// Initialize OpenCV camera capture.
		//

		globals.capture = VideoCapture(CAP_DSHOW);

		if (!globals.capture.isOpened())
		{
			cerr << "no camera" << endl;
			finalize(EXIT_FAILURE);
		}
		else
		{
			cout << "Camera " <<
				": width=" << globals.capture.get(CAP_PROP_FRAME_WIDTH) <<
				", height=" << globals.capture.get(CAP_PROP_FRAME_HEIGHT) <<
				", FPS=" << globals.capture.get(CAP_PROP_FPS) << endl;
		}

		if (!globals.capture.set(CAP_PROP_FRAME_WIDTH, 640))
			cerr << "Failed width." << endl;
		if (!globals.capture.set(CAP_PROP_FRAME_HEIGHT, 480))
			cerr << "Failed height." << endl;
		if (!globals.capture.set(CAP_PROP_FPS, 30))
			cerr << "Failed FPS." << endl;

		cout << "Camera changed:" <<
			": width=" << globals.capture.get(CAP_PROP_FRAME_WIDTH) <<
			", height=" << globals.capture.get(CAP_PROP_FRAME_HEIGHT) <<
			", FPS=" << globals.capture.get(CAP_PROP_FPS) << endl;
	}

	{
		const char *vendor_s = (const char *)glGetString(GL_VENDOR);
		const char *renderer_s = (const char *)glGetString(GL_RENDERER);
		const char *version_s = (const char *)glGetString(GL_VERSION);
		const char *glsl_s = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);

		cout << "OpenGL driver vendor: " << (vendor_s == nullptr ? "<UNKNOWN>" : vendor_s) << ", renderer: " << (renderer_s == nullptr ? "<UNKNOWN>" : renderer_s) << ", version: " << (version_s == nullptr ? "<UNKNOWN>" : version_s) << endl;

		GLint profile_flags;
		glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile_flags);
		cout << "Current profile: ";
		if (profile_flags & GL_CONTEXT_CORE_PROFILE_BIT)
			cout << "CORE";
		else
			cout << "COMPATIBILITY";
		cout << endl;

		GLint context_flags;
		glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
		cout << "Active context flags: ";
		if (context_flags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT)
			cout << "GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT ";
		if (context_flags & GL_CONTEXT_FLAG_DEBUG_BIT)
			cout << "GL_CONTEXT_FLAG_DEBUG_BIT ";
		if (context_flags & GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT)
			cout << "GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT ";
		if (context_flags & GL_CONTEXT_FLAG_NO_ERROR_BIT)
			cout << "GL_CONTEXT_FLAG_NO_ERROR_BIT";
		cout << endl;

		cout << "Primary GLSL shading language version: " << (glsl_s == nullptr ? "<UNKNOWN>" : glsl_s) << endl;
	}

	

	//
	// Lua engine
	//
	/*
	if (globals.lua.loadfile("./resources/test.lua") != EXIT_SUCCESS)
		cerr << "Lua? load" << endl;
	if (globals.lua.run() != EXIT_SUCCESS)
		cerr << "Lua? runtime error" << endl;
	*/
	//
	// GLM library
	//

	cout << "GLM version: " << GLM_VERSION_MAJOR << '.' << GLM_VERSION_MINOR << '.' << GLM_VERSION_PATCH << "rev" << GLM_VERSION_REVISION << endl;

	//
	// OpenGL settings
	// 

	//glEnable(GL_CULL_FACE);  // disable draw of back face
	//glCullFace(GL_BACK);

	
}

void mesh_draw(mesh & mesh) {
	if (mesh.vertices.size() == 0) return;

	
	glVertexPointer(3, GL_FLOAT, 0, mesh.vertices.data());
	glEnableClientState(GL_VERTEX_ARRAY);

	if (mesh.colors_used) {
		glColorPointer(4, GL_FLOAT, 0, mesh.colors.data());
		glEnableClientState(GL_COLOR_ARRAY);
	}

	glNormalPointer(GL_FLOAT, 0, mesh.normals.data());
	glEnableClientState(GL_NORMAL_ARRAY);

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color_white);

	if (mesh.textures_used) 
	{
		glEnable(GL_TEXTURE_2D);
		
		glTexCoordPointer(2, GL_FLOAT, 0, mesh.uvs.data());
		glEnableClientState(GL_TEXTURE_COORD_ARRAY_EXT);
	}
	glDrawArrays(mesh.primitive_type, 0, mesh.vertices.size());
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY_EXT);
	glDisable(GL_TEXTURE_2D);
}

bool loadOBJ(mesh & out_mesh, std::string obj_path)
{
	bool res;

	if (obj_path.size() == 0)
		return false;

	//out_mesh.clear();
	out_mesh.primitive_type = GL_TRIANGLES;

	res = loadOBJ(obj_path.c_str(), out_mesh.vertices, out_mesh.uvs, out_mesh.normals);
	if (!res)
		return false;

	if (out_mesh.normals.size() > 0)
	{
		out_mesh.normals_used = true;
		if (out_mesh.vertices.size() != out_mesh.normals.size())
			return false;
	}

	return true;
}

void set_subtexture(mesh & mesh, int tex_x, int tex_y, int subtex_count) {
	for (int i = 0; i < 6; i++) {
		mesh.uvs.data()[6 * i].x = (float)(tex_x+1) / subtex_count;
		mesh.uvs.data()[6 * i].y = (float)(tex_y+1) / subtex_count;
		mesh.uvs.data()[6 * i + 1].x = (float)(tex_x + 1) / subtex_count;
		mesh.uvs.data()[6 * i + 1].y = (float)tex_y / subtex_count;
		mesh.uvs.data()[6 * i + 2].x = (float)tex_x / subtex_count;
		mesh.uvs.data()[6 * i + 2].y = (float)(tex_y + 1) / subtex_count;
		mesh.uvs.data()[6 * i + 3].x = (float)tex_x / subtex_count;
		mesh.uvs.data()[6 * i + 3].y = (float)(tex_y + 1) / subtex_count;
		mesh.uvs.data()[6 * i + 4].x = (float)(tex_x + 1) / subtex_count;
		mesh.uvs.data()[6 * i + 4].y = (float)tex_y / subtex_count;
		mesh.uvs.data()[6 * i + 5].x = (float)(tex_x) / subtex_count;
		mesh.uvs.data()[6 * i + 5].y = (float)(tex_y) / subtex_count;
	}
}

bool check_collision(mesh& obj_a, mesh& obj_b) // AABB - AABB collision
{
	// collision x-axis?
	bool collisionX = obj_a.origin.x + obj_a.size.x >= obj_b.origin.x &&
		obj_b.origin.x + obj_b.size.x >= obj_a.origin.x;
	// collision y-axis?
	bool collisionY = obj_a.origin.y + obj_a.size.y >= obj_b.origin.y &&
		obj_b.origin.y + obj_b.size.y >= obj_a.origin.y;
	// collision z-axis?
	bool collisionZ = obj_a.origin.z + obj_a.size.z >= obj_b.origin.z &&
		obj_b.origin.z + obj_b.size.z >= obj_a.origin.z;
	// collision only if on both axes
	return collisionX && collisionY && collisionZ;
}

bool check_collisions() {
	bool is_collision = false;

	// check collision with the board
	for (int i = 0; i < 10; i++) {
		// board[i]->type tady je problem - 2D pole se v tomto pripade nechova spravne
		for (int j = 0; j < 10; j++) {
			is_collision = check_collision(ball, board[i][j]);
			if (is_collision) {
				ball_direction = 1;
				if (ball.type == board[i][j].type)
					score++;
				else
					score--;
				cout << "Score:" << score << endl;
				gen_board();
				return true;
			}
		}
	}

	is_collision = check_collision(ball, barrierl);
	if (is_collision) {
		ball_direction = 1;
		score = 0;
		return true;
	}

	is_collision = check_collision(ball, barrierr);
	if (is_collision) {
		ball_direction *= -1;
		score = 0;
		return true;
	}

	return false;
}

void move_player() {
	double currentTime = glfwGetTime();
	if (lastTime == 0) lastTime = currentTime;
	float deltaTime = float(currentTime - lastTime);
	//cout << floor(1 / deltaTime) << " FPS" << endl;
	lastTime = currentTime;

	if (firstMouse) // this bool variable is initially set to true
	{
		lastX = mouse_x;
		lastY = mouse_y;
		firstMouse = false;
	}

	float xoffset = mouse_x - lastX;
	float yoffset = lastY - mouse_y; // reversed since y-coordinates range from bottom to top
	lastX = mouse_x;
	lastY = mouse_y;

	xoffset *= mouseSpeed;
	yoffset *= mouseSpeed;

	camera.horizontalAngle -= xoffset * deltaTime;
	camera.verticalAngle += yoffset * deltaTime;

	// set the camera angle limit to 90° -> PI/2 1.56f
	if (camera.horizontalAngle > 0.785f)
		camera.horizontalAngle = 0.785f;
	if (camera.verticalAngle < -0.785f)
		camera.verticalAngle = -0.785f;

	camera.direction = glm::normalize(glm::vec3(cosf(camera.verticalAngle) * sinf(camera.horizontalAngle), sinf(camera.verticalAngle), cosf(camera.verticalAngle) * cosf(camera.horizontalAngle)));
	
	camera.right = glm::normalize(glm::vec3(sinf(camera.horizontalAngle - 3.14f / 2.0f), 0, cosf(camera.horizontalAngle - 3.14f / 2.0f)));

	camera.up = glm::cross(camera.direction, camera.right);

	// check collisions - ball vs. other objects
	bool is_collision = check_collisions();

	if (ball.origin.y >= ball_jump_max) {
		ball_direction = -1;
	}

	if (ball.origin.y <= ball_min) {
		cout << "GAME OVER" << endl;
		glfwSetWindowShouldClose(globals.window, 1);
	}

	if (barrierr.origin.z > barier_up) {
		barrierr_direction = -1;
	}
	else if (barrierr.origin.z < barier_down) {
		barrierr_direction = 1;
	}

	if (barrierl.origin.x > barier_right) {
		barrierl_direction = -1;
	}
	else if (barrierl.origin.x < barier_left) {
		barrierl_direction = 1;
	}

	barrierl.origin.x += barrierl_direction * deltaTime * speed;
	barrierr.origin.z += barrierr_direction * deltaTime * speed;
	
	ball.origin.y += ball_direction * deltaTime * speed;

	if (key_w) {
		ball.origin.x -= camera.direction.x * deltaTime * speed;
		ball.origin.z -= camera.direction.z * deltaTime * speed;
	}
	if (key_s) { 
		ball.origin.x += camera.direction.x * deltaTime * speed;
		ball.origin.z += camera.direction.z * deltaTime * speed;
	}
	if (key_a) { 
		ball.origin.x += camera.right.x * deltaTime * speed;
		ball.origin.z += camera.right.z * deltaTime * speed;
	}
	if (key_d){ 
		ball.origin.x -= camera.right.x * deltaTime * speed;
		ball.origin.z -= camera.right.z * deltaTime * speed;
	}
	if (key_esc) glfwSetWindowShouldClose(globals.window, 1);
	if (key_f) {
		if (globals.fullscreen) {
			globals.fullscreen = false;
			glfwSetWindowMonitor(globals.window, globals.fullscreen ? glfwGetPrimaryMonitor() : NULL, 50, 50, 800, 600, GLFW_DONT_CARE);
		}
		else {
			globals.fullscreen = true;
			glfwSetWindowMonitor(globals.window, globals.fullscreen ? glfwGetPrimaryMonitor() : NULL, 0, 0, 1920, 1080, GLFW_DONT_CARE);
		}
	}
	
}

void thread_code(const int tid, int &result) {
	thread::id this_id = this_thread::get_id();
	cout << "Nalezeni: " << this_id << endl;
	//find_object();
}

void gen_board() {
	
	for (int i = 0; i < 36; i++) {
		cube.colors.push_back(glm::vec4(1.00f, 1.0f, 1.0f, 1.0f));
	}
	cube.textures_used = true;
	for (int i = 0; i < 10; i++) {
		
		for (int j = 0; j < 10; j++) {
			cube.type = rand() % 4;
			set_subtexture(cube, 3+cube.type, 2, 16);
			board[i][j] = cube;
		}
	}
}

int main(int argc, char **argv) {
	unsigned int hw = thread::hardware_concurrency();
	cout << "Got HW threads: " << hw << endl;

	globals.fullscreen = false;

	init();
	player.origin = glm::vec3(0.0f, 0.0f, 0.0f);
	loadOBJ(cube, "cube.obj");
	cube.size = glm::vec3(100.0f, 100.0f, 100.0f);
	lastX = globals.width / 2;
	lastY = globals.height / 2;

	gen_board();

	loadOBJ(ball, "cube.obj");
	set_subtexture(ball, 2, 2, 16);
	ball.origin = glm::vec3(100.0f, ball_jump_max, 100.0f);
	ball.size = glm::vec3(50.0f, 50.0f, 50.0f);
	ball.type = 3;

	// skybox
	loadOBJ(skybox_t, "cube.obj");
	set_subtexture(skybox_t, 1, 0, 16);
	skybox_t.origin = glm::vec3(600.0f, 4000.0f, 100.0f);
	skybox_t.size = glm::vec3(8000.0f, 4000.0f, 8000.0f);

	loadOBJ(skybox_m, "cube.obj");
	set_subtexture(skybox_m, 0, 0, 16);
	skybox_m.origin = glm::vec3(600.0f, 200.0f, 100.0f);
	skybox_m.size = glm::vec3(8000.0f, 4000.0f, 8000.0f);

	loadOBJ(skybox_b, "cube.obj");
	set_subtexture(skybox_b, 2, 0, 16);
	skybox_b.origin = glm::vec3(600.0f, -3700.0f, 100.0f);
	skybox_b.size = glm::vec3(8000.0f, 4000.0f, 8000.0f);


	float barrierh = -300.0f;

	loadOBJ(barrierl, "cube.obj");
	set_subtexture(barrierl, 2, 2, 16);
	barrierl.origin = glm::vec3(200.0f, barrierh, 100.0f);
	barrierl.size = glm::vec3(300.0f, 5.0f, 300.0f);

	loadOBJ(barrierr, "cube.obj");
	set_subtexture(barrierr, 2, 2, 16);
	barrierr.origin = glm::vec3(1000.0f, barrierh, 0.0f);
	barrierr.size = glm::vec3(300.0f, 5.0f, 300.0f);


	vector<thread> threads;
	int result = 0;

	threads.resize(num_threads);
	for (int i = 0; i < num_threads; ++i) {
		threads[i] = thread(thread_code, i, ref(result));
	}
	thread::id this_id = this_thread::get_id();
	cout << "Vykresleni: " << this_id << endl;
	while (!glfwWindowShouldClose(globals.window)) {
		draw_all();
		move_player();
		glfwPollEvents();
	}
	finalize(EXIT_SUCCESS);

	for (int i = 0; i < num_threads; ++i) {
		threads[i].join();
	}

	cout << "Result: " << result << endl;

	return EXIT_SUCCESS;
}

void finalize(int code)
{
	// Close camera if opened
	if (globals.capture.isOpened())
		globals.capture.release();

	// Close OpenGL window and terminate GLFW  
	if (globals.window)
		glfwDestroyWindow(globals.window);
	glfwTerminate();
}

void draw_all()
{
	// Clear color buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Use ModelView matrix for following trasformations (translate,rotate,scale)
	glMatrixMode(GL_MODELVIEW);
	// Clear all tranformations
	glLoadIdentity();

	// draw camera
	const float radius = 200.0f;
	float cam_x = ball.origin.x + (radius * cos(camera.verticalAngle) * sin(camera.horizontalAngle));
	float cam_y = ball.origin.y + (radius * sin(camera.horizontalAngle) * sin(camera.verticalAngle));
	float cam_z = ball.origin.z + (radius * cos(camera.horizontalAngle));
	glm::mat4 v_m = glm::lookAt(glm::vec3(cam_x, cam_y, cam_z), ball.origin , glm::vec3(0, 1, 0));

	
	glLoadIdentity();
	glLoadMatrixf(glm::value_ptr(v_m));
	
	// lights
	init_lights();

	// skybox_t
	for (int i = 0; i < (skybox_t.vertices.size()); i++) {
		// transparency of object done using alpha in color
		skybox_t.colors.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.00f));
	}

	skybox_t.textures_used = true;
	glPushMatrix();
	glTranslatef(skybox_t.origin.x, skybox_t.origin.y, skybox_t.origin.z);
	glScalef(skybox_t.size.x, skybox_t.size.y, skybox_t.size.z);
	mesh_draw(skybox_t);
	glPopMatrix();

	// skybox_m
	for (int i = 0; i < (skybox_m.vertices.size()); i++) {
		// transparency of object done using alpha in color
		skybox_m.colors.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.00f));
	}

	skybox_m.textures_used = true;
	glPushMatrix();
	glTranslatef(skybox_m.origin.x, skybox_m.origin.y, skybox_m.origin.z);
	glScalef(skybox_m.size.x, skybox_m.size.y, skybox_m.size.z);
	mesh_draw(skybox_m);
	glPopMatrix();

	skybox_m.textures_used = true;
	glPushMatrix();
	glTranslatef(skybox_m.origin.x, skybox_m.origin.y , skybox_m.origin.z - skybox_m.size.z + 50);
	glScalef(skybox_m.size.x, skybox_m.size.y, skybox_m.size.z);
	mesh_draw(skybox_m);
	glPopMatrix();


	// skybox_b
	for (int i = 0; i < (skybox_b.vertices.size()); i++) {
		// transparency of object done using alpha in color
		skybox_b.colors.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.00f));
	}

	skybox_b.textures_used = true;
	glPushMatrix();
	glTranslatef(skybox_b.origin.x, skybox_b.origin.y, skybox_b.origin.z);
	glScalef(skybox_b.size.x, skybox_b.size.y, skybox_b.size.z);
	mesh_draw(skybox_b);
	glPopMatrix();

	// draw board
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {
			board[i][j].origin = glm::vec3(board[i][j].size.x + j * board[i][j].size.x, ball_jump_min, -300.0f + i * board[i][j].size.z);
			glPushMatrix();
			glTranslatef(board[i][j].origin.x, board[i][j].origin.y, board[i][j].origin.z);
			glScalef(board[i][j].size.x, board[i][j].size.y, board[i][j].size.z);
			mesh_draw(board[i][j]);
			glPopMatrix();
		}
	}

	for (int i = 0; i <( ball.vertices.size() ); i++) {
		// transparency of object done using alpha in color
		ball.colors.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	}

	ball.textures_used = true;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	for (int i = 0; i < (36); i++) {
		barrierl.colors.push_back(glm::vec4(1.0f, 0.0f, 0.0f, 0.5f));
	}

	//barrierl.textures_used = true;
	glPushMatrix();
	glTranslatef(barrierl.origin.x, barrierl.origin.y, barrierl.origin.z);
	glScalef(barrierl.size.x, barrierl.size.y, barrierl.size.z);
	mesh_draw(barrierl);
	glPopMatrix();

	for (int i = 0; i < (36); i++) {
		barrierr.colors.push_back(glm::vec4(1.0f, 0.0f, 0.0f, 0.5f));
	}

	//barrierr.textures_used = true;
	glPushMatrix();
	glTranslatef(barrierr.origin.x, barrierr.origin.y, barrierr.origin.z);
	glScalef(barrierr.size.x, barrierr.size.y, barrierr.size.z);
	mesh_draw(barrierr);
	glPopMatrix();

	
	//draw with materials, lights and texture
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color_white);

	//enable texturing
	glEnable(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//semi-transparent object, colour through Phong model
	
	glPushMatrix();
	glTranslatef(ball.origin.x, ball.origin.y, ball.origin.z);
	glScalef(ball.size.x, ball.size.y, ball.size.z);
	mesh_draw(ball);
	glPopMatrix();

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);

	glfwSwapBuffers(globals.window);
}