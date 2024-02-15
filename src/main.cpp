#include "simulation.hpp"
#include <filesystem>

namespace fs = std::filesystem;

/**
 * @brief entry point of the program
 *
 * @param argc number of arguments
 * @param argv array of arguments
 * @return return code
 */
int main(int argc, char** argv) {
	if (argc < 2) {
		std::cout << "Usage" << std::endl << std::endl;
		std::cout << "  mps <path-to-settings>" << std::endl << std::endl;
		std::cout << "Specify a settings for simulation." << std::endl;
		return 0;
	}

	auto settingPath = std::filesystem::path(argv[1]);
	Simulation sim(settingPath);
	sim.run();

	return 0;
}
