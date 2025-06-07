#include "particles_exporter.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

using std::cerr;
using std::endl;
namespace fs = std::filesystem;

bool ParticlesExporter::isBigEndian() const {
    uint32_t i         = 1; // 0x00000001
    uint8_t first_byte = *reinterpret_cast<uint8_t*>(&i);
    return first_byte == 0;
}

std::string ParticlesExporter::dataArrayBegin(
    const std::string& type, const std::string& name, int numberOfComponents, const std::string& format, size_t offset
) const {
    // --- assertion ---
    // allowed types and format https://docs.vtk.org/en/latest/design_documents/VTKFileFormats.html
    // -----------------
    assert(
        type == "Int8" || type == "UInt8" || type == "Int16" || type == "UInt16" || type == "UInt32" ||
        type == "Int32" || type == "UInt64" || type == "Int64" || type == "Float32" || type == "Float64"
    );                                                                       // allowed data types
    assert(format == "ascii" || format == "binary" || format == "appended"); // allowed formats
    assert(
        format == "appended" && offset != SIZE_MAX || format != "appended" && offset == SIZE_MAX
    ); // offset is required for appended format

    std::string ret;
    ret += "<DataArray type=\"" + type + "\" Name=\"" + name + "\" NumberOfComponents=\"" +
           std::to_string(numberOfComponents) + "\" format=\"" + format + "\"";
    if (format == "appended") {
        ret += " offset=\"" + std::to_string(offset) + "\"";
    }
    ret += ">";
    return ret;
}

std::string ParticlesExporter::dataArrayEnd() const {
    return "</DataArray>";
}

void ParticlesExporter::setParticles(const Particles& particles) {
    this->particles = particles;
}

