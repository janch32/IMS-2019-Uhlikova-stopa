#pragma once
#include "program_args.hpp"
#include "year_temp_outside.hpp"

class Simulation
{
public:
	double recommendedHeat = 0;

	double allCO2 = 0;
	double hybridCO2 = 0;
	double efficiency = 0;
	bool serverFail = false;

	double buildingEfficiency = 0;
	double buildingVolume = 0;
	double tempTarget = 0;
	bool serverNightCycle = false;
	double serverHeatDay = 0;
	double serverHeatNight = 0;
	int workStartHour = 0;
	int workEndHour = 0;

	Simulation(ProgramArgs args);
	void runAll();
	void run(bool serverHeat, bool boilerHeat);

private:
	/** Časový interval jednoho kroku výpočtu (v sekundách) */
	#define TIME_INTERVAL 60

	/** Inkrement výkonu topení při kalibraci kanceláře */
	#define HEATING_STEP_PRECISION 1000.0 
	
	double tempIn = 0;
	double tempOut = 0;
	double heatingPower = 0;
	double boilerHeatProduced = 0;
	double time = 0;

	double getCurrentServerHeat(ProgramArgs data, int time);
	void calculateRecommendedHeat();
	void recalculateInnerTemp();
	double getServerHeat(int time);
};
