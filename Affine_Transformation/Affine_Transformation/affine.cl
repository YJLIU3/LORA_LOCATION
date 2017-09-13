#pragma OPENCL EXTENSION cl_khr_fp64 : enable
__kernel void affine(double a, double b, double mid_x, double mid_y, float theta, __global double* sigema, __global double* outx, __global double* outy )
{
	int id = get_global_id(0);


	
	double x = a / cos(sigema[id]);
	double y = b*tan(sigema[id]);

//	printf("%lf %lf \n", x, y);

	//Affine traslation
	outx[id] = x*cos(theta) - y*sin(theta) + mid_x;
	outy[id] = x*sin(theta) + y*cos(theta) + mid_y;

}