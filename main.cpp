
#define  _CRT_SECURE_NO_WARNINGS 1

#include <iostream>
#include <array>
#include <fstream>
#include <vector>

std::array<size_t, 256> getFreqs(const std::string& filename);

int main(int argc, char* argv[])
{
	try
	{
		std::string filename = (argc == 1 ? argv[0] : argv[1]);
		std::array<size_t, 256> freqs{ getFreqs(filename) };

		{
			std::ofstream ofs("freqs.txt");
			ofs << "byte freqs of " << filename << std::endl;
			for (size_t i = 0; i < freqs.size(); i++)
				ofs << i << '\t' << static_cast<uint32_t>(freqs[i]) << std::endl;
		}

		std::cout << "check results in freqs.txt" << std::endl;
		return 0;
	}
	catch (std::exception& ex)
	{
		std::cout << "got exception: " << ex.what() << std::endl;
		return 1;
	}
}

std::array<size_t, 256> getFreqs(const std::string& filename)
{
	std::ifstream ifs(filename, std::ios::binary | std::ios::in);
	if (!ifs)
		throw std::runtime_error(filename + ": " + std::strerror(errno));

	std::array<size_t, 256> res{0};
	std::vector<std::byte> buffer(4096);

	size_t totalBytes{0};
	while (ifs.good())
	{
		ifs.read((char*)buffer.data(), buffer.size());

		for (unsigned i = 0; i < ifs.gcount(); i++)
		{
			const size_t ind{static_cast<size_t>(buffer[i])};
			res[ind]++;
		}
		totalBytes += ifs.gcount();
	}

	std::cout << "total bytes: " << totalBytes << std::endl;

	return res;
}