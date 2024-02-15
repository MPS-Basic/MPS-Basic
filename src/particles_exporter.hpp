#pragma once
#include "particle.hpp"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

namespace fs = std::filesystem;

/**
 * ParticlesExporter class
 *
 * This class is responsible for exporting the particles to a file. It can
 * export the particles to certain file formats. If you want to add a new
 * file format, you can add a new method to this class. It only requires
 * the particles (and the time) to export the particles to a file.
 *
 */
class ParticlesExporter {
public:
	std::vector<Particle> particles;

	void setParticles(const std::vector<Particle>& particles) {
		this->particles = particles;
	}

	void toProf(const fs::path& path, const double& time) {
		std::ofstream ofs(path);
		if (ofs.fail()) {
			std::cerr << "cannot write " << path << std::endl;
			std::exit(-1);
		}

		ofs << time << std::endl;
		ofs << particles.size() << std::endl;
		for (const auto& p : particles) {
			ofs << static_cast<int>(p.type) << " ";
			ofs << p.position.x() << " " << p.position.y() << " " << p.position.z() << " ";
			ofs << p.velocity.x() << " " << p.velocity.y() << " " << p.velocity.z();
			ofs << std::endl;
		}
	};

	void toVtu(const fs::path& path, const double& time) {
		std::ofstream ofs(path);
		if (ofs.fail()) {
			std::cerr << "cannot write " << path << std::endl;
			std::exit(-1);
		}

		// --------------
		// --- Header ---
		// --------------
		ofs << "<?xml version='1.0' encoding='UTF-8'?>" << std::endl;
		ofs << "<VTKFile xmlns='VTK' byte_order='LittleEndian' version='0.1' "
		       "type "
		       "= 'UnstructuredGrid' >"
		    << std::endl;
		ofs << "<UnstructuredGrid>" << std::endl;
		ofs << "<Piece NumberOfCells='" << particles.size() << "' NumberOfPoints='" << particles.size() << "'>"
		    << std::endl;

		/// ------------------
		/// ----- Points -----
		/// ------------------
		ofs << "<Points>" << std::endl;
		ofs << "<DataArray NumberOfComponents='3' type='Float64' "
		       "Name='position' format='ascii'>"
		    << std::endl;
		for (const auto& p : particles) {
			ofs << p.position.x() << " ";
			ofs << p.position.y() << " ";
			ofs << p.position.z() << std::endl;
		}
		ofs << "</DataArray>" << std::endl;
		ofs << "</Points>" << std::endl;

		// ---------------------
		// ----- PointData -----
		// ---------------------
		ofs << "<PointData>" << std::endl;

		dataArrayBegin(ofs, "1", "Int32", "Particle Type");
		for (auto& p : particles) {
			ofs << static_cast<int>(p.type) << std::endl;
		}
		dataArrayEnd(ofs);

		dataArrayBegin(ofs, "3", "Float64", "Velocity");
		for (const auto& p : particles) {
			ofs << p.velocity.x() << " ";
			ofs << p.velocity.y() << " ";
			ofs << 0.0 << std::endl;
		}
		dataArrayEnd(ofs);

		dataArrayBegin(ofs, "1", "Float64", "Pressure");
		for (const auto& p : particles)
			ofs << p.pressure << std::endl;
		dataArrayEnd(ofs);

		dataArrayBegin(ofs, "1", "Float64", "Number Density");
		for (const auto& p : particles)
			ofs << p.numberDensity << std::endl;
		dataArrayEnd(ofs);

		ofs << "</PointData>" << std::endl;

		// -----------------
		// ----- Cells -----
		// -----------------
		ofs << "<Cells>" << std::endl;
		ofs << "<DataArray type='Int32' Name='connectivity' format='ascii'>" << std::endl;
		for (int i = 0; i < particles.size(); i++) {
			ofs << i << std::endl;
		}
		ofs << "</DataArray>" << std::endl;
		ofs << "<DataArray type='Int32' Name='offsets' format='ascii'>" << std::endl;
		for (int i = 0; i < particles.size(); i++) {
			ofs << i + 1 << std::endl;
		}
		ofs << "</DataArray>" << std::endl;
		ofs << "<DataArray type='UInt8' Name='types' format='ascii'>" << std::endl;
		for (int i = 0; i < particles.size(); i++) {
			ofs << "1" << std::endl;
		}
		ofs << "</DataArray>" << std::endl;
		ofs << "</Cells>" << std::endl;

		// ------------------
		// ----- Footer -----
		// ------------------
		ofs << "</Piece>" << std::endl;
		ofs << "</UnstructuredGrid>" << std::endl;
		ofs << "</VTKFile>" << std::endl;
	}

private:
	void dataArrayBegin(std::ofstream& ofs,
	                    const std::string& numberOfComponents,
	                    const std::string& type,
	                    const std::string& name) {
		ofs << "<DataArray NumberOfComponents='" << numberOfComponents << "' type='" << type << "' Name='" << name
		    << "' format='ascii'>" << std::endl;
	}

	void dataArrayEnd(std::ofstream& ofs) {
		ofs << "</DataArray>" << std::endl;
	}
};
