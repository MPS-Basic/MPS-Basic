#include "mps.hpp"
#include "input.hpp"
#include "loader.hpp"
#include <filesystem>

int main(int argc, char** argv) {
	auto settingPath = std::filesystem::path("./input/settings.yml");
	Input input = Loader().load(settingPath);
	MPS mps = MPS(input);
	mps.run();

	return 0;
}
