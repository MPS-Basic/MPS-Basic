#include "simulation.hpp"
#include "input.hpp"
#include "pressure_calculator/explicit.hpp"
#include "pressure_calculator/implicit.hpp"
#include <cstdio>
#include <iostream>
#include <memory>

using std::cerr;
using std::cout;
using std::endl;
namespace fs     = std::filesystem;
namespace chrono = std::chrono;

Simulation::Simulation(fs::path& settingPath) {
    Input input = loader.load(settingPath);
    saver       = Saver(input.settings.outputDirectory);

    std::unique_ptr<PressureCalculator::Interface> pressureCalculator;
    if (input.settings.pressureCalculationMethod == "Implicit") {
        pressureCalculator.reset(new PressureCalculator::Implicit(
            input.settings.dim,
            input.settings.particleDistance,
            input.settings.re_forNumberDensity,
            input.settings.re_forLaplacian,
            input.settings.dt,
            input.settings.fluidDensity,
            input.settings.compressibility,
            input.settings.relaxationCoefficientForPressure
        ));

    } else if (input.settings.pressureCalculationMethod == "Explicit") {
        pressureCalculator.reset(new PressureCalculator::Explicit(
            input.settings.fluidDensity,
            input.settings.re_forNumberDensity,
            input.settings.soundSpeed,
            input.settings.dim,
            input.settings.particleDistance
        ));

    } else {
        cerr << "Invalid pressure calculation method: " << input.settings.pressureCalculationMethod << endl;
        cerr << "Please select either Implicit or Explicit." << endl;
        std::exit(-1);
    }

    mps          = MPS(input, std::move(pressureCalculator));
    startTime    = input.startTime;
    time         = startTime;
    endTime      = input.settings.endTime;
    dt           = input.settings.dt;
    outputPeriod = input.settings.outputPeriod;
}

void Simulation::run() {
    startSimulation();
    saver.save(mps, time);

    while (time < endTime) {
        auto timeStepStartTime = chrono::system_clock::now();

        mps.stepForward();
        timeStep++;
        time += dt;

        auto timeStepEndTime = chrono::system_clock::now();

        timeStepReport(timeStepStartTime, timeStepEndTime);
        if (saveCondition()) {
            saver.save(mps, time);
        }
    }
    endSimulation();
}

void Simulation::startSimulation() {
    cout << endl;
    cout << "*** START SIMULATION ***" << endl;
    realStartTime = chrono::system_clock::now();
}

void Simulation::endSimulation() {
    realEndTime = chrono::system_clock::now();
    cout << endl;
    cout << "Total Simulation time = " << calHourMinuteSecond(realEndTime - realStartTime) << endl;

    cout << endl;
    cout << "*** END SIMULATION ***" << endl;
}

void Simulation::timeStepReport(
    const chrono::system_clock::time_point& timeStepStartTime, const chrono::system_clock::time_point& timeStepEndTime
) {
    auto elapsedTime = timeStepEndTime - realStartTime;
    auto elapsed     = "elapsed=" + calHourMinuteSecond(elapsedTime);

    double ave = 0.0;
    if (timeStep != 0) {
        ave = (double) (chrono::duration_cast<chrono::nanoseconds>(elapsedTime).count()) / (timeStep * 1e9);
    }

    std::string remain = "remain=";
    if (timeStep == 0) {
        remain += "-h --m --s";
    } else {
        auto totalTime = chrono::nanoseconds((int64_t) (ave * (endTime - startTime) / dt * 1e9));
        remain += calHourMinuteSecond(totalTime - elapsedTime);
    }
    double last = chrono::duration_cast<chrono::nanoseconds>(timeStepEndTime - timeStepStartTime).count() * 1e-9;

    // terminal output
    printf(
        "%d: dt=%.gs   t=%.3lfs   fin=%.1lfs   %s   %s   ave=%.3lfs/step   "
        "last=%.3lfs/step   out=%dfiles   Courant=%.2lf\n",
        timeStep,
        dt,
        time,
        endTime,
        elapsed.c_str(),
        remain.c_str(),
        ave,
        last,
        saver.fileNumber,
        mps.courant
    );

    // error output
    fprintf(stderr, "%4d: t=%.3lfs\n", timeStep, time);
}

bool Simulation::saveCondition() {
    // NOTE: Is fileNumber really necessary?
    return time - startTime >= outputPeriod * double(saver.fileNumber);
}

// NOTE: If this function is also needed in other classes, it should be moved to a separate file.
std::string Simulation::getCurrentTimeString() {
    auto currentTime  = chrono::system_clock::to_time_t(chrono::system_clock::now());
    std::tm* timeInfo = std::localtime(&currentTime);
    std::stringstream formattedTime;
    formattedTime << std::put_time(timeInfo, "%Y-%m-%d_%H-%M-%S");
    std::string formattedTimeString = formattedTime.str();

    return formattedTimeString;
}
