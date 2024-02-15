#include "input.hpp"
#include "loader.hpp"
#include "mps.hpp"
#include <filesystem>

/**
 * @brief entry point of the program
 *
 * @param argc number of arguments
 * @param argv array of arguments
 * @return return code
 */
int main(int argc, char** argv) {
	auto settingPath = std::filesystem::path("./input/settings.yml");
	Input input      = Loader().load(settingPath);
	MPS mps          = MPS(input);
	mps.run();

	return 0;
}
