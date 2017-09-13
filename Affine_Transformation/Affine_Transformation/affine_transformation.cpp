#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string>
#include <CL\cl.h>
using namespace std;
#pragma warning(disable:4996)

#define PI	3.141592653


long int C = 3 * 10 ^ 8;

//Input data to get Hyperbola
double t1, t2;
double X1, Y1, X2, Y2;

double delta_D1 = fabs(t1 - t2)*C;

//Hyperbola paramaters
double a = delta_D1 / 2;

double focus = sqrt((X1 - X2)*(X1 - X2) + (Y1 - Y2)*(Y1 - Y2));

double b = sqrt(focus*focus - a*a);

//midpoint to get affine translate
double midle_x = (X1 + X2) / 2;
double midle_y = (Y1 + Y2) / 2;

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

float theta = get_theta(X1, X2, Y1, Y2);

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
	globalWorkSize[0] = 2*PI*1000;

	cl_int err;

	int number_of_point = 2 * PI * 1000;

	float *sigma = new float[number_of_point];
	float *buf_out_x = new float[number_of_point];
	float *buf_out_y = new float[number_of_point];

	size_t in_datasize = sizeof(float)*number_of_point;
	size_t out_datasize = sizeof(float)*number_of_point;
	for (int i = 0; i < number_of_point; i++)
	{
		sigma[i] = i/1000;
	}
	for (int i = 0; i < number_of_point; i++)
	{
		printf("%d", sigma[i]);
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

	buffer_sigma = clCreateBuffer(context, CL_MEM_READ_ONLY, in_datasize, NULL, NULL);

	buffer_outx = clCreateBuffer(context, CL_MEM_WRITE_ONLY, out_datasize, NULL, NULL);
	buffer_outy = clCreateBuffer(context, CL_MEM_WRITE_ONLY, out_datasize, NULL, NULL);

	clEnqueueWriteBuffer(cmdQueue, buffer_sigma, CL_FALSE, 0, in_datasize, sigma, 0, NULL, NULL);

	program = clCreateProgramWithSource(context, 1, (const char**)&buf_code, NULL, NULL);

	clBuildProgram(program, 1, &device, "-D FP_64", NULL, NULL);


	kernel = clCreateKernel(program, "affine", NULL);

	clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer_sigma);

	clSetKernelArg(kernel, 3, sizeof(cl_mem), &buffer_outx);
	clSetKernelArg(kernel, 3, sizeof(cl_mem), &buffer_outy);

	clEnqueueNDRangeKernel(cmdQueue, kernel, 1, NULL, globalWorkSize, NULL, 0, NULL, NULL);

	clEnqueueReadBuffer(cmdQueue, buffer_outx, CL_TRUE, 0, out_datasize, buf_out_x, 0, NULL, NULL);
	clEnqueueReadBuffer(cmdQueue, buffer_outy, CL_TRUE, 0, out_datasize, buf_out_y, 0, NULL, NULL);

	for (int i = 0; i < number_of_point; i++)
	{
		printf("%f,%f--", buf_out_x[i], buf_out_y[i]);
	}

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