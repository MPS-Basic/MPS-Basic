#include "saver.hpp"

namespace fs = std::filesystem;

Saver::Saver(const fs::path& dir) {
    this->dir = dir;
    fs::create_directories(dir / "prof");
    fs::create_directories(dir / "vtu");
    fs::create_directories(dir / "csv");
};

void Saver::save(const MPS& mps, const double time) {
    exporter.setParticles(mps.particles);

    std::stringstream profName;
    profName << "output_" << std::setfill('0') << std::setw(4) << fileNumber << ".prof";
    fs::path profPath = dir / "prof" / profName.str();
    exporter.toProf(profPath, time);

    std::stringstream vtuName;
    vtuName << "output_" << std::setfill('0') << std::setw(4) << fileNumber << ".vtu";
    fs::path vtuPath = dir / "vtu" / vtuName.str();
    exporter.toVtu(vtuPath, time, mps.refValuesForNumberDensity.n0);

    std::stringstream csvName;
    csvName << "output_" << std::setfill('0') << std::setw(4) << fileNumber << ".csv";
    fs::path csvPath = dir / "csv" / csvName.str();
    exporter.toCsv(csvPath, time);

    fileNumber++;
}
