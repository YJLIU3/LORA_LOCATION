#pragma OPENCL EXTENSION cl_khr_fp64 : enable
__kernel void affine(double a1, double a2, double a3, double b1, double b2, double b3,
	double mid_x1, double mid_x2, double mid_x3, double mid_y1, double mid_y2, double mid_y3,
	float theta1, float theta2, float theta3,
	__global float* sigema1, __global float* sigema2, __global float* sigema3,
	__global float* outx, __global float* outy )
{
	int id1 = get_global_id(0);
	int id2 = get_global_id(1);
	int id3 = get_global_id(2);

	float x1 = a1 / cos(sigema1[id1]);
	float y1 = b1*tan(sigema1[id1]);
	float x2 = a2 / cos(sigema2[id2]);
	float y2 = b2*tan(sigema2[id2]);
	float x3 = a3 / cos(sigema2[id3]);
	float y3 = b3*tan(sigema3[id3]);

	outx[id] = x*cos(theta) - y*sin(theta) + mid_x;
	outy[id] = x*sin(theta) + y*cos(theta) + mid_y;
	if (id>6280)
		printf("%d---outy=%f outx=%f\t", id, outy[id], outx[id]);
}