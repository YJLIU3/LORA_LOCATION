#pragma OPENCL EXTENSION cl_khr_fp64 : enable
__kernel void affine(double a, double b, double mid_x, double mid_y, float theta, __global float* sigema, __global float* outx, __global float* outy )
{
	int id = get_global_id(0);


	
	 float x = a / cos(sigema[id]);
	 float y = b*tan(sigema[id]);

	// printf("---%f%fcos=%f tan=%f sigma=%f %f %f--- \n", a,b,cos(sigema[id]), tan(sigema[id]), sigema[id],outx[id], outy[id]);
	//printf("%lf %lf \n", cos(sigema[id]), tan(sigema[id]));
	//printf("%lf %lf \n", x, y);

	outx[id] = x*cos(theta) - y*sin(theta) + mid_x;
	outy[id] = x*sin(theta) + y*cos(theta) + mid_y;
	if (id>6280)
		printf("%d---outy=%f outx=%f\t", id, outy[id], outx[id]);
}