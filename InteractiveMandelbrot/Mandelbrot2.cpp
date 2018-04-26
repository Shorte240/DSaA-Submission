#include "Mandelbrot2.h"
#include "complex_amp.h"

Mandelbrot2::Mandelbrot2(Input *in)
{
	// Store pointer for input class
	input = in;

	//OpenGL settings
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.39f, 0.58f, 93.0f, 1.0f);			// Cornflour Blue Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glClearStencil(0);									// Clear stencil buffer
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	glEnable(GL_TEXTURE_2D);
	// Other OpenGL / render setting should be applied here.

	// Initialise variables
	initVariables();

	// Initialise texutre
	initTexture();
}

Mandelbrot2::~Mandelbrot2()
{
	mandelbrot_timings_file.close();
}

// Render the scene
void Mandelbrot2::render()
{
	// Clear Color and Depth Buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Reset transformations
	glLoadIdentity();
	// Set the camera
	gluLookAt(0.0f, 0.0f, 6.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Render geometry/scene here -------------------------------------
	
	generateQuad();

	// End render geometry --------------------------------------

	// Render text, should be last object rendered.
	renderTextOutput();

	// Swap buffers, after all objects are rendered.
	glutSwapBuffers();
} // render

// Update the scene
void Mandelbrot2::update(float dt)
{
	setResolution();

	alterMovementModifierByZoomLevel();

	userMovement();

	userZoom();

	setIterations();

	setColour();

	// update scene related variables.
	if (recalculate)
	{
		// Start timing
		the_amp_clock::time_point start = the_amp_clock::now();

		//gpu_amp_mandelbrot(((-2.0f * zoom_) + X_Modifier_), ((1.0f *zoom_) + X_Modifier_), ((1.125f * zoom_) + Y_Modifier_), ((-1.125f * zoom_) + Y_Modifier_)); // full
		
		gpu_amp_mandelbrot_tiled(((-2.0f * zoom_) + X_Modifier_), ((1.0f *zoom_) + X_Modifier_), ((1.125f * zoom_) + Y_Modifier_), ((-1.125f * zoom_) + Y_Modifier_)); // full

		// Stop timing
		the_amp_clock::time_point end = the_amp_clock::now();

		// Compute the difference between the two times in milliseconds
		auto time_taken = duration_cast<milliseconds>(end - start).count();

		mandelbrot_timings_file << "Resolution Width: " << "," << WIDTH << endl;
		mandelbrot_timings_file << "Resolution Height: "  << "," << HEIGHT << endl;
		mandelbrot_timings_file << "Max Iterations: " << "," << MAX_ITERATIONS << endl;
		mandelbrot_timings_file << "Time taken: " << "," << time_taken << endl;
		mandelbrot_timings_file << endl;

		//gpu_amp_mandelbrot(((-0.751085f * zoom_) + X_Modifier_), ((-0.734975f *zoom_) + X_Modifier_), ((0.118378f * zoom_) + Y_Modifier_), ((0.134488f * zoom_) + Y_Modifier_)); // zoomed

		recalculate = false;
		glTexImage2D(GL_TEXTURE_2D, 0, 4, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image);
	}

	// Calculate FPS for output
	calculateFPS();
} // update

// Resize the window and scene components if user rescales window
void Mandelbrot2::resize(int w, int h)
{
	window_width = w;
	window_height = h;
	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if (h == 0)
		h = 1;

	float ratio = (float)w / (float)h;
	fov = 45.0f;
	nearPlane = 0.1f;
	farPlane = 100.0f;

	// Use the Projection Matrix
	glMatrixMode(GL_PROJECTION);

	// Reset Matrix
	glLoadIdentity();

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the correct perspective.
	gluPerspective(fov, ratio, nearPlane, farPlane);

	// Get Back to the Modelview
	glMatrixMode(GL_MODELVIEW);
} // resize

// Report all the characteristics of an accelerator
void Mandelbrot2::report_accelerator(const accelerator a)
{
	const std::wstring bs[2] = { L"false", L"true" };
	std::wcout << ": " << a.description << " "
		<< endl << "       device_path                       = " << a.device_path
		<< endl << "       dedicated_memory                  = " << std::setprecision(4) << float(a.dedicated_memory) / (1024.0f * 1024.0f) << " Mb"
		<< endl << "       has_display                       = " << bs[a.has_display]
		<< endl << "       is_debug                          = " << bs[a.is_debug]
		<< endl << "       is_emulated                       = " << bs[a.is_emulated]
		<< endl << "       supports_double_precision         = " << bs[a.supports_double_precision]
		<< endl << "       supports_limited_double_precision = " << bs[a.supports_limited_double_precision]
		<< endl;
} // report_accelerator

// List and select the accelerator to use
void Mandelbrot2::list_accelerators()
{
	//get all accelerators available to us and store in a vector so we can extract details
	std::vector<accelerator> accls = accelerator::get_all();

	// iterates over all accelerators and print characteristics
	for (int i = 0; i<accls.size(); i++)
	{
		accelerator a = accls[i];
		report_accelerator(a);

	}
	//accelerator::set_default(accls[1].device_path); // set default accelerator to GPU
	// [0] = NVIDIA GeForce 440 - Vita Lab, Intel (R) HD Graphics - 4506
	// [1] = Microsoft Basic Render Driver - Not support limited_double_precision
	// [2] = Software Adaptor - Blank Screen
	// [3] = CPU Accelerator - Not support parallel_for_each
	accelerator acc = accelerator(accelerator::default_accelerator);
	std::wcout << " default acc = " << acc.description << endl;
} // list_accelerators

// query if AMP accelerator exists on hardware
void Mandelbrot2::query_AMP_support()
{
	std::vector<accelerator> accls = accelerator::get_all();
	if (accls.empty())
	{
		cout << "No accelerators found that are compatible with C++ AMP" << std::endl;
	}
	else
	{
		cout << "Accelerators found that are compatible with C++ AMP" << std::endl;
		list_accelerators();
	}
} // query_AMP_support

// Generate mandelbrot set on GPU using amp
void Mandelbrot2::gpu_amp_mandelbrot(float left_, float right_, float top_, float bottom_)
{
	unsigned max_iter = MAX_ITERATIONS;
	unsigned w = WIDTH;
	unsigned h = HEIGHT;
	uint32_t *pImage = &(image[0][0]);
	unsigned r = red;
	unsigned g = green;
	unsigned b = blue;

	array_view<uint32_t, 2> a(h, w, pImage);
	a.discard_data();

	try
	{
		parallel_for_each(a.extent, [=](index<2> idx) restrict(amp)
		{
			//USE THREAD ID/INDEX TO MAP INTO THE COMPLEX PLANE
			int x = idx[1];
			int y = idx[0];

			// Work out the point in the complex plane that
			// corresponds to this pixel in the output image.
			Complex1 c = { left_ + (x * (right_ - left_) / w), top_ + (y * (bottom_ - top_) / h) };

			// Start off z at (0, 0).
			Complex1 z = { 0.0, 0.0 };

			// Iterate z = z^2 + c until z moves more than 2 units
			// away from (0, 0), or we've iterated too many times.
			int iterations = 0;
			while (c_abs(z) < 2.0 && iterations < max_iter)
			{
				z = c_add(c_mul(z, z), c);

				++iterations;
			}

			if (iterations == max_iter)
			{
				// z didn't escape from the circle.
				// This point is in the Mandelbrot set.
				a[y][x] = 0x000000; // black
			}
			else
			{
				// z escaped within less than MAX_ITERATIONS
				// iterations. This point isn't in the set.
				//a[y][x] = 0xFFFFFF; // white
				a[y][x] = (b * iterations << 16) | (g * iterations << 8) | r * iterations; // grayscale
				// BGR
			}
		});
		a.synchronize();
	}
	catch (const std::exception& ex)
	{
		MessageBoxA(NULL, ex.what(), "Error", MB_ICONERROR);
	}
} // gpu_amp_mandelbrot

  // Generate mandelbrot set on GPU using amp with tiles
void Mandelbrot2::gpu_amp_mandelbrot_tiled(float left_, float right_, float top_, float bottom_)
{
	unsigned max_iter = MAX_ITERATIONS;
	unsigned w = WIDTH;
	unsigned h = HEIGHT;
	uint32_t *pImage = &(image[0][0]);
	unsigned r = red;
	unsigned g = green;
	unsigned b = blue;
	extent<2> e(h, w);
	array_view<uint32_t, 2> a(e, pImage);
	a.discard_data();

	try
	{
		parallel_for_each(a.extent.tile<TS, TS>(), [=](tiled_index<TS,TS> t_idx) restrict(amp)
		{
			//USE THREAD ID/INDEX TO MAP INTO THE COMPLEX PLANE
			index<2> idx = t_idx.global;
			int x = t_idx.global[1];
			int y = t_idx.global[0];

			 /*Work out the point in the complex plane that
			 corresponds to this pixel in the output image.*/
			Complex1 c = { left_ + (x * (right_ - left_) / w), top_ + (y * (bottom_ - top_) / h) };

			// Start off z at (0, 0).
			Complex1 z = { 0.0, 0.0 };

			// Iterate z = z^2 + c until z moves more than 2 units
			// away from (0, 0), or we've iterated too many times.
			int iterations = 0;
			while (c_abs(z) < 2.0 && iterations < max_iter)
			{
				z = c_add(c_mul(z, z), c);

				++iterations;
			}

			if (iterations == max_iter)
			{
				// z didn't escape from the circle.
				// This point is in the Mandelbrot set.
				a[y][x] = 0x000000; // black
			}
			else
			{
				// z escaped within less than MAX_ITERATIONS
				// iterations. This point isn't in the set.
				//a[y][x] = 0xFFFFFF; // white
				a[y][x] = (b * iterations << 16) | (g * iterations << 8) | r * iterations; // grayscale
																						   // BGR
			}
		});
		a.synchronize();
	}
	catch (const std::exception& ex)
	{
		MessageBoxA(NULL, ex.what(), "Error", MB_ICONERROR);
	}
} // gpu_amp_mandelbrot_tiled

// Display text within the scene
void Mandelbrot2::displayText(float x, float y, float r, float g, float b, char * string)
{
	// Get Lenth of string
	int j = strlen(string);

	// Swap to 2D rendering
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, 5, 100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	// Orthographic lookAt (along the z-axis).
	gluLookAt(0.0f, 0.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Set text colour and position.
	glColor3f(r, g, b);
	glRasterPos2f(x, y);
	// Render text.
	for (int i = 0; i < j; i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, string[i]);
	}
	// Reset colour to white.
	glColor3f(1.f, 1.f, 1.f);

	// Swap back to 3D rendering.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, ((float)window_width / (float)window_height), nearPlane, farPlane);
	glMatrixMode(GL_MODELVIEW);
} // displayText

// Render the text to the screen
void Mandelbrot2::renderTextOutput()
{
	// Render current mouse position and frames per second.
	sprintf_s(mouseText, "Mouse: %i, %i", input->getMouseX(), input->getMouseY());
	displayText(-1.f, 0.96f, 1.f, 1.f, 1.f, mouseText);
	displayText(-1.f, 0.90f, 1.f, 1.f, 1.f, fps);

	// Render width value
	sprintf_s(widthText, "Width: %i", WIDTH);
	displayText(-1.f, 0.84f, 1.f, 1.f, 1.f, widthText);
	// Render height value
	sprintf_s(heightText, "Height: %i", HEIGHT);
	displayText(-1.f, 0.78f, 1.f, 1.f, 1.f, heightText);

	// Render max iterations value
	sprintf_s(iterationText, "Max_Iter: %i", MAX_ITERATIONS);
	displayText(-1.f, 0.72f, 1.f, 1.f, 1.f, iterationText);

	// Render zoom value
	sprintf_s(zoomText, "Zoom: %f", zoom_);
	displayText(-1.f, 0.66f, 1.f, 1.f, 1.f, zoomText);

	// Render blue value
	sprintf_s(blueText, "Blue: %i", blue);
	displayText(-1.f, 0.60f, 1.f, 1.f, 1.f, blueText);
	// Render green value
	sprintf_s(greenText, "Green: %i", green);
	displayText(-1.f, 0.54f, 1.f, 1.f, 1.f, greenText);
	// Render red value
	sprintf_s(redText, "Red: %i", red);
	displayText(-1.f, 0.48f, 1.f, 1.f, 1.f, redText);
} // renderTextOutput

// Calculate FPS
void Mandelbrot2::calculateFPS()
{
	frame++;
	time = glutGet(GLUT_ELAPSED_TIME);

	if (time - timebase > 1000) {
		sprintf_s(fps, "FPS: %4.2f", frame*1000.0 / (time - timebase));
		timebase = time;
		frame = 0;
	}
} // calculateFPS

// Initialise scene variables
void Mandelbrot2::initVariables()
{
	MAX_ITERATIONS = 500; // 500 - starter, 10000 - beautiful
	iteration_modifier_ = MAX_ITERATIONS;
	recalculate = true;
	X_Modifier_ = 0;
	Y_Modifier_ = 0;
	zoom_ = 1.0f;
	movement_modifier_ = 0.005f;
	mandelbrot_timings_file.open("amp_mandelbrot_timings.csv");
	red = 1;
	green = 1;
	blue = 1;
} // initVariables

// Initialise the texture which the mandelbrot set will be rendered to
void Mandelbrot2::initTexture()
{
	glGenTextures(1, &mandelbrotTexture);
	glBindTexture(GL_TEXTURE_2D, mandelbrotTexture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
} // initTexture

// Generate a 2x2 quad and scale it to the window size
void Mandelbrot2::generateQuad()
{
	glPushMatrix();

	glScalef(2.5f, 2.48f, 0.0f);

	glBegin(GL_QUADS);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 0.0f); // bottom right

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, 0.0f); // top right

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, 0.0f); // top left

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 0.0f); // bottom left

	glEnd();

	glPopMatrix();
} // generateQuad

