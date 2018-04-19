#include "mandelbrot.h"
#include "complex_amp.h"
#include "quad.h"

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
	glEnable(GL_TEXTURE_2D);
	pixel_amp_mandelbrot_.reserve(SIZE * 3);
	r_ = 250;
	g_ = 68;
	b_ = 32;

	MAX_ITERATIONS = 500; // 500 - starter, 10000 - beautiful

	recalculate = true;

	left_ = right_ = top_ = bottom_ = 0;
	zoom_ = 1.0f;

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

	glPushMatrix(); {
		glScalef(2.5f,2.48f,1.f);
		// render Mandelbrot
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glVertexPointer(3, GL_FLOAT, 0, quad_t_verts.data());
		glTexCoordPointer(2, GL_FLOAT, 0, quad_t_texcoords.data());

		glGenTextures(1, &amp_mandelbrot_texture_);
		glBindTexture(GL_TEXTURE_2D, amp_mandelbrot_texture_);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, WIDTH, HEIGHT,
			0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pixel_amp_mandelbrot_.data()); // <----- had to use GL_BGR_EXT

																			//glColor4f(_rgba.getR(), _rgba.getG(), _rgba.getB(), _rgba.getA());
		glDrawArrays(GL_TRIANGLES, 0, quad_t_verts.size() / 3);
		//glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glBindTexture(GL_TEXTURE_2D, NULL);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	} glPopMatrix();

	/*GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image_amp_mandelbrot_);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);*/


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
	if (input->isKeyDown('a'))
	{
		//Wireframe = true;
		//glPolygonMode(GL_FRONT, GL_LINE);
		left_ -= 0.1f;
		right_ -= 0.1f;
		recalculate = true;
		input->SetKeyUp('a');
	}
	if (input->isKeyDown('d'))
	{
		//glPolygonMode(GL_FRONT, GL_FILL);
		left_ += 0.1f;
		right_ += 0.1f;
		recalculate = true;
		input->SetKeyUp('d');
	}
	if (input->isKeyDown('w'))
	{
		//Wireframe = true;
		//glPolygonMode(GL_FRONT, GL_LINE);
		top_ += 0.1f;
		bottom_ += 0.1f;
		recalculate = true;
		input->SetKeyUp('w');
	}
	if (input->isKeyDown('s'))
	{
		//glPolygonMode(GL_FRONT, GL_FILL);
		top_ -= 0.1f;
		bottom_ -= 0.1f;
		recalculate = true;
		input->SetKeyUp('s');
	}
	if (input->isKeyDown('r'))
	{
		//Wireframe = true;
		//glPolygonMode(GL_FRONT, GL_LINE);
		zoom_ -= 0.1f;
		recalculate = true;
		input->SetKeyUp('r');
	}
	if (input->isKeyDown('t'))
	{
		//glPolygonMode(GL_FRONT, GL_FILL);
		zoom_ += 0.1f;
		recalculate = true;
		input->SetKeyUp('t');
	}
	// update scene related variables.
	if (recalculate)
	{
		compute_mandelbrot_amp((-2.0f + left_)*zoom_, (1.0f + right_)*zoom_, (1.125f + top_)*zoom_, (-1.125f + bottom_)*zoom_);
		//compute_mandelbrot_amp(-0.751085f, -0.734975f, 0.118378f, 0.134488f);
		recalculate = false;
	}

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
	accelerator::set_default(accls[0].device_path);
	// [0] = NVIDIA GeForce 440 - Works
	// [1] = Microsoft Basic Render Driver - Not support limited_double_precision
	// [2] = Software Adaptor - Blank Screen
	// [3] = CPU Accelerator - Not support parallel_for_each
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

void mandelbrot::compute_mandelbrot_amp(float left_, float right_, float top_, float bottom_)
{
	unsigned max_iter = MAX_ITERATIONS;
	unsigned r = r_;
	unsigned g = g_;
	unsigned b = b_;

	image_amp_mandelbrot_.empty();
	
	extent<2> e(WIDTH, HEIGHT);
	array_view<uint32_t, 2> array_view(e, image_amp_mandelbrot_);
	array_view.discard_data();

	try
	{
		parallel_for_each(array_view.extent.tile<TiS, TiS>(),[=](tiled_index<TiS, TiS> t_idx)mutable restrict(amp) {
			index<2> idx = t_idx.global;

			// Work out the point in the complex plane that
			// corresponds to this pixel in the output image.
			Complex1 c = { left_ + (idx[0] * (right_ - left_) / WIDTH), top_ + (idx[1] * (bottom_ - top_) / HEIGHT) };

			// Start off z at (0, 0).
			Complex1 z = { 0.0f, 0.0f };

			// Iterate z = z^2 + c until z moves more than 2 units
			// away from (0, 0), or we've iterated too many times.
			int iterations = 0;
			while (c_abs(z) < 4.0 && iterations < max_iter)
			{
				z = c_add(c_mul(z, z), c); //c_mul(z, z)

				++iterations;
			}

			if (iterations == max_iter)
			{
				// z didn't escape from the circle.
				// This point is in the Mandelbrot set.
				//array_view[1][x] = 0x000000; // black
				r = 0;
				g = 0;
				b = 0;
			}
			else
			{
				// z escaped within less than MAX_ITERATIONS
				// iterations. This point isn't in the set.
				r = iterations * iterations * r;
				g = iterations * iterations * g;
				b = iterations * iterations * b;

				//r = (r * iterations * iterations << 16);
				//g = (g * iterations * iterations << 8); //***REALLY COOL***
				//b = (b * iterations * iterations);

				/*r = (iterations << 16);
				g = (iterations << 8);
				b = (iterations);*/
			}
			array_view[idx] = (r << 16) | (g << 8) | (b);
		});
		array_view.synchronize();
	}
	catch (const std::exception& ex)
	{
		MessageBoxA(NULL, ex.what(), "Error", MB_ICONERROR);
	}
	// calculate pixel image
	auto pixel_image = std::async(std::launch::async, [&]()
	{
		pixel_amp_mandelbrot_.clear();
		// generating pixel vector with mandelbrot image 
		for (int y = 0; y < HEIGHT; ++y)
		{
			for (int x = 0; x < WIDTH; ++x)
			{
				pixel_amp_mandelbrot_.push_back((image_amp_mandelbrot_[x * HEIGHT + y]) & 0xFF); // blue channel
				pixel_amp_mandelbrot_.push_back((image_amp_mandelbrot_[x * HEIGHT + y] >> 8) & 0xFF); // green channel
				pixel_amp_mandelbrot_.push_back((image_amp_mandelbrot_[x * HEIGHT + y] >> 16) & 0xFF); // red channel
			}
		}
	});
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
