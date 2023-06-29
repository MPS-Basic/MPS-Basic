#include "src/particle.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

void check_fluid_range(std::vector<double>& x_range,
                       std::vector<double>& y_range,
                       double& l0,
                       double& eps);
bool isInside(Eigen::Vector2d& position,
              std::vector<double>& x_range,
              std::vector<double>& y_range,
              double& eps);
void writeProf(std::vector<Particle>& particles);
void dataArrayBegin(std::ofstream& ofs,
                    const std::string& numberOfComponents,
                    const std::string& type,
                    const std::string& name);
void dataArrayEnd(std::ofstream& ofs);

int main(int argc, char** argv) {
	std::vector<Particle> particles;

	double l0  = 0.025;
	double eps = 0.01 * l0;

	std::vector<double> fluid_x_range{0.0, 1.0};
	std::vector<double> fluid_y_range(0.0, 0.6);
	check_fluid_range(fluid_x_range, fluid_y_range, l0, eps);

	int ix_begin = (int)((fluid_x_range.at(0) + eps) / l0) - 4;
	int ix_end   = (int)((fluid_x_range.at(1) + eps) / l0) + 4;
	int iy_begin = (int)((fluid_y_range.at(0) + eps) / l0) - 4;
	int iy_end   = (int)((fluid_y_range.at(1) + eps) / l0) + 4;
	for (int ix = ix_begin; ix <= ix_end; ix++) {
		for (int iy = iy_begin; iy <= iy_end; iy++) {
			Eigen::Vector2d position((double)(ix)*l0, (double)(iy)*l0);
			ParticleType type = ParticleType::Ghost;

			// dummy wall region
			std::vector<double> x_range(2), y_range(2);
			x_range = {-4.0 * l0, 1.0 + 4.0 * l0};
			y_range = {-4.0 * l0, 0.6};
			if (isInside(position, x_range, y_range, eps))
				type = ParticleType::DummyWall;

			// wall region
			x_range = {-2.0 * l0, 1.0 + 2.0 * l0};
			y_range = {-2.0 * l0, 0.6};
			if (isInside(position, x_range, y_range, eps))
				type = ParticleType::Wall;

			// wall region
			x_range = {-4.0 * l0, 1.0 + 4.0 * l0};
			y_range = {0.6 - 2.0 * l0, 0.6};
			if (isInside(position, x_range, y_range, eps))
				type = ParticleType::Wall;

			// empty region
			x_range = {0.0, 1.0};
			y_range = {0.0, 0.6};
			if (isInside(position, x_range, y_range, eps))
				type = ParticleType::Ghost;

			// fluid region
			x_range = {0.0, 0.25};
			y_range = {0.0, 0.50};
			if (isInside(position, x_range, y_range, eps))
				type = ParticleType::Fluid;

			if (type != ParticleType::Ghost) {
				particles.emplace_back(position.x(), position.y(), 0.0, type);
			}
		}
	}
}

void check_fluid_range(std::vector<double>& x_range,
                       std::vector<double>& y_range,
                       double& l0,
                       double& eps) {
	std::sort(x_range.begin(), x_range.end());
	std::sort(y_range.begin(), y_range.end());

	double nx = (x_range.at(1) - x_range.at(0)) / l0;
	if (abs(nx - (int)(nx)) > eps) {
		std::cerr << "x_range of the fluid is not divisible by particle length."
		          << std::endl;
		exit(1);
	}

	double ny = (y_range.at(1) - y_range.at(0)) / l0;
	if (abs(ny - (int)(nx)) > eps) {
		std::cerr << "y_range of the fluid is not divisible by particle length."
		          << std::endl;
		exit(1);
	}
}

bool isInside(Eigen::Vector2d& position,
              std::vector<double>& x_range,
              std::vector<double>& y_range,
              double& eps) {
	std::sort(x_range.begin(), x_range.end());
	std::sort(y_range.begin(), y_range.end());

	if (((x_range.at(0) - eps < position.x()) &&
	     (position.x() < x_range.at(1) + eps)) &&
	    ((y_range.at(0) - eps < position.y()) &&
	     (position.y() < y_range.at(1) + eps)))
		return true;
	else
		return false;
}

