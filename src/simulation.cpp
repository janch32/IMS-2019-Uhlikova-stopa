#include "simulation.hpp"

Simulation::Simulation(ProgramArgs args){
    buildingVolume = args.V;
    buildingEfficiency = args.K;
    tempTarget = args.tt;
	serverNightCycle = args.nightSpecified;
	serverHeatDay = args.P;
	serverHeatNight = args.nP;
	workStartHour = args.startWork;
	workEndHour = args.endWork;
}

void Simulation::runAll()
{   
	// Získáme recommendedHeat
	calculateRecommendedHeat();

	// Topení pouze plynovým kotlem
	run(false, true);

	// Emise CO2 zemního plynu jsou 55.4kg/GJ viz Vyhláška č. 480/2012 Sb.
	// J -> GJ * emiseZemníhoPlynu
	// Výsledkem jsou kg CO2 vypuštěného do vzduchu
	allCO2 = (boilerHeatProduced / 1000000000) * 55.4;

	// Kombinované topení
	run(true, true);
	hybridCO2 = (boilerHeatProduced / 1000000000) * 55.4;

	// Topení pouze odpadním teplem serveru
	// Získáme efficiency a serverFail
	run(true, false);
}

void Simulation::run(bool serverHeat, bool boilerHeat)
{
	bool heating = true;
	bool boilerOn = false;
	long iterations = 0;
	long iterationsHeated = 0;
	tempIn = tempTarget;
	efficiency = 0;
	boilerHeatProduced = 0;
	serverFail = false;
	
	for (int d = 1; d <= OutsideTemp::days; ++d)
	{
		for (int s = 0; s < 60*60*24; s += TIME_INTERVAL)
		{
			// Simulujeme pouze topnou sezónu (září - začátek dubna)
			if (d > 100 && d < 265) continue;

			heatingPower = serverHeat ? getServerHeat(s) : 0;

			if (tempIn > tempTarget + 2) heating = false;
			else if (tempIn < tempTarget) heating = true;

			if (tempIn > tempTarget + 2) boilerOn = false;
			else if (tempIn < tempTarget - 1)
		 	{
				serverFail = true;
				boilerOn = boilerHeat;
			} 

			tempOut = OutsideTemp::getTemp(d, s);
			if(boilerOn) heatingPower += recommendedHeat;
			if(!heating) heatingPower = 0;
			
			recalculateInnerTemp();

			// Pokud je spuštěn bojler, uložíme si, kolik J tepla vytvořil
			if (boilerOn && heating) boilerHeatProduced += recommendedHeat * TIME_INTERVAL;

			if(heating) iterationsHeated++;
			iterations++;
		}
	}

	efficiency = iterationsHeated / (double)iterations;
}

void Simulation::recalculateInnerTemp()
{
    // Měrná tepelná kapacita vzduchu (J/(kg * K))
    // @see https://www.tzb-info.cz/tabulky-a-vypocty/38-fyzikalni-hodnoty-pro-suchy-vzduch-pri-tlaku-100-kpa
    double c = 1000;
    
    // Hmotnost vzduchu v místnosti (kg)
    // Pro zjednudušení výpočtů je použita konstanta 1.2 pro hustotu vzduchu
	double m = 1.2 * buildingVolume;
	
    // Tepelný zisk topením (J)
    double Qt = heatingPower * TIME_INTERVAL;
    
    // Tepelná ztráta budovy (J)
	double Qz = buildingEfficiency * (tempIn - tempOut) * TIME_INTERVAL;

    // Výsledná teplota místnosti po časovém intervalu TIME_INTERVAL
	tempIn += (Qt - Qz) / (c * m);
}

double Simulation::getServerHeat(int time)
{
	if(!serverNightCycle) return serverHeatDay;

	time = time / 3600;
	if (time > workStartHour && time < workEndHour) return serverHeatNight;
	else return serverHeatDay;
}

void Simulation::calculateRecommendedHeat()
{
	tempIn = tempTarget;
    double tempInOld = tempTarget;
	bool heating = true;
	
	for (int d = 1; d <= OutsideTemp::days; ++d)
	{
		for (int s = 0; s < 60*60*24; s += TIME_INTERVAL)
		{
			if (d > 100 && d < 265) continue;

			// Určení vhodného výkonu topení
			if(tempIn > tempTarget + 2) heating = false;
			else if(tempIn < tempTarget) heating = true;

			tempOut = OutsideTemp::getTemp(d, s);
			tempInOld = tempIn;
			heatingPower = heating ? recommendedHeat : 0;
			recalculateInnerTemp();

			if (heating && (tempInOld > tempIn))
			{
				recommendedHeat += HEATING_STEP_PRECISION;
			}
		}
	}
}
