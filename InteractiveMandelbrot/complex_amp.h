#pragma once
#include <amp_math.h>

using namespace concurrency;

// using our own structure as Complex function not available in the Concurrency namespace
struct Complex1 {
	float x;
	float y;
};

Complex1 c_add(Complex1 c1, Complex1 c2) restrict(cpu, amp) // restrict keyword - able to execute this function on the GPU and CPU
{
	Complex1 tmp;
	float a = c1.x;
	float b = c1.y;
	float c = c2.x;
	float d = c2.y;
	tmp.x = a + c;
	tmp.y = b + d;
	return tmp;
} // c_add

float c_abs(Complex1 c) restrict(cpu, amp)
{
	return fast_math::sqrt(c.x*c.x + c.y*c.y);
} // c_abs

Complex1 c_mul(Complex1 c1, Complex1 c2) restrict(cpu, amp)
{
	Complex1 tmp;
	float a = c1.x;
	float b = c1.y;
	float c = c2.x;
	float d = c2.y;
	tmp.x = a*c - b*d;
	tmp.y = b*c + a*d;
	return tmp;
} // c_mul
