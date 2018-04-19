#include "mandelbrot.h"
#include "complex_amp.h"

mandelbrot::mandelbrot(Input *in)
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

	// Other OpenGL / render setting should be applied here.


	// Initialise variables
}

void mandelbrot::render()
{
	// Clear Color and Depth Buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Reset transformations
	glLoadIdentity();
	// Set the camera
	gluLookAt(0.0f, 0.0f, 6.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	//Set colour to green
	//glColor3f(0.0f, 1.0f, 0.0f);

	//Turn on Wireframe
	//glPolygonMode(GL_FRONT, GL_LINE);

	// Render geometry/scene here -------------------------------------

//***Polygon***\\

	glBegin(GL_POLYGON);

	glColor3f(0.5f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);

	glColor3f(0.0f, 0.5f, 0.0f);
	glVertex3f(1.0f, 0.0f, 0.0f);

	glColor3f(0.0f, 0.0f, 0.5f);
	glVertex3f(1.5f, 1.0f, 0.0f);

	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(1.0f, 2.0f, 0.0f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 2.0f, 0.0f);

	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-0.5f, 1.0f, 0.0f);

	glEnd();


	// End render geometry --------------------------------------

	// Render text, should be last object rendered.
	renderTextOutput();

	// Swap buffers, after all objects are rendered.
	glutSwapBuffers();
}

void mandelbrot::update(float dt)
{
	bool Wireframe;
	// Handle user input
	if (input->isKeyDown('r'))
	{
		Wireframe = true;
		glPolygonMode(GL_FRONT, GL_LINE);
		input->SetKeyUp('r');
	}
	if (input->isKeyDown('t'))
	{
		glPolygonMode(GL_FRONT, GL_FILL);
		input->SetKeyUp('t');
	}
	// update scene related variables.

	// Calculate FPS for output
	calculateFPS();
}

void mandelbrot::resize(int w, int h)
{
	width = w;
	height = h;
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
}

void mandelbrot::report_accelerator(const accelerator a)
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
}
// List and select the accelerator to use
void mandelbrot::list_accelerators()
{
	//get all accelerators available to us and store in a vector so we can extract details
	std::vector<accelerator> accls = accelerator::get_all();

	// iterates over all accelerators and print characteristics
	for (int i = 0; i<accls.size(); i++)
	{
		accelerator a = accls[i];
		report_accelerator(a);

	}

	accelerator acc = accelerator(accelerator::default_accelerator);
	std::wcout << " default acc = " << acc.description << endl;
} // list_accelerators
  // query if AMP accelerator exists on hardware
void mandelbrot::query_AMP_support()
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

void mandelbrot::compute_mandelbrot_amp(float left_, float right_, float top_, float bottom_, std::vector<std::vector<uint32_t>>& image_)
{
	for (int y = 0; y < HEIGHT; y++)
	{
		concurrency::extent<2> e(WIDTH, 1);
		concurrency::array_view<uint32_t, 2> smol_brot(e, image_.at(y));
		smol_brot.discard_data();

		try
		{
			parallel_for_each(smol_brot.extent, [=](concurrency::index<2> idx) restrict(amp) {
				int x = idx[0];

				// Work out the point in the complex plane that
				// corresponds to this pixel in the output image.
				Complex1 c = { left_ + (x * (right_ - left_) / WIDTH), top_ + (y * (bottom_ - top_) / HEIGHT) };

				// Start off z at (0, 0).
				Complex1 z = { 0.0f, 0.0f };

				// Iterate z = z^2 + c until z moves more than 2 units
				// away from (0, 0), or we've iterated too many times.
				int iterations = 0;
				while (c_abs(z) < 2.0 && iterations < MAX_ITERATIONS)
				{
					z = c_add(c_mul(z, z), c);

					++iterations;
				}

				if (iterations == MAX_ITERATIONS)
				{
					// z didn't escape from the circle.
					// This point is in the Mandelbrot set.
					smol_brot[1][x] = 0x000000; // black
				}
				else
				{
					// z escaped within less than MAX_ITERATIONS
					// iterations. This point isn't in the set.
					smol_brot[1][x] = (iterations << 16) | (iterations << 8) | iterations;
				}

			});
		}
		catch (const std::exception& ex)
		{
			MessageBoxA(NULL, ex.what(), "Error", MB_ICONERROR);
		}
	}
}

void mandelbrot::displayText(float x, float y, float r, float g, float b, char * string)
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
	gluPerspective(fov, ((float)width / (float)height), nearPlane, farPlane);
	glMatrixMode(GL_MODELVIEW);
}

void mandelbrot::renderTextOutput()
{
	// Render current mouse position and frames per second.
	sprintf_s(mouseText, "Mouse: %i, %i", input->getMouseX(), input->getMouseY());
	displayText(-1.f, 0.96f, 1.f, 0.f, 0.f, mouseText);
	displayText(-1.f, 0.90f, 1.f, 0.f, 0.f, fps);
}

void mandelbrot::calculateFPS()
{
	frame++;
	time = glutGet(GLUT_ELAPSED_TIME);

	if (time - timebase > 1000) {
		sprintf_s(fps, "FPS: %4.2f", frame*1000.0 / (time - timebase));
		timebase = time;
		frame = 0;
	}
}