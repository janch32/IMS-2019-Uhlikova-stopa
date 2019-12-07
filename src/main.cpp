#include "main.hpp"

int main(int argc, char *const *argv)
{
	double ti = 0.0;
	double K = 0.0;
	double P = 0.0;
	double V = 0.0;

	try
	{
		parseArgs(argc, argv, &ti, &K, &P, &V);
	}
	catch(const std::exception& e)
	{
		cerr << "Exception: " << e.what() << endl;
		return -1;
	}

	calcViability(ti, K, P, V);

	return 0;
}

#define TIME_INTERVAL 60 // sec
#define HEAT_STRENGHT 10000.0 // Watt

void calcViability(double tt, double K, double P, double V)
{
	double toS;
	double tiS = tt;

	double toB;
	double tiB = tt;

	double toSB;
	double tiSB = tt;

	long heatedStepsS = 0;
	long stepsS = 0;

	long heatedStepsSB = 0;
	long stepsSB = 0;

	bool heatingS = true;
	bool heatingB = true;
	bool heatingSB = true;

	double heatConsumedB = 0.0;
	double heatConsumedSB = 0.0;

	double totalHeatSB = 0.0;

	bool boilerOn = false;

	for (int d = 0; d < 365; ++d)
	{
		for (int s = 0; s < 60*60*24; s += TIME_INTERVAL)
		{
			// Server

			if(tiS > tt + 2) heatingS = false;
			else if(tiS < tt) heatingS = true;
			
			if (d > 100 && d < 265) heatingS = false;
			else stepsS++;

			heatedStepsS += heatingS;

			toS = calcCurrentOuterTemp(d, s);
			tiS = calcNewInnerTemp(tiS, toS, K, heatingS ? P : 0, V, TIME_INTERVAL);

			cout << tiS << "," << toS << endl;

			// Bojler

			if(tiB > tt + 2) heatingB = false;
			else if(tiB < tt) heatingB = true;
			
			if (d > 100 && d < 265) heatingB = false;

			toB = calcCurrentOuterTemp(d, s);
			tiB = calcNewInnerTemp(tiB, toB, K, heatingB ? HEAT_STRENGHT : 0, V, TIME_INTERVAL);

			if (heatingB) heatConsumedB += HEAT_STRENGHT * (TIME_INTERVAL / 3600.0);

			// Kombinované topení

			if(tiSB > tt + 2) heatingSB = false;
			else if(tiSB < tt) heatingSB = true;
			
			if (d > 100 && d < 265) heatingSB = false;
			else stepsSB++;

			heatedStepsSB += heatingSB;

			if (tiSB < tt - 2)
			{
				totalHeatSB = HEAT_STRENGHT + P;
				boilerOn = true;
			}

			else if (tiSB > tt + 2)
			{
				totalHeatSB = P;
				boilerOn = false;
			}

			toSB = calcCurrentOuterTemp(d, s);
			tiSB = calcNewInnerTemp(tiSB, toSB, K, heatingSB ? totalHeatSB : 0, V, TIME_INTERVAL);

			if (boilerOn && heatingSB) heatConsumedSB += HEAT_STRENGHT * (TIME_INTERVAL / 3600.0);
		}
	}

	// Topení serverem
	cout << endl << heatedStepsS / (double)stepsS * 100 << "%" << endl;

	// Kombinované topení

	cout << endl << "Tun CO2 vytvořeno navíc za rok hybridního topení: " << (heatConsumedSB / 1000000) * 0.2 << " t" << endl;

	// Normální topení
	cout << endl << "Tun CO2 vytvořeno navíc za rok normálního topení: " << (heatConsumedB / 1000000) * 0.2 << " t" << endl;
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

	return ti + ((Qt - Qz) / (c * m)); // Výsledná teplota místnosti ti po časovém intervalu t
}

void parseArgs(int argc, char *const *argv, double *t, double *K, double *P, double *V)
{
	int opt;
	bool ist = false;
	bool isK = false;
	bool isP = false;
	bool isV = false;

	while((opt = getopt(argc, argv, "-:t:K:P:V:")) != -1)  
    {  
        switch(opt)
        {
            case 't':
				if (ist) throw std::invalid_argument( "Duplikován přepínač -t" );

				*t = stod(optarg);

				ist = true;
				break;

            case 'K': 
				if (isK) throw std::invalid_argument( "Duplikován přepínač -K" );

				*K = stod(optarg);
				isK = true;
                break;

			case 'P': 
				if (isP) throw std::invalid_argument( "Duplikován přepínač -P" );

				*P = stod(optarg);
				isP = true;
                break;

			case 'V': 
				if (isV) throw std::invalid_argument( "Duplikován přepínač -V" );

				*V = stod(optarg);
				isV = true;
                break;

			case ':':
                throw std::invalid_argument( "Parametru chybí hodnota" );

			default:
				throw std::invalid_argument( "Nesrozumitelné argumenty" );
        }
    }

	if (!ist || !isK || !isP || !isV) throw std::invalid_argument( "Všechny čtyři přepínače (-t, -V, -P, -K) jsou povinné." );
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
