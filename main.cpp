

#include "tests.h"



int main(int argc, char* argv[])
{
	size_t len = 100;
	for (size_t i = 0; i < len; i++)
	{
		//if (!testM2OQueue())
		//	break;
		if (!testM2MQueue())
			break;
	}
	return 0;
}