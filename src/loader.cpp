#include "loader.hpp"

#include "particles_loader/csv.hpp"
#include "particles_loader/prof.hpp"
#include "particles_loader/vtu.hpp"

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
    } else if (extension == ".vtu") {
        return std::make_unique<ParticlesLoader::Vtu>();
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
    YAML::Node comp    = yaml["computationalConditions"];
    s.dim              = comp["dim"].as<int>();
    s.particleDistance = comp["particleDistance"].as<double>();
    s.dt               = comp["dt"].as<double>();
    s.endTime          = comp["endTime"].as<double>();
    s.outputPeriod     = comp["outputPeriod"].as<double>();
    s.cflCondition     = comp["cflCondition"].as<double>();

    // physical properties
    YAML::Node phys      = yaml["physicalProperties"];
    s.defaultDensity     = phys["defaultDensity"].as<double>();
    s.kinematicViscosity = phys["kinematicViscosity"].as<double>();

    // gravity
    YAML::Node grav = yaml["gravity"];
    s.gravityMethod = grav["method"] ? grav["method"].as<std::string>() : "xyz";
    if (s.gravityMethod == "xyz") {
        s.gravity[0] = grav["xyz"][0].as<double>();
        s.gravity[1] = grav["xyz"][1].as<double>();
        s.gravity[2] = grav["xyz"][2].as<double>();
    } else if (s.gravityMethod == "normAndAngle") {
        s.gNorm  = grav["normAndAngle"]["norm"] ? grav["normAndAngle"]["norm"].as<double>() : 0.0;
        s.gAngle = grav["normAndAngle"]["angle"] ? grav["normAndAngle"]["angle"].as<double>() : 0.0;
    } else {
        cerr << "unsupported gravity method: " << s.gravityMethod << endl;
        std::exit(-1);
    }

    // free surface detection
    YAML::Node surf                                   = yaml["surfaceDetection"];
    s.surfaceDetection_numberDensity_threshold        = surf["numberDensity"]["threshold"].as<double>();
    s.surfaceDetection_particleDistribution           = surf["particleDistribution"]["enabled"].as<bool>();
    s.surfaceDetection_particleDistribution_threshold = surf["particleDistribution"]["threshold"].as<double>();

    // pressure calculation method
    YAML::Node press            = yaml["pressureCalculation"];
    s.pressureCalculationMethod = press["method"].as<std::string>();
    if (press["method"].as<std::string>() == "Implicit") {
        // for Implicit
        s.compressibility                  = press["implicit"]["compressibility"].as<double>();
        s.relaxationCoefficientForPressure = press["implicit"]["relaxationCoefficient"].as<double>();
    } else if (press["method"].as<std::string>() == "Explicit") {
        // for Explicit
        s.soundSpeed = press["explicit"]["soundSpeed"].as<double>();
    } else {
        cerr << "unsupported pressure calculation method: " << press["method"].as<std::string>() << endl;
        std::exit(-1);
    }

    // collision
    YAML::Node coll            = yaml["collision"];
    s.collisionDistance        = coll["distanceRatio"].as<double>() * s.particleDistance;
    s.coefficientOfRestitution = coll["coefficientOfRestitution"].as<double>();

    // effective radius
    YAML::Node re         = yaml["effectiveRadius"];
    s.re_forNumberDensity = re["ratioForNumberDensity"].as<double>() * s.particleDistance;
    s.re_forGradient      = re["ratioForGradient"].as<double>() * s.particleDistance;
    s.re_forLaplacian     = re["ratioForLaplacian"].as<double>() * s.particleDistance;
    s.reMax               = std::max({s.re_forNumberDensity, s.re_forGradient, s.re_forLaplacian});

    // domain
    YAML::Node dom   = yaml["domain"];
    s.domain.xMin    = dom["min"][0].as<double>();
    s.domain.xMax    = dom["max"][0].as<double>();
    s.domain.yMin    = dom["min"][1].as<double>();
    s.domain.yMax    = dom["max"][1].as<double>();
    s.domain.zMin    = dom["min"][2].as<double>();
    s.domain.zMax    = dom["max"][2].as<double>();
    s.domain.xLength = s.domain.xMax - s.domain.xMin;
    s.domain.yLength = s.domain.yMax - s.domain.yMin;
    s.domain.zLength = s.domain.zMax - s.domain.zMin;

    // i/o
    YAML::Node io         = yaml["io"];
    auto yamlDir          = settingPath.parent_path();
    auto relativeProfPath = io["particlesPath"].as<std::string>();
    s.particlesPath       = fs::weakly_canonical(yamlDir / relativeProfPath);

    // outputVtkFormatInBinary
    s.outputVtkInBinary = io["outputVtkInBinary"] ? io["outputVtkInBinary"].as<bool>() : false;
    return s;
}