// Set the WIDTH/HEIGHT the mandelbrot set will be calculated with based on key presses
void Mandelbrot2::setResolution()
{
	// 640x480
	if (input->isKeyDown('1'))
	{
		if (WIDTH != 640 && HEIGHT != 480)
		{
			WIDTH = 640;
			HEIGHT = 480;
			recalculate = true;
		}
		input->SetKeyUp('1');
	}
	// 960x720
	if (input->isKeyDown('2'))
	{
		if (WIDTH != 960 && HEIGHT != 720)
		{
			WIDTH = 960;
			HEIGHT = 720;
			recalculate = true;
		}
		input->SetKeyUp('2');
	}
	// 1280x960
	if (input->isKeyDown('3'))
	{
		if (WIDTH != 1280 && HEIGHT != 960)
		{
			WIDTH = 1280;
			HEIGHT = 960;
			recalculate = true;
		}
		input->SetKeyUp('3');
	}
	// 1920x1280
	if (input->isKeyDown('4'))
	{
		if (WIDTH != 1920 && HEIGHT != 1280)
		{
			WIDTH = 1920;
			HEIGHT = 1280;
			recalculate = true;
		}
		input->SetKeyUp('4');
	}
} // setResolution

// Set the movement modifier to be smaller/larger depending on zoom level
void Mandelbrot2::alterMovementModifierByZoomLevel()
{
	// Set a more precise movement variable depending on zoom level
	if (zoom_ > 0.5f)
	{
		movement_modifier_ = 0.05f;
	}
	if (zoom_ < 0.05f && zoom_ > 0.005f)
	{
		movement_modifier_ = 0.005f;
	}
	if (zoom_ < 0.005f && zoom_ > 0.00005f)
	{
		movement_modifier_ = 0.00005f;
	}
	if (zoom_ < 0.00005f)
	{
		movement_modifier_ = 0.000005f;
	}
} // alterMovementModifierByZoomLevel

