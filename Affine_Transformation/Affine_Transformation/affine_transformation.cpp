#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string>
#include <CL\cl.h>
using namespace std;
#pragma warning(disable:4996)

#define PI	3.141592653




float get_theta(double x1, double x2, double y1, double y2)
{
	float theta;
	if (x1 - x2 == 0)
		return PI / 2;
	else
	{
		float k = (y2 - y1) / (x2 - x1);
		theta = atan(k);
	}

}



bool GetFileData(const char* fname, string& str)
{
	FILE* fp = fopen(fname, "r");
	if (fp == NULL)
	{
		printf("no found file\n");
		return false;
	}

	int n = 0;
	while (feof(fp) == 0)
	{
		str += fgetc(fp);
	}

	return true;
}

int CL_init()
{
	double C = 300000000.0;

	//Input data to get Hyperbola
	double t1 = 5.0 / 3000000.0, t2 = 5.0 / 3000000.0;
	double X1 = 0.0, Y1 = 0.0, X2 = 0.0, Y2 = 1000.0;
	
	double delta_t;
	if (t1 > t2)
		delta_t = t1 - t2;
	else
		delta_t = t2 - t1;

	double delta_D1 = delta_t*C;

	printf("delta = %lf\n",delta_D1);

	//Hyperbola paramaters
	double a = delta_D1 / 2;

	double focus = sqrt((X1 - X2)*(X1 - X2) + (Y1 - Y2)*(Y1 - Y2))/2.0;

	double b = sqrt(focus*focus - a*a);

	//midpoint to get affine translate
	double midle_x = (X1 + X2) / 2;
	double midle_y = (Y1 + Y2) / 2;

	float theta = get_theta(X1, X2, Y1, Y2);
	printf("theta = %lf\n", theta);
	

	string code_file;

	if (false == GetFileData("affine.cl", code_file))
		return 0;

	char* buf_code = new char[code_file.size()];
	strcpy(buf_code, code_file.c_str());
	buf_code[code_file.size() - 1] = NULL;

	cl_device_id device;
	cl_platform_id platform_id = NULL;
	cl_context context;
	cl_command_queue cmdQueue;
	cl_mem buffer_sigma, buffer_outx, buffer_outy;
	cl_program program;
	cl_kernel kernel = NULL;

	size_t globalWorkSize[1];
	globalWorkSize[0] =6283;

	cl_int err;

	int number_of_point = 6238;

	float *sigma = new float[number_of_point];
	float *buf_out_x = new float[number_of_point];
	float *buf_out_y = new float[number_of_point];

	size_t datasize = sizeof(float)*number_of_point;

	for (int i = 0; i < number_of_point; i++)
	{
		sigma[i] = i/1000.0;
		printf("%lf--i=%d\t", sigma[i],i);
		if ((i + 1) / 6 == 0)
			printf("\n");
	}

	err = clGetPlatformIDs(1, &platform_id, NULL);

	if (err != CL_SUCCESS)
	{
		printf("clGetPlatformIDs error\n");
		return 0;
	}
	clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device, NULL);

	context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);

	cmdQueue = clCreateCommandQueue(context, device, 0, NULL);

	buffer_sigma = clCreateBuffer(context, CL_MEM_READ_ONLY, datasize, NULL, NULL);

	buffer_outx = clCreateBuffer(context, CL_MEM_WRITE_ONLY, datasize, NULL, NULL);
	buffer_outy = clCreateBuffer(context, CL_MEM_WRITE_ONLY, datasize, NULL, NULL);

	clEnqueueWriteBuffer(cmdQueue, buffer_sigma, CL_TRUE, 0, datasize, sigma, 0, NULL, NULL);

	program = clCreateProgramWithSource(context, 1, (const char**)&buf_code, NULL, NULL);


	cl_device_fp_config DeviceDouble;
	cl_int fff = clGetDeviceInfo(device, CL_DEVICE_DOUBLE_FP_CONFIG, sizeof(cl_device_fp_config),&DeviceDouble,NULL);

	printf("device if support fp double : %d\n", DeviceDouble==0?0:1);

	clBuildProgram(program, 1, &device, "-D FP_64", NULL, NULL);


	kernel = clCreateKernel(program, "affine", NULL);



	clSetKernelArg(kernel, 0, sizeof(cl_double), &a);
	clSetKernelArg(kernel, 1, sizeof(cl_double), &b);
	clSetKernelArg(kernel, 2, sizeof(cl_double), &midle_x);
	clSetKernelArg(kernel, 3, sizeof(cl_double), &midle_y);
	clSetKernelArg(kernel, 4, sizeof(cl_float), &theta);
	clSetKernelArg(kernel, 5, sizeof(cl_mem), &buffer_sigma);

	clSetKernelArg(kernel, 6, sizeof(cl_mem), &buffer_outx);
	clSetKernelArg(kernel, 7, sizeof(cl_mem), &buffer_outy);



	clEnqueueNDRangeKernel(cmdQueue, kernel, 1, NULL, globalWorkSize, NULL, 0, NULL, NULL);

	clEnqueueReadBuffer(cmdQueue, buffer_outx, CL_TRUE, 0, datasize, buf_out_x, 0, NULL, NULL);
	clEnqueueReadBuffer(cmdQueue, buffer_outy, CL_TRUE, 0, datasize, buf_out_y, 0, NULL, NULL);

	//FILE *fp;
	//fp = fopen("hyperbola.xls", "w");
	//for (int i = 0; i < number_of_point; i++)
	//{
	//	printf( "%f\t,%f\n", buf_out_x[i], buf_out_y[i]);
	//	fprintf(fp,"%f\t,%f\n", buf_out_x[i], buf_out_y[i]);

	//}

	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(cmdQueue);
	clReleaseMemObject(buffer_sigma);
	clReleaseMemObject(buffer_outx);
	clReleaseMemObject(buffer_outy);
	clReleaseContext(context);
	return 0;
}

int main()
{

	printf("Please input number of nodes:\n");

	CL_init();
	getchar();

	return 0;
}