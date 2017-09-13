__kernel void combination(__global int* IN0, __global int* IN1, __global int* IN2, __global int* OUT)
{

	
	int in0 = get_global_id(0);
	int in1 = get_global_id(1);
	int in2 = get_global_id(2);
	
	int location = (in0 * 25 + in1 * 5 + in2)*3;

	printf("in0 = %d\n", in0);
	printf("in1 = %d\n", in1);
	printf("in2 = %d\n", in2);

	if ((in0 < in1) && (in1< in2))
	{
		printf("if = true\n");


		OUT[location] = IN0[in0];
		OUT[location + 1] = IN1[in1];
		OUT[location + 2] = IN2[in2];
	}
	else
	{
		OUT[location] = 0;
		OUT[location + 1] = 0;
		OUT[location + 2] = 0;
	}

}