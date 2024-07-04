#include "loader.hpp"

#include "particles_loader/csv.hpp"
#include "particles_loader/prof.hpp"

#include <cmath>
#include <iostream>
#include <yaml-cpp/yaml.h>

using std::cerr;
using std::cout;
using std::endl;

namespace fs = std::filesystem;

Input Loader::load(const fs::path& settingPath, const fs::path& outputDirectory) {
    Input input;

    input.settings = loadSettingYaml(settingPath);

    auto particlesPath          = input.settings.particlesPath;
    this->particlesLoader       = getParticlesLoader(particlesPath);
    auto [startTime, particles] = this->particlesLoader->load(particlesPath, input.settings.defaultDensity);
    input.startTime             = startTime;
    input.particles             = particles;

    copyInputFileToOutputDirectory(settingPath, outputDirectory);
    copyInputFileToOutputDirectory(particlesPath, outputDirectory);

    return input;
}

/**
 * @brief Get the Particles Loader object according to the input file extension
 * @param particlesPath Path to the particles file
 * @return std::unique_ptr<ParticlesLoader::Interface> Particles Loader object
 * @details Supported file formats: prof, csv
 */
std::unique_ptr<ParticlesLoader::Interface> Loader::getParticlesLoader(const fs::path& particlesPath) {
    auto extension = particlesPath.extension();
    if (extension == ".csv") {
        return std::make_unique<ParticlesLoader::Csv>();
    } else if (extension == ".prof") {
        return std::make_unique<ParticlesLoader::Prof>();
    } else {
        cerr << "unsupported file format: " << particlesPath.extension() << endl;
        std::exit(-1);
    }
}

/**
 * @brief Copy the input file to the output directory
 * @param inputFilePath Path to the input file
 * @param outputDirectory Path to the output directory
 * @warning If the file already exists in the output directory, the program will exit.
 */
void Loader::copyInputFileToOutputDirectory(const fs::path& inputFilePath, const fs::path& outputDirectory) {
    auto outputFilePath = outputDirectory / inputFilePath.filename();
    if (fs::exists(outputFilePath)) {
        cerr << "file " << outputFilePath << " already exists in the output directory" << endl;
        std::exit(-1);
    } else {
        fs::copy_file(inputFilePath, outputFilePath);
    }
}

Settings Loader::loadSettingYaml(const fs::path& settingPath) {
    YAML::Node yaml = YAML::LoadFile(settingPath.string());

    Settings s;

    // computational conditions
    s.dim              = yaml["dim"].as<int>();
    s.particleDistance = yaml["particleDistance"].as<double>();
    s.dt               = yaml["dt"].as<double>();
    s.endTime          = yaml["endTime"].as<double>();
    s.outputPeriod     = yaml["outputPeriod"].as<double>();
    s.cflCondition     = yaml["cflCondition"].as<double>();
    s.numPhysicalCores = yaml["numPhysicalCores"].as<int>();

    // physical properties
    s.defaultDensity     = yaml["defaultDensity"].as<double>();
    s.kinematicViscosity = yaml["kinematicViscosity"].as<double>();

    // gravity
    s.gravity[0] = yaml["gravity"][0].as<double>();
    s.gravity[1] = yaml["gravity"][1].as<double>();
    s.gravity[2] = yaml["gravity"][2].as<double>();

    // free surface detection
    s.surfaceDetection_numberDensity_threshold = yaml["surfaceDetection-numberDensity-threshold"].as<double>();
    s.surfaceDetection_particleDistribution    = yaml["surfaceDetection-particleDistribution"].as<bool>();
    s.surfaceDetection_particleDistribution_threshold =
        yaml["surfaceDetection-particleDistribution-threshold"].as<double>();

    // pressure calculation method
    s.pressureCalculationMethod = yaml["pressureCalculationMethod"].as<std::string>();
    // for Implicit
    s.compressibility                  = yaml["compressibility"].as<double>();
    s.relaxationCoefficientForPressure = yaml["relaxationCoefficientForPressure"].as<double>();
    // for Explicit
    s.soundSpeed = yaml["soundSpeed"].as<double>();

    // collision
    s.collisionDistance        = yaml["collisionDistanceRatio"].as<double>() * s.particleDistance;
    s.coefficientOfRestitution = yaml["coefficientOfRestitution"].as<double>();

    // effective radius
    s.re_forNumberDensity = yaml["radiusRatioForNumberDensity"].as<double>() * s.particleDistance;
    s.re_forGradient      = yaml["radiusRatioForGradient"].as<double>() * s.particleDistance;
    s.re_forLaplacian     = yaml["radiusRatioForLaplacian"].as<double>() * s.particleDistance;
    s.reMax               = std::max({s.re_forNumberDensity, s.re_forGradient, s.re_forLaplacian});

    // domain
    s.domain.xMin    = yaml["domainMin"][0].as<double>();
    s.domain.xMax    = yaml["domainMax"][0].as<double>();
    s.domain.yMin    = yaml["domainMin"][1].as<double>();
    s.domain.yMax    = yaml["domainMax"][1].as<double>();
    s.domain.zMin    = yaml["domainMin"][2].as<double>();
    s.domain.zMax    = yaml["domainMax"][2].as<double>();
    s.domain.xLength = s.domain.xMax - s.domain.xMin;
    s.domain.yLength = s.domain.yMax - s.domain.yMin;
    s.domain.zLength = s.domain.zMax - s.domain.zMin;

    // particlesPath
    auto yamlDir          = settingPath.parent_path();
    auto relativeProfPath = yaml["particlesPath"].as<std::string>();
    s.particlesPath       = fs::weakly_canonical(yamlDir / relativeProfPath);

    return s;
}
