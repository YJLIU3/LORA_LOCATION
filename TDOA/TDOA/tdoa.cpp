#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string>
#include <CL\cl.h>
using namespace std;
#pragma warning(disable:4996)


#define C 3*10^8

int nodes = 5;
int number_of_location0 = 5;
int number_of_location1 = number_of_location0;
int number_of_location2 = number_of_location0;

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

	if (false == GetFileData("combination.cl", code_file))
		return 0;

	char* buf_code = new char[code_file.size()];
	strcpy(buf_code, code_file.c_str());
	buf_code[code_file.size() - 1] = NULL;

	cl_device_id device;
	cl_platform_id platform_id = NULL;
	cl_context context;
	cl_command_queue cmdQueue;
	cl_mem buffer_input0, buffer_output, buffer_input1, buffer_input2;
	cl_program program;
	cl_kernel kernel = NULL;

	size_t globalWorkSize[3];
	globalWorkSize[0] = number_of_location0;
	globalWorkSize[1] = number_of_location1;
	globalWorkSize[2] = number_of_location2;

//	int all_data = number_of_location*number_of_location*number_of_location;
	int all_data = number_of_location0*number_of_location0*number_of_location0*3;
	cl_int err;

	int *buf_locat_element0 = new int[number_of_location0];
	int *buf_locat_element1 = new int[number_of_location1];
	int *buf_locat_element2 = new int[number_of_location2];
	int *buf_out_element = new int[all_data];

	size_t datasize = sizeof(int)*number_of_location0;
	size_t all_datasize = sizeof(int)*all_data;
	for (int i = 0; i < number_of_location0; i++)
	{
		buf_locat_element0[i] = i+1;
		buf_locat_element1[i] = i+1;
		buf_locat_element2[i] = i+1;
	}
	for (int i = 0; i < number_of_location0; i++)
	{
		printf("%d", buf_locat_element0[i]);
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

	buffer_input0 = clCreateBuffer(context, CL_MEM_READ_ONLY, datasize, NULL, NULL);
	buffer_input1 = clCreateBuffer(context, CL_MEM_READ_ONLY, datasize, NULL, NULL);
	buffer_input2 = clCreateBuffer(context, CL_MEM_READ_ONLY, datasize, NULL, NULL);

	buffer_output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, all_datasize, NULL, NULL);

	clEnqueueWriteBuffer(cmdQueue, buffer_input0, CL_FALSE, 0, datasize, buf_locat_element0, 0, NULL, NULL);
	clEnqueueWriteBuffer(cmdQueue, buffer_input1, CL_FALSE, 0, datasize, buf_locat_element1, 0, NULL, NULL);
	clEnqueueWriteBuffer(cmdQueue, buffer_input2, CL_FALSE, 0, datasize, buf_locat_element2, 0, NULL, NULL);

	program = clCreateProgramWithSource(context, 1, (const char**)&buf_code, NULL, NULL);

	clBuildProgram(program, 1, &device, NULL, NULL, NULL);


	kernel = clCreateKernel(program, "combination", NULL);

	clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer_input0);
	clSetKernelArg(kernel, 1, sizeof(cl_mem), &buffer_input1);
	clSetKernelArg(kernel, 2, sizeof(cl_mem), &buffer_input2);
	clSetKernelArg(kernel, 3, sizeof(cl_mem), &buffer_output);

	clEnqueueNDRangeKernel(cmdQueue, kernel, 3, NULL, globalWorkSize, NULL, 0, NULL, NULL);

	clEnqueueReadBuffer(cmdQueue, buffer_output, CL_TRUE, 0, all_datasize, buf_out_element, 0, NULL, NULL);


	for (int i = 0; i < all_data; i++)
	{
		if (buf_out_element[i] == 0)
			continue;

		printf("%d", buf_out_element[i]);
		
		if ((i + 1) % 3 == 0)
			printf("\n");
	}

	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(cmdQueue);
	clReleaseMemObject(buffer_input0);
	clReleaseMemObject(buffer_input1);
	clReleaseMemObject(buffer_input2);
	clReleaseMemObject(buffer_output);
	clReleaseContext(context);

}

int main()
{
	
	printf("Please input number of nodes:%d\n",nodes);
	CL_init();
	getchar();

	return 0;
}