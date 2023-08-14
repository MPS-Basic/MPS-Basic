#pragma once
#include "common.hpp"
#include <iostream>

void startSimulation(clock_t& simStartTime);
void endSimulation(clock_t& simStartTime);
void calSecondMinuteHour(int& second, int& minute, int& hour);

void startSimulation(clock_t& simStartTime) {
	std::cout << std::endl;
	std::cout << "*** START SIMULATION ***" << std::endl;
	simStartTime = std::clock();
}

void endSimulation(clock_t& simStartTime, FILE* logFile) {
	clock_t simEndTime = std::clock();

	int second, minute, hour;
	second = (double) (simEndTime - simStartTime) / CLOCKS_PER_SEC;
	calSecondMinuteHour(second, minute, hour);
	printf("\nTotal Simulation Time = %dh %02dm %02ds\n", hour, minute, second);

	// fclose(logFile);

	std::cout << std::endl;
	std::cout << "*** END SIMULATION ***" << std::endl;
}

void calSecondMinuteHour(int& second, int& minute, int& hour) {
	hour = second / 3600;
	second %= 3600;
	minute = second / 60;
	second %= 60;
}