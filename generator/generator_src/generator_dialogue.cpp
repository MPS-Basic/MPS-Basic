#include "generator_dialogue.hpp"
#include "../../src/particles_exporter.hpp"

namespace fs = std::filesystem;

void GeneratorDialogue::generatorDialogue(
    const fs::path& parentPath, 
    const Particles& particles, 
    const std::vector<std::string>& extensions
) {
    if (extensions.empty()) {
        std::cerr << "No file extensions provided for exporting particles." << std::endl;
        std::exit(0);
    } else {
        for (const auto& ext : extensions) {
            if (ext != ".prof" && ext != ".vtu" && ext != ".csv") {
                std::cerr << "Invalid file extension: " << ext << std::endl;
                std::exit(0);
            }
        }
    }

    ParticlesExporter exporter;
    exporter.setParticles(particles);
    if (!fs::exists(parentPath)) {
        fs::create_directories(parentPath);
        std::cout << "New directory has been made: " << parentPath << std::endl;
    } else {
        bool dialogueNeeded = false;
        for (const auto& ext : extensions) {
            fs::path filePath = parentPath / ("input" + ext);
            if (fs::exists(filePath)) {
                std::cout << "File already exists: " << filePath << std::endl;
                dialogueNeeded = true;
            }
        }
        if (dialogueNeeded) {
            std::string response;
            while (true) {
                std::cout << "Are you sure you want to overwrite the input files? (y/n): ";
                std::cin >> response;
                if (response == "n" || response == "N") {
                    std::cout << "The request has been cancelled." << std::endl;
                    std::exit(0);
                } else if (response == "y" || response == "Y") {
                    break;
                } else {
                    std::cout << "Invalid input. Please enter 'y' or 'n'." << std::endl;
                }
            }
        }
    }

    for (const auto& ext : extensions) {
        fs::path filePath = parentPath / ("input" + ext);
        if (ext == ".prof") {
            exporter.toProf(filePath, 0.0);
            std::cout << "Particles exported to " << filePath << std::endl;
        } else if (ext == ".vtu") {
            exporter.toVtu(filePath, 0.0);
            std::cout << "Particles exported to " << filePath << std::endl;
        } else if (ext == ".csv") {
            exporter.toCsv(filePath, 0.0);
            std::cout << "Particles exported to " << filePath << std::endl;
        }
    }

}
