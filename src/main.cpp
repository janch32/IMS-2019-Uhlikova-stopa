#include "main.hpp"

int main(int argc, char *const *argv)
{
	ProgramArgs args;

	try
	{
		parseArgs(argc, argv, &args);
	}
	catch (const std::exception &e)
	{
		cerr << "Chyba při zpracování vstupních parametrů: " << e.what() << endl;
		return -1;
	}

	auto sim = Simulation(args);
	sim.runAll();

	dumpData(sim);

	return 0;
}

void dumpData(Simulation data)
{
	cout << "Výsledky simulace topné sezóny podle teplotních dat zimy 2018" <<
	endl << "-------------------------------------------------------------" <<

	endl << endl << "Vhodný výkon topení pro specifikovanou kancelář: " << 
	endl << "\t" << data.recommendedHeat << " W" <<

	endl << endl << "Tun CO2 vytvořeno za rok normálního topení: " << 
	endl << "\t" << data.allCO2 / 1000 << " t" << endl;

	if (!data.serverFail)
	{
		cout << endl << "Vaše samotné servery by kancelář vytopily." << endl;
	}

	else
	{
		cout << endl << "Vaše samotné servery by kancelář vytopit nezvládly." << endl;
		cout << endl << "Tun CO2 vytvořeno za rok hybridního topení: " <<
		endl << "\t" << data.hybridCO2 / 1000 << " t (" << (data.allCO2 - data.hybridCO2) / 1000 
		<< " t CO2 ušetřeno hybridním topením)" << endl;
	} 

	cout << endl << "Podíl využitého tepla ze serverů: " << 
	endl << "\t" << data.efficiency * 100 << "%" << endl;
}
