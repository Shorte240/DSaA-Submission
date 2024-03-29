#pragma once

// Include GLUT, openGL, input.
#include "Includes.h"

// define a tile size
// max threads 1024 per tile
// max tiles 65536 in any dimension
// 32 = MAX
#define TS 4
//#define TS 8
//#define TS 16
//#define TS 32

class Mandelbrot2
{
public:
	Mandelbrot2(Input *in);
	~Mandelbrot2();
	// Main render function
	void render();
	// Update function receives delta time from parent (used for frame independent updating).
	void update(float dt);
	// Resizes the OpenGL output based on new window size.
	void resize(int w, int h);

	void report_accelerator(const accelerator a);

	void list_accelerators();

	void query_AMP_support();

	void gpu_amp_mandelbrot(float left_, float right_, float top_, float bottom_);

	void gpu_amp_mandelbrot_tiled(float left_, float right_, float top_, float bottom_);

protected:
	// Renders text (x, y positions, RGB colour of text, string of text to be rendered)
	void displayText(float x, float y, float r, float g, float b, char* string);
	// A function to collate all text output in a single location
	void renderTextOutput();
	void calculateFPS();
	// Initialise variables and texture
	void initVariables();
	void initTexture();
	// functions to time the mandelbrot set at various settings
	void timeNonTiled();
	void timeTiled();
	// Generates a 2x2 quad and scales to window size
	void generateQuad();
	// Sets WIDTH/HEIGHT based on user key presses
	void setWidth_Height();
	// Alters the movement modifier based upon the current value of zoom
	void alterMovementModifierByZoomLevel();
	// Lets the user move around the mandelbrot set based on key presses
	void userMovement();
	// Lets the user zoom into the mandelbrot set based on key presses
	void userZoom();
	// Alters the value of MAX_ITERATIONS based on key presses
	void setIterations();
	// Alters the values of the colours the mandelbrot set is calculated with based on key presses
	void setColour();
	// Allows the user to choose what computation to run
	void setComputation();

	// The number of times to iterate before we assume that a point isn't in the
	// Mandelbrot set.
	// (You may need to turn this up if you zoom further into the set.)
	int MAX_ITERATIONS;

	// Resolution/ Width/Height the mandelbrot set is calculated with
	int WIDTH = 640;
	int HEIGHT = 480;
	
	// Value by which MAX_ITERATIONS is modified by based on key press
	int iteration_modifier_;
	// Boolean to check if user has modified any variables and if the mandelbrot set needs recalculated as a result
	bool recalculate;
	// Booleans to check if the user is running tiled or non tiled
	bool running_non_tiled, running_tiled;
	// 2D Array for which the mandelbrot set information is stored in
	uint32_t image[1920][1280];
	// Texture for which the mandelbrot set is applied to
	GLuint mandelbrotTexture;
	// .CSV file for which the timings of the calculations of the mandelbrot set are saved to
	ofstream mandelbrot_timings_file;

	float X_Modifier_, Y_Modifier_, zoom_, movement_modifier_;
	int red, green, blue;

	// string to ouput to screen what computation mode is running
	std::string computationModeName;

	// For access to user input.
	Input* input;

	// For Window and frustum calculation.
	int window_width, window_height;
	float fov, nearPlane, farPlane;

	// For FPS counter and mouse coordinate output.
	int frame = 0, time = 0, timebase = 0;
	char fps[40];
	char mouseText[40];

	// char arrays to output to screen
	char iterationText[40];
	char zoomText[40];

	char blueText[40];
	char greenText[40];
	char redText[40];

	char widthText[40];
	char heightText[40];

	char computationText[40];
};

