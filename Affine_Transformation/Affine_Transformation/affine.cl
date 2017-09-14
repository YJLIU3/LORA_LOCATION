#pragma OPENCL EXTENSION cl_khr_fp64 : enable
__kernel void affine(double a, double b, double mid_x, double mid_y, float theta, __global double* sigema, __global double* outx, __global double* outy )
{
	int id = get_global_id(0);


	
	 outx[id] = a / cos(sigema[id]);
	 outy[id] = b*tan(sigema[id]);
//	 if(id>6270)
	 printf("%d---sigma=% f\t", id, sigema[id]);
	// printf("---%f%fcos=%f tan=%f sigma=%f %f %f--- \n", a,b,cos(sigema[id]), tan(sigema[id]), sigema[id],outx[id], outy[id]);
	//printf("%lf %lf \n", cos(sigema[id]), tan(sigema[id]));
	//printf("%lf %lf \n", x, y);

	//outx[id] = x*cos(theta) - y*sin(theta) + mid_x;
	//outy[id] = x*sin(theta) + y*cos(theta) + mid_y;

}