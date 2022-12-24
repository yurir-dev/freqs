
#include <iostream>


int main(int argc, char* argv[])
{
	std::cout << "hello from ctest_template: ";
	for (size_t i = 0; i < argc; i++)
	{
		std::cout << argv[i] << ", ";
	}
	std::cout << std::endl;

	return 0;
}