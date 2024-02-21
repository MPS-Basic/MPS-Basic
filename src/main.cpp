#include "common.hpp"
#include "simulation.hpp"
#include <filesystem>

/**
 * @brief entry point of the program
 *
 * @param argc number of arguments
 * @param argv array of arguments
 * @return return code
 */
int main(int argc, char** argv) {
	if (argc < 2) {
		cout << "Usage" << endl << endl;
		cout << "  mps <path-to-settings>" << endl << endl;
		cout << "Specify a settings for simulation." << endl;
		return 0;
	}

	auto settingPath = fs::path(argv[1]);
	Simulation sim(settingPath);
	sim.run();

	return 0;
}
