#pragma once

// Include GLUT, openGL, input.
#include "Includes.h"

#define TILE_SIZE 8
#define wi 1024
#define he 768
#define DATA_SIZE 65536

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

	void compute_mandelbrot_amp(float left_, float right_, float top_, float bottom_);
	void compute_mandelbrot_amp_tiled(float left_, float right_, float top_, float bottom_);

	//compute_mandelbrot_amp(-0.751085f, -0.734975f, 0.118378f, 0.134488f, image); // Zoomed
	//compute_mandelbrot_amp(-2.0f, 1.0f, 1.125f, -1.125f, image); // Full

protected:
	// Renders text (x, y positions, RGB colour of text, string of text to be rendered)
	void displayText(float x, float y, float r, float g, float b, char* string);
	// A function to collate all text output in a single location
	void renderTextOutput();
	void calculateFPS();

	// draw primitive functions

	// The number of times to iterate before we assume that a point isn't in the
	// Mandelbrot set.
	// (You may need to turn this up if you zoom further into the set.)
	int MAX_ITERATIONS;
	int WIDTH = 640;
	int HEIGHT = 480;
	
	int iteration_modifier_;
	bool recalculate;
	uint32_t image[1920][1080];
	std::array<uint32_t, DATA_SIZE> i;
	GLuint texture;

	ofstream mandelbrot_timings_file;

	float left_, right_, top_, bottom_, zoom_, movement_modifier_;
	int red, green, blue;

	// For access to user input.
	Input* input;

	// For Window and frustum calculation.
	int window_width, window_height;
	float fov, nearPlane, farPlane;

	// For FPS counter and mouse coordinate output.
	int frame = 0, time = 0, timebase = 0;
	char fps[40];
	char mouseText[40];
	char iterationText[40];
	char zoomText[40];
	char blueText[40];
	char greenText[40];
	char redText[40];
	char widthText[40];
	char heightText[40];
};

