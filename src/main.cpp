#include "main.hpp"

int main(int argc, char *const *argv)
{
	input args;

	try
	{
		parseArgs(argc, argv, &args);
	}
	catch(const std::exception& e)
	{
		cerr << "Exception: " << e.what() << endl;
		return -1;
	}

	output data = calcViability(&args);

	dumpData(data);

	return 0;
}

#define TIME_INTERVAL 60 // Sekundy
#define HEATING_STEP_PRECISION 1000.0 // Inkrement výkonu topení při kalibraci kanceláře

output calcViability(input *args)
{
	double serverHeat = args->P;	// Teplota serveru v závislosti na denní době
	double to; 						// Teplota venku
	long steps = 0;					// Počet měřených minut

	// Běh k určení vhodného výkonu topení
	double tiPre = args->tt;
	double tiPreOld = args->tt;
	bool heatingPre = true;
	double heatStrenght = 0.0;

	// Simulace topení serverem
	double tiS = args->tt;
	bool heatingS = true;
	long heatedStepsS = 0;
	bool serversFailedS = false;

	// Simulace topení samotným bojlerem
	double tiB = args->tt;
	bool heatingB = true;
	double heatConsumedB = 0.0;

	// Simulace hybridního vytápění
	double tiSB = args->tt;
	bool heatingSB = true;
	long heatedStepsSB = 0;
	double heatConsumedSB = 0.0;
	double totalHeatSB = 0.0;
	bool boilerOnSB = false;

	for (int d = 0; d < 365; ++d)
	{
		for (int s = 0; s < 60*60*24; s += TIME_INTERVAL)
		{
			if (d > 100 && d < 265) continue;

			// Určení vhodného výkonu topení
			if(tiPre > args->tt + 2) heatingPre = false;
			else if(tiPre < args->tt) heatingPre = true;

			to = calcCurrentOuterTemp(d, s);
			tiPreOld = tiPre;
			tiPre = calcNewInnerTemp(tiPre, to, args->K, heatingPre ? heatStrenght : 0, args->V, TIME_INTERVAL);

			if (heatingPre && (tiPreOld > tiPre)) heatStrenght += HEATING_STEP_PRECISION;
		}
	}

	for (int d = 0; d < 365; ++d)
	{
		for (int s = 0; s < 60*60*24; s += TIME_INTERVAL)
		{
			if (d > 100 && d < 265) continue;

			if (args->nightSpecified) serverHeat = calcCurrentServerHeat(*args, s);
			
			// Bojler

			if (tiB > args->tt + 2) heatingB = false;
			else if (tiB < args->tt - 1) heatingB = true;
			

			to = calcCurrentOuterTemp(d, s);
			tiB = calcNewInnerTemp(tiB, to, args->K, heatingB ? heatStrenght : 0, args->V, TIME_INTERVAL);

			if (heatingB) heatConsumedB += heatStrenght * (TIME_INTERVAL / 3600.0);

			// Server

			if (tiS > args->tt + 2) heatingS = false;
			else if (tiS < args->tt) heatingS = true;

			heatedStepsS += heatingS;

			to = calcCurrentOuterTemp(d, s);
			tiS = calcNewInnerTemp(tiS, to, args->K, heatingS ? serverHeat : 0, args->V, TIME_INTERVAL);

			if (heatingS && (tiS < args->tt - 1)) serversFailedS = true;

			// Kombinované topení

			if (tiSB > args->tt + 2) heatingSB = false;
			else if (tiSB < args->tt) heatingSB = true;

			heatedStepsSB += heatingSB;

			if (tiSB < args->tt - 1)
			{
				totalHeatSB = heatStrenght;
				boilerOnSB = true;
			}

			else if (tiSB > args->tt + 2)
			{
				totalHeatSB = serverHeat;
				boilerOnSB = false;
			}

			to = calcCurrentOuterTemp(d, s);
			tiSB = calcNewInnerTemp(tiSB, to, args->K, heatingSB ? totalHeatSB : 0, args->V, TIME_INTERVAL);

			if (boilerOnSB && heatingSB) heatConsumedSB += heatStrenght * (TIME_INTERVAL / 3600.0);

			steps++;
		}
	}

	output result;

	result.allCO2 = (heatConsumedB / 1000000) * 0.2;
	result.hybridCO2 = (heatConsumedSB / 1000000) * 0.2;
	result.effectivity = (int)(heatedStepsS / (double)steps * 100);
	result.heatStrenght = heatStrenght;
	result.serverFail = serversFailedS;

	return result;
}

