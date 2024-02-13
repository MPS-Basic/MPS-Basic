#include "input.hpp"
#include "loader.hpp"
#include "mps.hpp"
#include <filesystem>

int main(int argc, char** argv) {
	if (argc < 2) {
		std::cout << "Usage" << std::endl << std::endl;
		std::cout << "  mps <path-to-settings>" << std::endl << std::endl;
		std::cout << "Specify a settings for simulation." << std::endl;
		return 0;
	}

	auto settingPath = std::filesystem::path(argv[1]);
	Input input      = Loader().load(settingPath);
	MPS mps          = MPS(input);
	mps.run();

	return 0;
}
