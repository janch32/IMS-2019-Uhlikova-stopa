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

void calcViability(double tt, double K, double P, double V)
{
	double to;
	double ti = tt;

	for (int d = 0; d < 365; ++d)
	{
		//cout << "\nDay: " << d + 1 << "\n\n";

		int heatedSteps = 0;
		int steps = 0;

		bool heating = true;
		
		for (int s = 0; s < 60*60*24; s += TIME_INTERVAL)
		{
			heatedSteps += heating;
			steps++;

			if(ti > tt + 2) heating = false;
			else if(ti < tt) heating = true;

			to = calcCurrentOuterTemp(d, s);
			ti = calcNewInnerTemp(ti, to, K, heating ? P : 0, V, TIME_INTERVAL);

			cout << ti << "," << to << endl;
		}

		//cout << heatedSteps / (double)steps * 100 << "%" << endl;
	}
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
