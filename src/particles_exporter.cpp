#include "particles_exporter.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

using std::cerr;
using std::endl;
namespace fs = std::filesystem;

void ParticlesExporter::setParticles(const std::vector<Particle>& particles) {
	this->particles = particles;
}

void ParticlesExporter::toProf(const fs::path& path, const double& time) {
	std::ofstream ofs(path);
	if (ofs.fail()) {
		cerr << "cannot write " << path << endl;
		std::exit(-1);
	}

	ofs << time << endl;
	ofs << particles.size() << endl;
	for (const auto& p : particles) {
		ofs << static_cast<int>(p.type) << " ";
		ofs << p.position.x() << " " << p.position.y() << " " << p.position.z() << " ";
		ofs << p.velocity.x() << " " << p.velocity.y() << " " << p.velocity.z();
		ofs << endl;
	}
}

void ParticlesExporter::toVtu(const fs::path& path, const double& time) {
	std::ofstream ofs(path);
	if (ofs.fail()) {
		cerr << "cannot write " << path << endl;
		std::exit(-1);
	}

	// --------------
	// --- Header ---
	// --------------
	ofs << "<?xml version='1.0' encoding='UTF-8'?>" << endl;
	ofs << "<VTKFile xmlns='VTK' byte_order='LittleEndian' version='0.1' "
	       "type "
	       "= 'UnstructuredGrid' >"
	    << endl;
	ofs << "<UnstructuredGrid>" << endl;
	ofs << "<Piece NumberOfCells='" << particles.size() << "' NumberOfPoints='" << particles.size() << "'>" << endl;

	/// ------------------
	/// ----- Points -----
	/// ------------------
	ofs << "<Points>" << endl;
	ofs << "<DataArray NumberOfComponents='3' type='Float64' "
	       "Name='position' format='ascii'>"
	    << endl;
	for (const auto& p : particles) {
		ofs << p.position.x() << " ";
		ofs << p.position.y() << " ";
		ofs << p.position.z() << endl;
	}
	ofs << "</DataArray>" << endl;
	ofs << "</Points>" << endl;

	// ---------------------
	// ----- PointData -----
	// ---------------------
	ofs << "<PointData>" << endl;

	dataArrayBegin(ofs, "1", "Int32", "Particle Type");
	for (auto& p : particles) {
		ofs << static_cast<int>(p.type) << endl;
	}
	dataArrayEnd(ofs);

	dataArrayBegin(ofs, "3", "Float64", "Velocity");
	for (const auto& p : particles) {
		ofs << p.velocity.x() << " ";
		ofs << p.velocity.y() << " ";
		ofs << 0.0 << endl;
	}
	dataArrayEnd(ofs);

	dataArrayBegin(ofs, "1", "Float64", "Pressure");
	for (const auto& p : particles)
		ofs << p.pressure << endl;
	dataArrayEnd(ofs);

	dataArrayBegin(ofs, "1", "Float64", "Number Density");
	for (const auto& p : particles)
		ofs << p.numberDensity << endl;
	dataArrayEnd(ofs);

	ofs << "</PointData>" << endl;

	// -----------------
	// ----- Cells -----
	// -----------------
	ofs << "<Cells>" << endl;
	ofs << "<DataArray type='Int32' Name='connectivity' format='ascii'>" << endl;
	for (int i = 0; i < particles.size(); i++) {
		ofs << i << endl;
	}
	ofs << "</DataArray>" << endl;
	ofs << "<DataArray type='Int32' Name='offsets' format='ascii'>" << endl;
	for (int i = 0; i < particles.size(); i++) {
		ofs << i + 1 << endl;
	}
	ofs << "</DataArray>" << endl;
	ofs << "<DataArray type='UInt8' Name='types' format='ascii'>" << endl;
	for (int i = 0; i < particles.size(); i++) {
		ofs << "1" << endl;
	}
	ofs << "</DataArray>" << endl;
	ofs << "</Cells>" << endl;

	// ------------------
	// ----- Footer -----
	// ------------------
	ofs << "</Piece>" << endl;
	ofs << "</UnstructuredGrid>" << endl;
	ofs << "</VTKFile>" << endl;
}

void ParticlesExporter::dataArrayBegin(std::ofstream& ofs,
                                       const std::string& numberOfComponents,
                                       const std::string& type,
                                       const std::string& name) {
	ofs << "<DataArray NumberOfComponents='" << numberOfComponents << "' type='" << type << "' Name='" << name
	    << "' format='ascii'>" << endl;
}

void ParticlesExporter::dataArrayEnd(std::ofstream& ofs) {
	ofs << "</DataArray>" << endl;
}