// Detect key presses by user to move around the mandelbrot set and recalculate upon moving
void Mandelbrot2::userMovement()
{
	// move left
	if (input->isKeyDown('a'))
	{
		X_Modifier_ -= movement_modifier_;

		recalculate = true;
		input->SetKeyUp('a');
	}
	// move right
	if (input->isKeyDown('d'))
	{
		X_Modifier_ += movement_modifier_;
		recalculate = true;
		input->SetKeyUp('d');
	}
	// move up
	if (input->isKeyDown('w'))
	{
		Y_Modifier_ += movement_modifier_;
		recalculate = true;
		input->SetKeyUp('w');
	}
	// move down
	if (input->isKeyDown('s'))
	{
		Y_Modifier_ -= movement_modifier_;
		recalculate = true;
		input->SetKeyUp('s');
	}
} // userMovement

// Detect key presses by user to alter the zoom variable and magnify/minify the mandelbrot set and recalculate
void Mandelbrot2::userZoom()
{
	// zoom in
	if (input->isKeyDown('r'))
	{
		zoom_ -= (zoom_ / 10);
		recalculate = true;
		input->SetKeyUp('r');
	}
	// zoom out
	if (input->isKeyDown('f'))
	{
		zoom_ += (zoom_ / 10);
		recalculate = true;
		input->SetKeyUp('f');
	}
} // userZoom

