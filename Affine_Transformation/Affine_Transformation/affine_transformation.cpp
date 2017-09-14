#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string>
#include <CL\cl.h>
using namespace std;
#pragma warning(disable:4996)

#define PI	3.141592653




float get_theta(float x1, float x2, float y1, float y2)
{
	float theta;
	if (x1 - x2 == 0)
		return PI / 2.0;
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

double delta_t(double t1, double t2)
{
	if (t1 > t2)
		return t1 - t2;
	else
		return t2 - t1;
}

int CL_init()
{
	double C = 300000000.0;

	//Input data to get Hyperbola
	double  t1 = 0.0, t2 = 0.0, t3 = 0.0;
	double	X1 = 0.0, Y1 = 0.0,
			X2 = 0.0, Y2 = 0.0,
			X3 = 0.0, Y3 = 0.0;
	
	float start_sigma = 0;
	float stop_sigma = 2.0*PI;
	int step_division = 1000;

	double delta_d12 = delta_t(t1, t2)*C;
	double delta_d13 = delta_t(t1, t3)*C;
	double delta_d23 = delta_t(t2, t3)*C;

	//Hyperbola paramaters
	double a12 = delta_d12 / 2;
	double a13 = delta_d13 / 2;
	double a23 = delta_d23 / 2;

	double focus_12 = sqrt((X1 - X2)*(X1 - X2) + (Y1 - Y2)*(Y1 - Y2)) / 2.0;
	double focus_13 = sqrt((X1 - X3)*(X1 - X3) + (Y1 - Y3)*(Y1 - Y3)) / 2.0;
	double focus_23 = sqrt((X3 - X2)*(X1 - X2) + (Y3 - Y2)*(Y3 - Y2)) / 2.0;

	double b12 = sqrt(focus_12*focus_12 - a12*a12);
	double b13 = sqrt(focus_13*focus_13 - a13*a13);
	double b23 = sqrt(focus_23*focus_23 - a23*a23);

	//midpoint to get affine translate
	double midle_x12 = (X1 + X2) / 2;
	double midle_y12 = (Y1 + Y2) / 2;
	double midle_x13 = (X1 + X3) / 2;
	double midle_y13 = (Y1 + Y3) / 2;
	double midle_x23 = (X3 + X2) / 2;
	double midle_y23 = (Y3 + Y2) / 2;


	float theta12 = get_theta(X1, X2, Y1, Y2);
	float theta13 = get_theta(X1, X3, Y1, Y3);
	float theta23 = get_theta(X3, X2, Y3, Y2);

	

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
	cl_mem cl_buf_sigma1, cl_buf_sigma2, cl_buf_sigma3, cl_buf_outx, cl_buf_outy;
	cl_program program;
	cl_kernel kernel = NULL;

	size_t globalWorkSize[3];
	globalWorkSize[0] = (int)(stop_sigma - start_sigma) * step_division;
	globalWorkSize[1] = (int)(stop_sigma - start_sigma) * step_division;
	globalWorkSize[2] = (int)(stop_sigma - start_sigma) * step_division;

	cl_int err;

	int number_of_point = (int)(stop_sigma - start_sigma) * step_division;
	int number_of_outpoint = number_of_point*number_of_point*number_of_point;

	float *sigma1 = new float[number_of_point];
	float *sigma2 = new float[number_of_point];
	float *sigma3 = new float[number_of_point];

	float *buf_out_x = new float[number_of_outpoint];
	float *buf_out_y = new float[number_of_outpoint];

	size_t datasize = sizeof(float)*number_of_point;
	size_t out_datasize = sizeof(float)*number_of_outpoint;

	for (int i = 0; i < number_of_point; i++)
	{
		sigma1[i] = i / step_division + start_sigma;
		sigma2[i] = i / step_division + start_sigma;
		sigma3[i] = i / step_division + start_sigma;
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

	cl_buf_sigma1 = clCreateBuffer(context, CL_MEM_READ_ONLY, datasize, NULL, NULL);
	cl_buf_sigma2 = clCreateBuffer(context, CL_MEM_READ_ONLY, datasize, NULL, NULL);
	cl_buf_sigma3 = clCreateBuffer(context, CL_MEM_READ_ONLY, datasize, NULL, NULL);

	cl_buf_outx = clCreateBuffer(context, CL_MEM_WRITE_ONLY, out_datasize, NULL, NULL);
	cl_buf_outy = clCreateBuffer(context, CL_MEM_WRITE_ONLY, out_datasize, NULL, NULL);

	clEnqueueWriteBuffer(cmdQueue, cl_buf_sigma1, CL_TRUE, 0, datasize, sigma1, 0, NULL, NULL);
	clEnqueueWriteBuffer(cmdQueue, cl_buf_sigma2, CL_TRUE, 0, datasize, sigma2, 0, NULL, NULL);
	clEnqueueWriteBuffer(cmdQueue, cl_buf_sigma3, CL_TRUE, 0, datasize, sigma3, 0, NULL, NULL);

	program = clCreateProgramWithSource(context, 1, (const char**)&buf_code, NULL, NULL);


	cl_device_fp_config DeviceDouble;
	cl_int fff = clGetDeviceInfo(device, CL_DEVICE_DOUBLE_FP_CONFIG, sizeof(cl_device_fp_config),&DeviceDouble,NULL);

	clBuildProgram(program, 1, &device, "-D FP_64", NULL, NULL);


	kernel = clCreateKernel(program, "affine", NULL);



	clSetKernelArg(kernel, 0, sizeof(cl_double), &a12);
	clSetKernelArg(kernel, 1, sizeof(cl_double), &a13);
	clSetKernelArg(kernel, 2, sizeof(cl_double), &a23);
	clSetKernelArg(kernel, 3, sizeof(cl_double), &b12);
	clSetKernelArg(kernel, 4, sizeof(cl_double), &b13);
	clSetKernelArg(kernel, 5, sizeof(cl_double), &b23);
	clSetKernelArg(kernel, 6, sizeof(cl_double), &midle_x12);
	clSetKernelArg(kernel, 7, sizeof(cl_double), &midle_y12);
	clSetKernelArg(kernel, 8, sizeof(cl_double), &midle_x13);
	clSetKernelArg(kernel, 9, sizeof(cl_double), &midle_y13);
	clSetKernelArg(kernel, 10, sizeof(cl_double), &midle_x23);
	clSetKernelArg(kernel, 11, sizeof(cl_double), &midle_y23);
	clSetKernelArg(kernel, 12, sizeof(cl_float), &theta12);
	clSetKernelArg(kernel, 13, sizeof(cl_float), &theta13);
	clSetKernelArg(kernel, 14, sizeof(cl_float), &theta23);
	clSetKernelArg(kernel, 15, sizeof(cl_mem), &cl_buf_sigma1);
	clSetKernelArg(kernel, 16, sizeof(cl_mem), &cl_buf_sigma2);
	clSetKernelArg(kernel, 17, sizeof(cl_mem), &cl_buf_sigma3);

	clSetKernelArg(kernel, 18, sizeof(cl_mem), &cl_buf_outx);
	clSetKernelArg(kernel, 19, sizeof(cl_mem), &cl_buf_outy);



	clEnqueueNDRangeKernel(cmdQueue, kernel, 1, NULL, globalWorkSize, NULL, 0, NULL, NULL);

	clEnqueueReadBuffer(cmdQueue, cl_buf_outx, CL_TRUE, 0, datasize, buf_out_x, 0, NULL, NULL);
	clEnqueueReadBuffer(cmdQueue, cl_buf_outy, CL_TRUE, 0, datasize, buf_out_y, 0, NULL, NULL);

	FILE *fp;
	fp = fopen("hyperbola.xls", "w");
	for (int i = 0; i < number_of_point; i++)
	{
			printf("i = %d", i);
			printf("%f\t%f\n", buf_out_x[i], buf_out_y[i]);
			fprintf(fp, "%f\t%f\n", buf_out_x[i], buf_out_y[i]);
	}
	fclose(fp);

	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(cmdQueue);
	clReleaseMemObject(cl_buf_sigma1);
	clReleaseMemObject(cl_buf_sigma2);
	clReleaseMemObject(cl_buf_sigma3);
	clReleaseMemObject(cl_buf_outx);
	clReleaseMemObject(cl_buf_outy);
	clReleaseContext(context);
	return 0;
}

int main()
{


	CL_init();
	getchar();

	return 0;
}