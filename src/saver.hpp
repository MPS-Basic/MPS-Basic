#pragma once

#include "common.hpp"
#include "mps.hpp"
#include "particles_exporter.hpp"
#include <filesystem>

/**
 * Saver class
 *
 * This class is responsible for saving the simulation results. It saves the
 * results to the file system. It uses the ParticlesExporter class to export the
 * particles to a file.
 *
 */
class Saver {
public:
	ParticlesExporter exporter;
	int fileNumber = 0;
	fs::path dir;

	Saver() = default;

	Saver(const fs::path& dir) {
		this->dir = dir;
		fs::create_directories(dir / "prof");
		fs::create_directories(dir / "vtu");
	};

	void save(const MPS& mps, const double time) {
		exporter.setParticles(mps.particles);

		std::stringstream profName;
		profName << "output_" << std::setfill('0') << std::setw(4) << fileNumber << ".prof";
		fs::path profPath = dir / "prof" / profName.str();
		exporter.toProf(profPath, time);

		std::stringstream vtuName;
		vtuName << "output_" << std::setfill('0') << std::setw(4) << fileNumber << ".vtu";
		fs::path vtuPath = dir / "vtu" / vtuName.str();
		exporter.toVtu(vtuPath, time);

		fileNumber++;
	};

private:
};
