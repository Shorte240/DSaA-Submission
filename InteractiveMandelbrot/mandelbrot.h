#pragma once

// Include GLUT, openGL, input.
#include "Includes.h"

#define SIZE 73728 // same as 1<<29  == 2^20 // Sizes: 65536, 73728. Anything less than 80k

#define TiS 1024 // same as 1<<29  == 2^20

// The size of the image to generate.
#define WIDTH 640
#define HEIGHT 480

// The number of times to iterate before we assume that a point isn't in the
// Mandelbrot set.
// (You may need to turn this up if you zoom further into the set.)
#define MAX_ITERATIONS 500

class mandelbrot
{
public:
	mandelbrot(Input *in);
	// Main render function
	void render();
	// Update function receives delta time from parent (used for frame independent updating).
	void update(float dt);
	// Resizes the OpenGL output based on new window size.
	void resize(int w, int h);

	void report_accelerator(const accelerator a);

	void list_accelerators();

	void query_AMP_support();

	void compute_mandelbrot_amp(float left_, float right_, float top_, float bottom_, std::vector<std::vector<uint32_t>>& image_);

	//compute_mandelbrot_amp(-0.751085f, -0.734975f, 0.118378f, 0.134488f, image); // Zoomed
	//compute_mandelbrot_amp(-2.0f, 1.0f, 1.125f, -1.125f, image); // Full

protected:
	// Renders text (x, y positions, RGB colour of text, string of text to be rendered)
	void displayText(float x, float y, float r, float g, float b, char* string);
	// A function to collate all text output in a single location
	void renderTextOutput();
	void calculateFPS();

	// draw primitive functions


	// For access to user input.
	Input* input;

	// For Window and frustum calculation.
	int width, height;
	float fov, nearPlane, farPlane;

	// For FPS counter and mouse coordinate output.
	int frame = 0, time, timebase = 0;
	char fps[40];
	char mouseText[40];
};
