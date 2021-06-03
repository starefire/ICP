// icp.cpp 
// Author: JJ

// C++ 
#include <cstdlib>
#include <iostream>
#include <vector>
#include <chrono>
// OpenCV 
#include <opencv2\opencv.hpp>
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

using namespace cv;
using namespace std;

typedef struct s_globals {
	GLFWwindow *window;
	cv::VideoCapture capture;
	int height;
	int width;
	double app_start_time;
} s_globals;

// the only global variable - encapsulates all
s_globals globals = { NULL, };

//forward declaration
void finalize(int code);

//=====================================================================================================
static void error_callback(int error, const char* description)
{
	std::cerr << "Error: " << description << std::endl;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void windowsize_callback(GLFWwindow* window, int width, int height)
{
	globals.width = width;
	globals.height = height;

	glMatrixMode(GL_PROJECTION);				// set projection matrix for following transformations
	glLoadIdentity();							// clear all transformations

	glOrtho(0, width, 0, height, -1000, 1000);  // set Orthographic projection

	//glScalef(1, -1, 1);							// invert Y-axis, so that values decrease in downwards direction to be same as OS coordinates

	//glTranslatef(0, -height, 0);				// shift the origin to bottom left corner 
	glViewport(0, 0, width, height);			// set visible area

	std::cout << width << " " << height << std::endl;
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
		std::cout << "GLFW init err" << std::endl;
		finalize(EXIT_FAILURE);
	}

	// Try to obtain OpenGL 2.0 context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	globals.window = glfwCreateWindow(640, 480, "OpenGL context", NULL, NULL);
	if (!globals.window)
	{
		std::cerr << "GLFW window creation error." << std::endl;
		finalize(EXIT_FAILURE);
	}

	// Get some GLFW info.
	{
		int major, minor, revision;
		glfwGetVersion(&major, &minor, &revision);
		std::cout << "Running GLFW " << major << '.' << minor << '.' << revision << std::endl;
		std::cout << "Compiled against GLFW " << GLFW_VERSION_MAJOR << '.' << GLFW_VERSION_MINOR << '.' << GLFW_VERSION_REVISION << std::endl;
	}

	// Set more GLFW
	glfwSetKeyCallback(globals.window, key_callback);							// Key callback function (onKeyPress).
	glfwSetWindowSizeCallback(globals.window, windowsize_callback);				// On window resize callback.

	glfwMakeContextCurrent(globals.window);										// Set current window.
	glfwGetFramebufferSize(globals.window, &globals.width, &globals.height);	// Get window size.
	globals.app_start_time = glfwGetTime();										// Get start time.
	glfwSwapInterval(1);														// Set V-Sync ON.

	//initial GL window size setting - call callback manually
	windowsize_callback(globals.window, globals.width, globals.height);


	//
	// Initialize all valid GL extensions with GLEW.
	// Usable AFTER creating GL context!
	//

	GLenum glew_ret = glewInit();
	if (glew_ret != GLEW_OK)
	{
		std::cerr << "Failed to init extensions." << std::endl;
		finalize(EXIT_FAILURE);
	}


	//
	// Initialize OpenCV camera capture.
	//

	globals.capture = cv::VideoCapture(cv::CAP_DSHOW);

	if (!globals.capture.isOpened())
	{
		std::cerr << "no camera" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
	{
		std::cout << "Camera " <<
			": width=" << globals.capture.get(cv::CAP_PROP_FRAME_WIDTH) <<
			", height=" << globals.capture.get(cv::CAP_PROP_FRAME_HEIGHT) <<
			", FPS=" << globals.capture.get(cv::CAP_PROP_FPS) << std::endl;
	}

	if (!globals.capture.set(cv::CAP_PROP_FRAME_WIDTH, 640))
		std::cout << "Failed width." << std::endl;
	if (!globals.capture.set(cv::CAP_PROP_FRAME_HEIGHT, 480))
		std::cout << "Failed height." << std::endl;
	if (!globals.capture.set(cv::CAP_PROP_FPS, 60))
		std::cout << "Failed FPS." << std::endl;

	std::cout << "Camera changed:" <<
		": width=" << globals.capture.get(cv::CAP_PROP_FRAME_WIDTH) <<
		", height=" << globals.capture.get(cv::CAP_PROP_FRAME_HEIGHT) <<
		", FPS=" << globals.capture.get(cv::CAP_PROP_FPS) << std::endl;
}


void DrawMyObject(double _x, double _y)
{
	glm::vec3 color_red = { 1.0f, 0.0f, 0.0f };
	glm::vec3 color_green = { 0.0f, 1.0f, 0.0f };
	glm::vec3 color_blue = { 0.0f, 0.0f, 1.0f };
	glm::vec3 color_white = { 1.0f, 1.0f, 1.0f };
	glm::vec3 color_black = { 0.0f, 0.0f, 0.0f };
	glm::vec3 vertex_c = { 0.0f, 29.0f, 0.0f };
	glTranslatef(-_x, _y, 0.0f); // translate origin to new position

	//draw triangle (relative to origin)
	glBegin(GL_TRIANGLES);

	glColor3f(1.0, 0.0, 0.0);                // example of using immediate value for colour
	glVertex3f(-25, -14, 0.0);               // example of using immediate value for vertex coordinates
	glColor3fv(glm::value_ptr(color_green)); // example of using memory array value for colour
	glVertex3f(25, -14, 0.0);
	glColor3fv(glm::value_ptr(color_blue));
	glVertex3fv(glm::value_ptr(vertex_c));   // example of using memory array value for vertex coordinates

	glEnd();
}

int main(int argc, char * argv[])
{
	cv::Mat frame, scene;

	
	int iLowH = 0;
	int iHighH = 179;

	int iLowS = 146;
	int iHighS = 255;

	int iLowV = 0;
	int iHighV = 255;
	

	init();

	while (1)
	{
		globals.capture.read(frame);
		if (frame.empty())
		{
			std::cerr << "Cam disconnected?" << std::endl;
			break;
		}

		

		// analyze the image...
		// ...
		cv::Mat scene_hsv, scene_threshold;

		cv::cvtColor(frame, scene_hsv, cv::COLOR_BGR2HSV);

		cv::Scalar lower_threshold = cv::Scalar(iLowH, iLowS, iLowV);
		cv::Scalar upper_threshold = cv::Scalar(iHighH, iHighS, iHighV);
		cv::inRange(scene_hsv, lower_threshold, upper_threshold, scene_threshold);

		Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));

		erode(scene_threshold, scene_threshold, element);
		dilate(scene_threshold, scene_threshold, element);


		// nalezeni centroidu
		Moments m = moments(scene_threshold, false);
		// nalezeni stredoveho bodu
		Point p1(m.m10 / m.m00, m.m01 / m.m00);

		double centerX = p1.x;
		double centerY = p1.y;

		// jendotlive body pro vykresleni ohraniceni
		Point cp1 = Point(centerX - 20, centerY);
		Point cp2 = Point(centerX + 20, centerY);
		Point cp3 = Point(centerX, centerY + 20);
		Point cp4 = Point(centerX, centerY - 20);

		// spojeni bodu carami
		cv::line(frame, cp1, cp2, CV_RGB(0, 255, 0), 2, LINE_AA);
		cv::line(frame, cp3, cp4, CV_RGB(0, 255, 0), 2, LINE_AA);
		

		cv::imshow("threshold", scene_threshold);
		cv::imshow("grabbed", frame);
		
		
		// Clear color buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Use ModelView matrix for following trasformations (translate,rotate,scale)
		glMatrixMode(GL_MODELVIEW);
		// Clear all tranformations
		glLoadIdentity();

		//move to window center
		glTranslatef(globals.width / 2.0f, globals.height / 2.0f, 0.0f);

		// Render here 
		// ...
		DrawMyObject(centerX, centerY);

		// Swap front and back buffers 
		// Calls glFlush() inside
		glfwSwapBuffers(globals.window);

		// Poll for and process events
		glfwPollEvents();

		if (cv::waitKey(1) == 27)
			break;
	}

	finalize(EXIT_SUCCESS);
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

	exit(code);
}
