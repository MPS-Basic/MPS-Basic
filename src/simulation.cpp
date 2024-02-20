#include "simulation.hpp"
#include "input.hpp"
#include <cstdio>
#include <iostream>

using namespace std;


Simulation::Simulation(fs::path& settingPath) {
	Input input = loader.load(settingPath);
	saver       = Saver(input.settings.outputDirectory);

	mps          = MPS(input);
	startTime    = input.startTime;
	time         = startTime;
	endTime      = input.settings.endTime;
	dt           = input.settings.dt;
	outputPeriod = input.settings.outputPeriod;
}

void Simulation::run() {
	startSimulation();
	mps.init();
	saver.save(mps, time);

	while (time < endTime) {
		auto timeStepStartTime = std::chrono::system_clock::now();

		mps.stepForward();
		timeStep++;
		time += dt;

		auto timeStepEndTime = std::chrono::system_clock::now();

		timeStepReport(timeStepStartTime, timeStepEndTime);
		if (saveCondition()) {
			saver.save(mps, time);
		}
	}
	endSimulation();
}

void Simulation::startSimulation() {
	std::cout << std::endl;
	std::cout << "*** START SIMULATION ***" << std::endl;
	realStartTime = std::chrono::system_clock::now();
}

void Simulation::endSimulation() {
	realEndTime = std::chrono::system_clock::now();
	std::cout << std::endl;
	std::cout << "Total Simulation time = " << calHourMinuteSecond(realEndTime - realStartTime) << std::endl;

	std::cout << std::endl;
	std::cout << "*** END SIMULATION ***" << std::endl;
}

void Simulation::timeStepReport(const std::chrono::system_clock::time_point& timeStepStartTime,
                                const std::chrono::system_clock::time_point& timeStepEndTime) {

	auto elapsedTime = timeStepEndTime - realStartTime;
	auto elapsed     = "elapsed=" + calHourMinuteSecond(elapsedTime);

	double ave = 0.0;
	if (timeStep != 0) {
		ave = (double) (std::chrono::duration_cast<std::chrono::nanoseconds>(elapsedTime).count()) / (timeStep * 1e9);
	}

	std::string remain = "remain=";
	if (timeStep == 0) {
		remain += "-h --m --s";
	} else {
		auto totalTime = std::chrono::nanoseconds((int64_t) (ave * (endTime - startTime) / dt * 1e9));
		remain += calHourMinuteSecond(totalTime - elapsedTime);
	}
	double last =
	    std::chrono::duration_cast<std::chrono::nanoseconds>(timeStepEndTime - timeStepStartTime).count() * 1e-9;

	// terminal output
	printf("%d: dt=%.gs   t=%.3lfs   fin=%.1lfs   %s   %s   ave=%.3lfs/step   "
	       "last=%.3lfs/step   out=%dfiles   Courant=%.2lf\n",
	       timeStep, dt, time, endTime, elapsed.c_str(), remain.c_str(), ave, last, saver.fileNumber, mps.courant);

	// error output
	fprintf(stderr, "%4d: t=%.3lfs\n", timeStep, time);
}

bool Simulation::saveCondition() {
	// NOTE: Is fileNumber really necessary?
	return time - startTime >= outputPeriod * double(saver.fileNumber);
}

// NOTE: If this function is also needed in other classes, it should be moved to a separate file.
std::string Simulation::getCurrentTimeString() {
	auto currentTime  = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::tm* timeInfo = std::localtime(&currentTime);
	std::stringstream formattedTime;
	formattedTime << std::put_time(timeInfo, "%Y-%m-%d_%H-%M-%S");
	std::string formattedTimeString = formattedTime.str();

	return formattedTimeString;
}