void writeProf(std::vector<Particle>& particles) {
	std::stringstream ss;
	ss << "input.prof";

	std::ofstream ofs(ss.str());
	if (ofs.fail()) {
		std::cerr << "cannot write " << ss.str() << std::endl;
	}

	ofs << 0.0 << std::endl; // time
	ofs << particles.size() << std::endl;
	for (int i = 0; i < particles.size(); i++) {
		ofs << particles.at(i).position.x() << " ";
		ofs << particles.at(i).position.y() << " ";
		ofs << particles.at(i).position.z() << " ";
		ofs << particles.at(i).velocity.x() << " ";
		ofs << particles.at(i).velocity.y() << " ";
		ofs << particles.at(i).velocity.z() << " ";
		ofs << particles.at(i).pressure << " ";
		ofs << particles.at(i).numberDensity << std::endl;
	}
}

void writeVtu(std::vector<Particle>& particles) {
	std::stringstream ss;
	ss << "input.vtu";

	std::ofstream ofs(ss.str());
	if (ofs.fail()) {
		std::cerr << "cannot write " << ss.str() << std::endl;
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
	ofs << "<Piece NumberOfCells='" << particles.size() << "' NumberOfPoints='"
	    << particles.size() << "'>" << std::endl;

	/// ------------------
	/// ----- Points -----
	/// ------------------
	ofs << "<Points>" << std::endl;
	ofs << "<DataArray NumberOfComponents='3' type='Float64' "
	       "Name='position' format='ascii'>"
	    << std::endl;
	for (int i = 0; i < particles.size(); i++) {
		ofs << particles[i].position.x() << " " << particles[i].position.y()
		    << " " << particles[i].position.z() << std::endl;
	}
	ofs << "</DataArray>" << std::endl;
	ofs << "</Points>" << std::endl;

	// ---------------------
	// ----- PointData -----
	// ---------------------
	ofs << "<PointData>" << std::endl;

	dataArrayBegin(ofs, "1", "Int34", "particleType");
	for (int i = 0; i < particles.size(); i++) {
		ofs << static_cast<int>(particles[i].particleType) << std::endl;
	}
	dataArrayEnd(ofs);

	dataArrayBegin(ofs, "3", "Float64", "velocity");
	for (int i = 0; i < particles.size(); i++) {
		ofs << particles[i].velocity.x() << " " << particles[i].velocity.y()
		    << " " << particles[i].velocity.z() << std::endl;
	}
	dataArrayEnd(ofs);

	dataArrayBegin(ofs, "1", "Float64", "pressure");
	for (int i = 0; i < particles.size(); i++) {
		ofs << particles[i].pressure << std::endl;
	}
	dataArrayEnd(ofs);

	dataArrayBegin(ofs, "1", "Float64", "numberDensity");
	for (int i = 0; i < particles.size(); i++) {
		ofs << particles[i].numberDensity << std::endl;
	}
	dataArrayEnd(ofs);

	ofs << "</PointData>" << std::endl;

	// -----------------
	// ----- Cells -----
	// -----------------
	ofs << "<Cells>" << std::endl;
	ofs << "<DataArray type='Int32' Name='connectivity' format='ascii'>"
	    << std::endl;
	for (int i = 0; i < particles.size(); i++) {
		ofs << i << std::endl;
	}
	ofs << "</DataArray>" << std::endl;
	ofs << "<DataArray type='Int32' Name='offsets' format='ascii'>"
	    << std::endl;
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

void dataArrayBegin(std::ofstream& ofs,
                    const std::string& numberOfComponents,
                    const std::string& type,
                    const std::string& name) {
	ofs << "<DataArray NumberOfComponents='" << numberOfComponents << "' type='"
	    << type << "' Name='" << name << "' format='ascii'>" << std::endl;
}

void dataArrayEnd(std::ofstream& ofs) {
	ofs << "</DataArray>" << std::endl;
}
