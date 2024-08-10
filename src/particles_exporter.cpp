#include "particles_exporter.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vtkDataCompressor.h>
#include <vtkDoubleArray.h>
#include <vtkFieldData.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVertex.h>
#include <vtkXMLUnstructuredGridWriter.h>

using std::cerr;
using std::endl;
namespace fs = std::filesystem;

void ParticlesExporter::setParticles(const Particles& particles) {
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

void ParticlesExporter::toVtu(const fs::path& path, const double& time, const double& n0ForNumberDensity) {
    vtkNew<vtkUnstructuredGrid> grid;

    // -----------------
    // --- Time data ---
    // -----------------
    vtkNew<vtkDoubleArray> timeArray;
    timeArray->SetName("Time");
    timeArray->SetNumberOfComponents(1);
    timeArray->InsertNextValue(time);
    grid->GetFieldData()->AddArray(timeArray);

    /// ------------------
    /// ----- Points -----
    /// ------------------
    vtkNew<vtkPoints> points;
    for (const auto& p : particles) {
        points->InsertNextPoint(p.position.x(), p.position.y(), p.position.z());
    }
    grid->SetPoints(points);

    // ---------------------
    // ----- PointData -----
    // ---------------------
    // Particle Type
    vtkNew<vtkIntArray> particleTypeArray;
    particleTypeArray->SetName("Particle Type");
    particleTypeArray->SetNumberOfComponents(1);
    for (auto& p : particles) {
        particleTypeArray->InsertNextValue(static_cast<int>(p.type));
    }
    grid->GetPointData()->AddArray(particleTypeArray);
    // Velocity
    vtkNew<vtkDoubleArray> velocityArray;
    velocityArray->SetName("Velocity");
    velocityArray->SetNumberOfComponents(3);
    for (const auto& p : particles) {
        velocityArray->InsertNextTuple3(p.velocity.x(), p.velocity.y(), p.velocity.z());
    }
    grid->GetPointData()->AddArray(velocityArray);
    // Pressure
    vtkNew<vtkDoubleArray> pressureArray;
    pressureArray->SetName("Pressure");
    pressureArray->SetNumberOfComponents(1);
    for (const auto& p : particles) {
        pressureArray->InsertNextValue(p.pressure);
    }
    grid->GetPointData()->AddArray(pressureArray);
    // Number Density
    vtkNew<vtkDoubleArray> numberDensityArray;
    numberDensityArray->SetName("Number Density");
    numberDensityArray->SetNumberOfComponents(1);
    for (const auto& p : particles) {
        numberDensityArray->InsertNextValue(p.numberDensity);
    }
    grid->GetPointData()->AddArray(numberDensityArray);
    // Number Density Ratio
    vtkNew<vtkDoubleArray> numberDensityRatioArray;
    numberDensityRatioArray->SetName("Number Density Ratio");
    numberDensityRatioArray->SetNumberOfComponents(1);
    for (auto& p : particles) {
        numberDensityRatioArray->InsertNextValue(p.numberDensity / n0ForNumberDensity);
    }
    grid->GetPointData()->AddArray(numberDensityRatioArray);
    // Boundary Condition
    vtkNew<vtkIntArray> boundaryConditionArray;
    boundaryConditionArray->SetName("Boundary Condition");
    boundaryConditionArray->SetNumberOfComponents(1);
    for (auto& p : particles) {
        boundaryConditionArray->InsertNextValue(static_cast<int>(p.boundaryCondition));
    }
    grid->GetPointData()->AddArray(boundaryConditionArray);
    // Fluid Type
    vtkNew<vtkIntArray> fluidTypeArray;
    fluidTypeArray->SetName("Fluid Type");
    fluidTypeArray->SetNumberOfComponents(1);
    for (auto& p : particles) {
        fluidTypeArray->InsertNextValue(p.fluidType);
    }
    grid->GetPointData()->AddArray(fluidTypeArray);

    // ---------------------
    // ----- CellData ------
    // ---------------------
    vtkNew<vtkCellArray> cells;
    for (vtkIdType i = 0; i < particles.size(); i++) {
        vtkNew<vtkVertex> vertex;
        vertex->GetPointIds()->SetId(0, i);
        cells->InsertNextCell(vertex);
    }
    grid->SetCells(VTK_VERTEX, cells);

    // ---------------------
    // ---- Write file -----
    // ---------------------
    vtkNew<vtkXMLUnstructuredGridWriter> writer;
    writer->SetFileName(path.c_str());
    writer->SetInputData(grid);
    writer->SetCompressorTypeToZLib();
    writer->SetDataModeToAppended();
    writer->SetEncodeAppendedData(false);
    writer->Write();
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
