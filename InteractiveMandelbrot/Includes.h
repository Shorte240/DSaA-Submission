#pragma once

// Include glut, opengl libraries and custom classes
#include "glut.h"
#include <gl/GL.h>
#include <gl/GLU.h>
#include "Input.h"
#include <stdio.h>

// Further includes should go here:
#include "SOIL.h"
#include <vector>

// Mandelbrot specific includes
#include <chrono>
#include <iostream>
#include <iomanip>
#include <amp.h>
#include <time.h>
#include <string>
#include <fstream>
#include <array>


// Need to access the concurrency libraries 
using namespace concurrency;

// Import things we need from the standard library
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::vector;
using std::cout;
using std::endl;
using std::ofstream;
using std::thread;
using namespace concurrency;

// Define the alias "the_clock" for the clock type we're going to use.
typedef std::chrono::steady_clock the_serial_clock;
typedef std::chrono::steady_clock the_amp_clock;
