#pragma once

#include "common.hpp"
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

    std::chrono::system_clock::time_point realStartTime, realEndTime;
    double startTime, time, endTime, dt;
    double outputPeriod;
    int timeStep = 0;

    Simulation(std::filesystem::path& settingPath, std::filesystem::path& outputDirectory);

    void run();

private:
    void startSimulation();

    template <typename Rep, typename Period> std::string calHourMinuteSecond(std::chrono::duration<Rep, Period> d) {
        auto h = std::chrono::duration_cast<std::chrono::hours>(d);
        d -= h;
        auto m = std::chrono::duration_cast<std::chrono::minutes>(d);
        d -= m;
        auto s = std::chrono::duration_cast<std::chrono::seconds>(d);

        std::stringstream out;
        out << h.count() << "h ";
        out << std::setfill('0') << std::setw(2) << m.count() << "m " << s.count() << "s";
        return out.str();
    }

    void endSimulation();

    /**
     * @brief Report time step information to the console
     */
    void timeStepReport(
        const std::chrono::system_clock::time_point& timeStepStartTime,
        const std::chrono::system_clock::time_point& timeStepEndTime
    );

    bool saveCondition();

    // NOTE: If this function is also needed in other classes, it should be moved to a separate file.
    std::string getCurrentTimeString();
};
