#pragma once

#include "common.hpp"
#include "input.hpp"
#include "loader.hpp"
#include "mps.hpp"
#include "saver.hpp"
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <string>

/**
 * Simulation class
 *
 * This class is responsible for the simulation process. It loads the input,
 * saves the results, and manages the time. It also contains the main loop of
 * the simulation.
 *
 */
class Simulation {
public:
	MPS mps;
	Loader loader;
	Saver saver;

	chrono::system_clock::time_point realStartTime, realEndTime;
	double startTime, time, endTime, dt;
	double outputPeriod;
	int timeStep = 0;

	Simulation(fs::path& settingPath) {
		Input input = loader.load(settingPath);
		saver       = Saver(input.settings.outputDirectory);

		mps          = MPS(input);
		startTime    = input.startTime;
		time         = startTime;
		endTime      = input.settings.endTime;
		dt           = input.settings.dt;
		outputPeriod = input.settings.outputPeriod;
	};

	void run() {
		startSimulation();
		mps.init();
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
	};

private:
	void startSimulation() {
		cout << endl;
		cout << "*** START SIMULATION ***" << endl;
		realStartTime = chrono::system_clock::now();
	}

	template <typename Rep, typename Period> std::string calHourMinuteSecond(chrono::duration<Rep, Period> d) {
		auto h = chrono::duration_cast<chrono::hours>(d);
		d -= h;
		auto m = chrono::duration_cast<chrono::minutes>(d);
		d -= m;
		auto s = chrono::duration_cast<chrono::seconds>(d);

		std::stringstream out;
		out << h.count() << "h ";
		out << std::setfill('0') << std::setw(2) << m.count() << "m " << s.count() << "s";
		return out.str();
	}

	void endSimulation() {
		realEndTime = chrono::system_clock::now();
		cout << endl;
		cout << "Total Simulation time = " << calHourMinuteSecond(realEndTime - realStartTime) << endl;

		cout << endl;
		cout << "*** END SIMULATION ***" << endl;
	}

	// Report time step information to the console
	void timeStepReport(const chrono::system_clock::time_point& timeStepStartTime,
	                    const chrono::system_clock::time_point& timeStepEndTime) {

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
		printf("%d: dt=%.gs   t=%.3lfs   fin=%.1lfs   %s   %s   ave=%.3lfs/step   "
		       "last=%.3lfs/step   out=%dfiles   Courant=%.2lf\n",
		       timeStep, dt, time, endTime, elapsed.c_str(), remain.c_str(), ave, last, saver.fileNumber, mps.courant);

		// error output
		fprintf(stderr, "%4d: t=%.3lfs\n", timeStep, time);
	}

	bool saveCondition() {
		// NOTE: Is fileNumber really necessary?
		return time - startTime >= outputPeriod * double(saver.fileNumber);
	}

	// NOTE: If this function is also needed in other classes, it should be moved to a separate file.
	std::string getCurrentTimeString() {
		auto currentTime  = chrono::system_clock::to_time_t(chrono::system_clock::now());
		std::tm* timeInfo = std::localtime(&currentTime);
		std::stringstream formattedTime;
		formattedTime << std::put_time(timeInfo, "%Y-%m-%d_%H-%M-%S");
		std::string formattedTimeString = formattedTime.str();

		return formattedTimeString;
	}
};