// Detect key presses by user to alter the MAX_ITERATIONS the mandelbrot set is calculated with
void Mandelbrot2::setIterations()
{
	// increase MAX_ITERATIONS
	if (input->isKeyDown('q'))
	{
		if (MAX_ITERATIONS < 10000)
		{
			MAX_ITERATIONS += iteration_modifier_;
			recalculate = true;
		}
		input->SetKeyUp('q');
	}
	// decrease MAX_ITERATIONS
	if (input->isKeyDown('e'))
	{
		if (MAX_ITERATIONS > 0)
		{
			MAX_ITERATIONS -= iteration_modifier_;
			recalculate = true;
		}
		input->SetKeyUp('e');
	}
} // setIterations

// Detect key presses by user to alter the colour the mandelbrot set is calculated with
void Mandelbrot2::setColour()
{
	// increase blue
	if (input->isKeyDown('t'))
	{
		if (blue < 255)
		{
			blue++;
			recalculate = true;
		}
		input->SetKeyUp('t');
	}
	//decrease blue
	if (input->isKeyDown('g'))
	{
		if (blue > 1)
		{
			blue--;
			recalculate = true;
		}
		input->SetKeyUp('g');
	}

	//increase green
	if (input->isKeyDown('y'))
	{
		if (green < 255)
		{
			green++;
			recalculate = true;
		}
		input->SetKeyUp('y');
	}
	//decrease green
	if (input->isKeyDown('h'))
	{
		if (green > 1)
		{
			green--;
			recalculate = true;
		}
		input->SetKeyUp('h');
	}

	//increase red
	if (input->isKeyDown('u'))
	{
		if (red < 255)
		{
			red++;
			recalculate = true;
		}
		input->SetKeyUp('u');
	}
	//decrease red
	if (input->isKeyDown('j'))
	{
		if (red > 1)
		{
			red--;
			recalculate = true;
		}
		input->SetKeyUp('j');
	}
} // setColour