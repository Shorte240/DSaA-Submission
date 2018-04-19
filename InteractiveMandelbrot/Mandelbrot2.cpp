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
	MAX_ITERATIONS = 500; // 500 - starter, 10000 - beautiful
	recalculate = true;

	left_ = right_ = top_ = bottom_ = 0;
	zoom_ = 1.0f;

	// Initialise variables
}

void Mandelbrot2::render()
{
	// Clear Color and Depth Buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Reset transformations
	glLoadIdentity();
	// Set the camera
	gluLookAt(0.0f, 0.0f, 6.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Render geometry/scene here -------------------------------------
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glScalef(2.5f,2.48f,0.0f);

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
	// End render geometry --------------------------------------

	// Render text, should be last object rendered.
	renderTextOutput();

	// Swap buffers, after all objects are rendered.
	glutSwapBuffers();
}

void Mandelbrot2::update(float dt)
{
	if (input->isKeyDown('a'))
	{
		left_ -= 0.1f;
		right_ -= 0.1f;
		recalculate = true;
		input->SetKeyUp('a');
	}
	if (input->isKeyDown('d'))
	{
		left_ += 0.1f;
		right_ += 0.1f;
		recalculate = true;
		input->SetKeyUp('d');
	}
	if (input->isKeyDown('w'))
	{
		top_ += 0.1f;
		bottom_ += 0.1f;
		recalculate = true;
		input->SetKeyUp('w');
	}
	if (input->isKeyDown('s'))
	{
		top_ -= 0.1f;
		bottom_ -= 0.1f;
		recalculate = true;
		input->SetKeyUp('s');
	}
	if (input->isKeyDown('r'))
	{
		zoom_ -= 0.1f;
		recalculate = true;
		input->SetKeyUp('r');
	}
	if (input->isKeyDown('t'))
	{
		if (zoom_ <= 1.0f)
		{
			zoom_ += 0.1f;
		}
		recalculate = true;
		input->SetKeyUp('t');
	}
	// update scene related variables.
	if (recalculate)
	{
		compute_mandelbrot_amp((-2.0f + left_)*zoom_, (1.0f + right_)*zoom_, (1.125f + top_)*zoom_, (-1.125f + bottom_)*zoom_);
		//compute_mandelbrot_amp((-0.751085f + left_)*zoom_, (-0.734975f + right_)*zoom_, (0.118378f + top_)*zoom_, (0.134488f + bottom_)*zoom_);
		recalculate = false;
	}

	// Calculate FPS for output
	calculateFPS();
}

void Mandelbrot2::resize(int w, int h)
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
}
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
	accelerator::set_default(accls[0].device_path);
	// [0] = NVIDIA GeForce 440 - Works
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

void Mandelbrot2::compute_mandelbrot_amp(float left_, float right_, float top_, float bottom_)
{
	unsigned max_iter = MAX_ITERATIONS;
	uint32_t *pImage = &(image[0][0]);

	array_view<uint32_t, 2> a(HEIGHT, WIDTH, pImage);
	a.discard_data();

	try
	{
		parallel_for_each(a.extent, [=](concurrency::index<2> idx) restrict(amp)
		{
			//USE THREAD ID/INDEX TO MAP INTO THE COMPLEX PLANE
			int x = idx[1];
			int y = idx[0];

			// Work out the point in the complex plane that
			// corresponds to this pixel in the output image.
			Complex1 c = { left_ + (x * (right_ - left_) / WIDTH), top_ + (y * (bottom_ - top_) / HEIGHT) };

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
				a[y][x] = (iterations << 16) | (iterations << 8) | iterations; // grayscale
			}
		});
		a.synchronize();
	}
	catch (const std::exception& ex)
	{
		MessageBoxA(NULL, ex.what(), "Error", MB_ICONERROR);
	}
}

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
	gluPerspective(fov, ((float)width / (float)height), nearPlane, farPlane);
	glMatrixMode(GL_MODELVIEW);
}

void Mandelbrot2::renderTextOutput()
{
	// Render current mouse position and frames per second.
	sprintf_s(mouseText, "Mouse: %i, %i", input->getMouseX(), input->getMouseY());
	displayText(-1.f, 0.96f, 1.f, 0.f, 0.f, mouseText);
	displayText(-1.f, 0.90f, 1.f, 0.f, 0.f, fps);
}

void Mandelbrot2::calculateFPS()
{
	frame++;
	time = glutGet(GLUT_ELAPSED_TIME);

	if (time - timebase > 1000) {
		sprintf_s(fps, "FPS: %4.2f", frame*1000.0 / (time - timebase));
		timebase = time;
		frame = 0;
	}
}