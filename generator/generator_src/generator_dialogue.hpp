#include "../../src/particles.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <fstream>

class GeneratorDialogue {
public:
    /// @brief Show a dialogue to check the directory and file names before exporting the initial particles in
    /// generators.
    /// @details This method will create a directory if it does not exist.
    /// If the directory exists, it will show the files in the directory and ask for confirmation to overwrite them.
    /// @param parentPath path to the parent directory where the files will be exported
    /// @param particles Particles to be exported
    /// @param extensions vector of file extensions to be used for exporting the particles
    void generatorDialogue(
        const std::filesystem::path& parentPath, 
        const Particles& particles, 
        const std::vector<std::string>& extensions
    );
};