void dumpData(output data)
{
	cout << "Výsledky na základě teplotních dat topné sezóny zimy 2018" << endl;
	cout << "---------------------------------------------------------" << endl;

	cout << endl << "Vhodný výkon topení pro specifikovanou kancelář: " << endl 
	<< endl << "\t" << data.heatStrenght << " W" << endl;

	cout << endl << "Tun CO2 vytvořeno za rok normálního topení: " << endl 
	<< endl << "\t" << data.allCO2 << " t" << endl;

	if (!data.serverFail)
	{
		cout << endl << "Vaše samotné servery by kancelář vytopily." << endl;
	}

	else
	{
		cout << endl << "Vaše samotné servery by kancelář vytopit nezvládly." << endl;
		cout << endl << "Tun CO2 vytvořeno za rok hybridního topení: " << endl 
		<< endl << "\t" << data.hybridCO2 << " t " << "(" << (data.allCO2 - data.hybridCO2) 
		<< " t CO2 ušetřeno hybridním topením)" << endl;
	} 

	cout << endl << "Podíl využitého tepla ze serverů: " << endl 
	<< endl << "\t" << data.effectivity << "%" << endl;
}

double calcNewInnerTemp(double ti, double to, double K, double P, double V, int time)
{
	double m; 		// (Kg) Hmotnost vzduchu v místnosti
	double c;		// 		Měrná tepelná kapacita vzduchu

	double Qt;		// (J) 	Tepelný zisk topením
	double Qz;		// (J) 	Tepelná ztráta zvenku

	c = 1000.0;
	m = 1.2 * V;
	Qt = P * time;
	Qz = K * (ti - to) * time;

	return ti + ((Qt - Qz) / (c * m)); 	// Výsledná teplota místnosti ti po časovém intervalu t
}

double calcCurrentServerHeat(input data, int time)
{
	time = time / 3600;

	if (time > data.startWork && time < data.endWork) return data.P;
	else return (((double)data.nightUsage * data.P) / 100);
}

void parseArgs(int argc, char *const *argv, input *args)
{
	int opt;

	bool ist = false;
	bool isK = false;
	bool isP = false;
	bool isV = false;
	bool iss = false;
	bool ise = false;
	bool isn = false;

	args->nightSpecified = false;

	while((opt = getopt(argc, argv, "-:t:K:P:V:s:e:n:")) != -1)  
    {  
        switch(opt)
        {
            case 't':
				if (ist) throw std::invalid_argument( "Duplikován přepínač -t" );

				args->tt = stod(optarg);

				ist = true;
				break;

            case 'K': 
				if (isK) throw std::invalid_argument( "Duplikován přepínač -K" );

				args->K = stod(optarg);
				isK = true;
                break;

			case 'P': 
				if (isP) throw std::invalid_argument( "Duplikován přepínač -P" );

				args->P = stod(optarg);
				isP = true;
                break;

			case 'V': 
				if (isV) throw std::invalid_argument( "Duplikován přepínač -V" );

				args->V = stod(optarg);
				isV = true;
                break;
			
			case 's': 
				if (iss) throw std::invalid_argument( "Duplikován přepínač -s" );

				args->startWork = stoi(optarg);
				iss = true;
                break;

			case 'e': 
				if (ise) throw std::invalid_argument( "Duplikován přepínač -e" );

				args->endWork = stoi(optarg);
				ise = true;
                break;

			case 'n': 
				if (isn) throw std::invalid_argument( "Duplikován přepínač -n" );

				args->nightUsage = stoi(optarg);
				args->nightSpecified = true;
				isn = true;
                break;

			case ':':
                throw std::invalid_argument( "Parametru chybí hodnota" );

			default:
				throw std::invalid_argument( "Nesrozumitelné argumenty" );
        }
    }

	if (!ist || !isK || !isP || !isV) 
		throw std::invalid_argument( "Všechny čtyři přepínače (-t, -V, -P, -K) jsou povinné." );

	if (!(iss && ise && isn) && !(!iss && !ise && !isn)) 
		throw std::invalid_argument( "Přepínače -s, -e a -n je třeba použít všechny, nebo žádný." );

	if (args->startWork > args->endWork)
		throw std::invalid_argument( "Pracovní doba lze udat pouze v rámci jednoho dne." );
}

double calcCurrentOuterTemp(int day, int sec)
{
	if (day == 0 && sec < 7*3600)	// Integritní omezení (nelze sáhnout mimo rozsah dat)
	{
		return morn[day];
	}

	if (day == 364 && sec >= 21*3600)	// Integritní omezení (nelze sáhnout mimo rozsah dat)
	{
		return eve[day];
	}

	double t1;	// Nástupní teplota úseku
	double t2;	// Konečná teplota úseku

	int current;	// Současná pozice času ve zvoleném okně

	int win = 10*3600;		// Velikost časového okna

	if (sec >=7*3600 && sec < 21*3600)
	{
		win = 7*3600;
	}

	if (sec < 7*3600)		// Dnešní noc
	{
		t1 = eve[day - 1];
		t2 = morn[day];
		current = sec + 3*3600;
	}

	else if (sec < 14*3600) 	// Dopoledne
	{
		t1 = morn[day];
		t2 = noon[day];
		current = sec - 7*3600;
	}

	else if (sec < 21*3600) 	// Odpoledne
	{
		t1 = noon[day];
		t2 = eve[day];
		current = sec - 14*3600;
	}

	else	// Příští noc
	{
		t1 = eve[day];
		t2 = morn[day + 1];
		current = sec - 21*3600;
	}
	
	return (t1 + (((double)current / (double)win) * (t2 - t1)));
}
