#include "mps.hpp"
#include "input.hpp"
#include "loader.hpp"
#include <filesystem>

int main(int argc, char** argv) {
	auto exeDir = std::filesystem::path(argv[0]).parent_path();
	auto settingPath = exeDir.parent_path() / "input/settings.yml";
	Input input = Loader().load(settingPath);
	MPS mps = MPS(input);
	mps.run();

	return 0;
}