void ParticlesExporter::generatorDialogue(const fs::path& parentPath) {
    if (!fs::exists(parentPath)) {
        fs::create_directories(parentPath);
        std::cout << "New directory has been made: " << parentPath << std::endl;
    } else {
        for (const auto& entry : fs::directory_iterator(parentPath)) {
            bool inputFileFound = false;
            if (entry.path().filename().string().find("input") == 0) {
                if (!inputFileFound) {
                    // Show the message only once when "input.xxx" file was found.
                    std::cout << "The following files might be overwritten in the directory: " << parentPath << std::endl;
                    inputFileFound = true;
                } 
                std::cout << "|   " << entry.path().filename() << std::endl;
            }
        }
        std::string response;
        while (true) {
            std::cout << "Are you sure you want to export the input particles? (y/n): ";
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
void ParticlesExporter::toVtuAscii(const fs::path& path, const double& time, const double& n0ForNumberDensity) {
    std::ofstream ofs(path);
    if (ofs.fail()) {
        cerr << "cannot write " << path << endl;
        std::exit(-1);
    }

    // header
    ofs << "<?xml version='1.0' encoding='UTF-8'?>" << endl;
    ofs << "<VTKFile type=\"UnstructuredGrid\" version=\"1.0\" header_type=\"UInt64\" byte_order=\"";
    if (isBigEndian()) {
        ofs << "BigEndian";
    } else {
        ofs << "LittleEndian";
    }
    ofs << "\">" << endl;
    ofs << "<UnstructuredGrid>" << endl
        << "<Piece NumberOfPoints=\"" << particles.size() << "\" NumberOfCells=\"" << particles.size() << "\">" << endl;

    /// ------------------
    /// ----- Points -----
    /// ------------------
    ofs << "<Points>" << endl;
    ofs << dataArrayBegin("Float64", "Position", 3, "ascii") << endl;
    for (const auto& p : particles) {
        ofs << p.position.x() << " " << p.position.y() << " " << p.position.z() << endl;
    }
    ofs << dataArrayEnd() << endl;
    ofs << "</Points>" << endl;

    // ---------------------
    // ----- PointData -----
    // ---------------------
    ofs << "<PointData>" << endl;
    // Particle Type
    ofs << dataArrayBegin("Int32", "Particle Type", 1, "ascii") << endl;
    for (const auto& p : particles) {
        ofs << static_cast<int32_t>(p.type) << endl;
    }
    ofs << dataArrayEnd() << endl;
    // Velocity
    ofs << dataArrayBegin("Float64", "Velocity", 3, "ascii") << endl;
    for (const auto& p : particles) {
        ofs << p.velocity.x() << " " << p.velocity.y() << " " << p.velocity.z() << endl;
    }
    ofs << dataArrayEnd() << endl;
    // Pressure
    ofs << dataArrayBegin("Float64", "Pressure", 1, "ascii") << endl;
    for (const auto& p : particles) {
        ofs << p.pressure << endl;
    }
    ofs << dataArrayEnd() << endl;
    // Number Density
    ofs << dataArrayBegin("Float64", "Number Density", 1, "ascii") << endl;
    for (const auto& p : particles) {
        ofs << p.numberDensity << endl;
    }
    ofs << dataArrayEnd() << endl;
    // Number Density Ratio
    ofs << dataArrayBegin("Float64", "Number Density Ratio", 1, "ascii") << endl;
    for (const auto& p : particles) {
        ofs << p.numberDensity / n0ForNumberDensity << endl;
    }
    ofs << dataArrayEnd() << endl;
    // Boundary Condition
    ofs << dataArrayBegin("Int32", "Boundary Condition", 1, "ascii") << endl;
    for (const auto& p : particles) {
        ofs << static_cast<int32_t>(p.boundaryCondition) << endl;
    }
    ofs << dataArrayEnd() << endl;
    // Fluid Type
    ofs << dataArrayBegin("Int32", "Fluid Type", 1, "ascii") << endl;
    for (const auto& p : particles) {
        ofs << p.fluidType << endl;
    }
    ofs << dataArrayEnd() << endl;
    ofs << "</PointData>" << endl;

    /// -----------------
    /// ----- Cells -----
    /// -----------------
    ofs << "<Cells>" << endl;
    // connectivity
    ofs << dataArrayBegin("Int64", "connectivity", 1, "ascii") << endl;
    for (size_t i = 0; i < particles.size(); i++) {
        ofs << i << endl;
    }
    ofs << dataArrayEnd() << endl;
    // offsets
    ofs << dataArrayBegin("Int64", "offsets", 1, "ascii") << endl;
    for (size_t i = 0; i < particles.size(); i++) {
        ofs << i + 1 << endl;
    }
    ofs << dataArrayEnd() << endl;
    // types
    ofs << dataArrayBegin("UInt8", "types", 1, "ascii") << endl;
    for (const auto& p : particles) {
        ofs << 1 << endl; // 1: particle
    }
    ofs << dataArrayEnd() << endl;
    ofs << "</Cells>" << endl;
    ofs << "</Piece>" << endl;
    // ---------------------
    // ---- Field data  ----
    // ---------------------
    ofs << "<FieldData>" << endl;
    ofs << "<DataArray type=\"Float64\" Name=\"Time\" NumberOfTuples=\"1\" format=\"ascii\">" << endl;
    ofs << time << endl;
    ofs << "</DataArray>" << endl;
    ofs << "</FieldData>" << endl;
    ofs << "</UnstructuredGrid>" << endl;
    ofs << "</VTKFile>" << endl;
}

void ParticlesExporter::toVtuBinary(const fs::path& path, const double& time, const double& n0ForNumberDensity) {
    std::ofstream ofs(path);
    if (ofs.fail()) {
        cerr << "cannot write " << path << endl;
        std::exit(-1);
    }

    std::stringstream binaryData; // binary data to be written

    // header
    ofs << "<?xml version='1.0' encoding='UTF-8'?>" << endl;
    ofs << "<VTKFile type=\"UnstructuredGrid\" version=\"1.0\" header_type=\"UInt64\" byte_order=\"";
    if (isBigEndian()) {
        ofs << "BigEndian";
    } else {
        ofs << "LittleEndian";
    }
    ofs << "\">" << endl;
    ofs << "<UnstructuredGrid>" << endl
        << "<Piece NumberOfPoints=\"" << particles.size() << "\" NumberOfCells=\"" << particles.size() << "\">" << endl;
    /// ------------------
    /// ----- Points -----
    /// ------------------
    ofs << "<Points>" << endl;
    ofs << dataArrayBegin("Float64", "Position", 3, "appended", binaryData.tellp()) << endl;
    ofs << dataArrayEnd() << endl;
    uint64_t length_points = particles.size() * 3 * sizeof(double);
    binaryData.write(reinterpret_cast<char*>(&length_points), sizeof(uint64_t));
    for (const auto& p : particles) {
        double x = p.position.x();
        double y = p.position.y();
        double z = p.position.z();
        binaryData.write(reinterpret_cast<char*>(&x), sizeof(double));
        binaryData.write(reinterpret_cast<char*>(&y), sizeof(double));
        binaryData.write(reinterpret_cast<char*>(&z), sizeof(double));
    }
    ofs << "</Points>" << endl;
    // ---------------------
    // ----- PointData -----
    // ---------------------
    ofs << "<PointData>" << endl;
    // Particle Type
    ofs << dataArrayBegin("Int32", "Particle Type", 1, "appended", binaryData.tellp()) << endl;
    ofs << dataArrayEnd() << endl;
    uint64_t length_particle_type = particles.size() * sizeof(int32_t);
    binaryData.write(reinterpret_cast<char*>(&length_particle_type), sizeof(uint64_t));
    for (const auto& p : particles) {
        int32_t type = static_cast<int32_t>(p.type);
        binaryData.write(reinterpret_cast<char*>(&type), sizeof(int32_t));
    }
    // Velocity
    ofs << dataArrayBegin("Float64", "Velocity", 3, "appended", binaryData.tellp()) << endl;
    ofs << dataArrayEnd() << endl;
    uint64_t length_velocity = particles.size() * 3 * sizeof(double);
    binaryData.write(reinterpret_cast<char*>(&length_velocity), sizeof(uint64_t));
    for (const auto& p : particles) {
        double vx = p.velocity.x();
        double vy = p.velocity.y();
        double vz = p.velocity.z();
        binaryData.write(reinterpret_cast<char*>(&vx), sizeof(double));
        binaryData.write(reinterpret_cast<char*>(&vy), sizeof(double));
        binaryData.write(reinterpret_cast<char*>(&vz), sizeof(double));
    }
    // Pressure
    ofs << dataArrayBegin("Float64", "Pressure", 1, "appended", binaryData.tellp()) << endl;
    ofs << dataArrayEnd() << endl;
    uint64_t length_pressure = particles.size() * sizeof(double);
    binaryData.write(reinterpret_cast<char*>(&length_pressure), sizeof(uint64_t));
    for (const auto& p : particles) {
        double pressure = p.pressure;
        binaryData.write(reinterpret_cast<char*>(&pressure), sizeof(double));
    }
    // Number Density
    ofs << dataArrayBegin("Float64", "Number Density", 1, "appended", binaryData.tellp()) << endl;
    ofs << dataArrayEnd() << endl;
    uint64_t length_number_density = particles.size() * sizeof(double);
    binaryData.write(reinterpret_cast<char*>(&length_number_density), sizeof(uint64_t));
    for (const auto& p : particles) {
        double number_density = p.numberDensity;
        binaryData.write(reinterpret_cast<char*>(&number_density), sizeof(double));
    }
    // Number Density Ratio
    ofs << dataArrayBegin("Float64", "Number Density Ratio", 1, "appended", binaryData.tellp()) << endl;
    ofs << dataArrayEnd() << endl;
    uint64_t length_number_density_ratio = particles.size() * sizeof(double);
    binaryData.write(reinterpret_cast<char*>(&length_number_density_ratio), sizeof(uint64_t));
    for (const auto& p : particles) {
        double number_density_ratio = p.numberDensity / n0ForNumberDensity;
        binaryData.write(reinterpret_cast<char*>(&number_density_ratio), sizeof(double));
    }
    // Boundary Condition
    ofs << dataArrayBegin("Int32", "Boundary Condition", 1, "appended", binaryData.tellp()) << endl;
    ofs << dataArrayEnd() << endl;
    uint64_t length_boundary_condition = particles.size() * sizeof(int32_t);
    binaryData.write(reinterpret_cast<char*>(&length_boundary_condition), sizeof(uint64_t));
    for (const auto& p : particles) {
        int32_t boundary_condition = static_cast<int32_t>(p.boundaryCondition);
        binaryData.write(reinterpret_cast<char*>(&boundary_condition), sizeof(int32_t));
    }
    // Fluid Type
    ofs << dataArrayBegin("Int32", "Fluid Type", 1, "appended", binaryData.tellp()) << endl;
    ofs << dataArrayEnd() << endl;
    uint64_t length_fluid_type = particles.size() * sizeof(int32_t);
    binaryData.write(reinterpret_cast<char*>(&length_fluid_type), sizeof(uint64_t));
    for (const auto& p : particles) {
        int32_t fluid_type = p.fluidType;
        binaryData.write(reinterpret_cast<char*>(&fluid_type), sizeof(int32_t));
    }
    ofs << "</PointData>" << endl;
    /// ------------------
    /// ----- Cells -----
    /// ------------------
    ofs << "<Cells>" << endl;
    // connectivity
    ofs << dataArrayBegin("Int64", "connectivity", 1, "appended", binaryData.tellp()) << endl;
    ofs << dataArrayEnd() << endl;
    uint64_t length_connectivity = particles.size() * sizeof(int64_t);
    binaryData.write(reinterpret_cast<char*>(&length_connectivity), sizeof(uint64_t));
    for (size_t i = 0; i < particles.size(); i++) {
        int64_t connectivity = i;
        binaryData.write(reinterpret_cast<char*>(&connectivity), sizeof(int64_t));
    }
    // offsets
    ofs << dataArrayBegin("Int64", "offsets", 1, "appended", binaryData.tellp()) << endl;
    ofs << dataArrayEnd() << endl;
    uint64_t length_offset = particles.size() * sizeof(int64_t);
    binaryData.write(reinterpret_cast<char*>(&length_offset), sizeof(uint64_t));
    for (size_t i = 0; i < particles.size(); i++) {
        int64_t offset = i + 1;
        binaryData.write(reinterpret_cast<char*>(&offset), sizeof(int64_t));
    }
    // types
    ofs << dataArrayBegin("UInt8", "types", 1, "appended", binaryData.tellp()) << endl;
    ofs << dataArrayEnd() << endl;
    uint64_t length_types = particles.size() * sizeof(uint8_t);
    binaryData.write(reinterpret_cast<char*>(&length_types), sizeof(uint64_t));
    for (const auto& p : particles) {
        uint8_t type = 1; // 1: particle
        binaryData.write(reinterpret_cast<char*>(&type), sizeof(uint8_t));
    }
    ofs << "</Cells>" << endl;
    ofs << "</Piece>" << endl;
    // ---------------------
    // ---- Field data  ----
    // ---------------------
    ofs << "<FieldData>" << endl;
    ofs << "<DataArray type=\"Float64\" Name=\"Time\" NumberOfTuples=\"1\" format=\"appended\" offset=\""
        << binaryData.tellp() << "\"/>" << endl;
    // Time
    uint64_t length_time = sizeof(double);
    binaryData.write(reinterpret_cast<char*>(&length_time), sizeof(uint64_t));
    double time_copied = time; // copy time to use reinterpret_cast
    binaryData.write(reinterpret_cast<char*>(&time_copied), sizeof(double));
    ofs << "</FieldData>" << endl;
    ofs << "</UnstructuredGrid>" << endl;
    // ---------------------
    // --- Appended Data ---
    // ---------------------
    ofs << "<AppendedData encoding=\"raw\">" << endl;
    ofs << "_" << binaryData.str() << endl;
    ofs << "</AppendedData>" << endl;
    ofs << "</VTKFile>" << endl;
}

void ParticlesExporter::toVtu(
    const std::filesystem::path& path, const double& time, const double& n0ForNumberDensity, const bool binary
) {
    if (binary) {
        toVtuBinary(path, time, n0ForNumberDensity);
    } else {
        toVtuAscii(path, time, n0ForNumberDensity);
    }
}

void ParticlesExporter::toCsv(const fs::path& path, const double& time) {
    std::ofstream ofs(path);
    if (ofs.fail()) {
        cerr << "cannot write " << path << endl;
        std::exit(-1);
    }

    ofs << time << endl;
    ofs << particles.size() << endl;
    ofs << "type,fluidType,x,y,z,vx,vy,vz" << endl;
    for (const auto& p : particles) {
        ofs << static_cast<int>(p.type) << ",";
        ofs << p.fluidType << ",";
        ofs << p.position.x() << "," << p.position.y() << "," << p.position.z() << ",";
        ofs << p.velocity.x() << "," << p.velocity.y() << "," << p.velocity.z();
        ofs << endl;
    }
}
