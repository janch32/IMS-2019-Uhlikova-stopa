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

void calcViability(double ti, double K, double P, double V)
{
	double to;

	for (int d = 0; d < 365; ++d)
	{
		cout << "\nDay: " << d + 1 << "\n\n";

		for (int h = 0; h < 24; ++h)
		{
			to = calcCurrentOuterTemp(d, h);

			ti = calcNewInnerTemp(ti, to, K, P, V);

			cout << "Inside: " << ti << " °C" << "\t\t\tOutside: " << to << " °C\n";
		}
	}
}

double calcNewInnerTemp(double ti, double to, double K, double P, double V)
{
	double t;		// (s)	Časový interval měření

	double m; 		// (Kg) Hmotnost vzduchu v místnosti
	double c;		// 		Měrná tepelná kapacita vzduchu

	double Qt;		// (J) 	Tepelný zisk topením
	double Qz;		// (J) 	Tepelná ztráta zvenku

	t = 3600;
	c = 1000.0;
	m = 1.2 * V;
	Qt = P * t;
	Qz = K * (ti - to) * t;

	ti = ti + ((Qt - Qz) / (c * m)); 	// Výsledná teplota místnosti ti po časovém intervalu t

	if (ti > 25.0) return 25.0;			// Přebytečné teplo je vypouštěno ven
	 return ti;
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

double calcCurrentOuterTemp(int day, int hour)
{
	if (day == 0 && hour < 7)	// Integritní omezení (nelze sáhnout mimo rozsah dat)
	{
		return morn[day];
	}

	if (day == 364 && hour >= 21)	// Integritní omezení (nelze sáhnout mimo rozsah dat)
	{
		return eve[day];
	}

	double t1;	// Nástupní teplota úseku
	double t2;	// Konečná teplota úseku

	int current;	// Současná pozice času ve zvoleném okně

	int win = 10;		// Velikost časového okna

	if (hour >=7 && hour < 21)
	{
		win = 7;
	}

	if (hour < 7)		// Dnešní noc
	{
		t1 = eve[day - 1];
		t2 = morn[day];
		current = hour + 3;
	}

	else if (hour < 14) 	// Dopoledne
	{
		t1 = morn[day];
		t2 = noon[day];
		current = hour - 7;
	}

	else if (hour < 21) 	// Odpoledne
	{
		t1 = noon[day];
		t2 = eve[day];
		current = hour - 14;
	}

	else	// Příští noc
	{
		t1 = eve[day];
		t2 = morn[day + 1];
		current = hour - 21;
	}
	
	return (t1 + (((double)current / (double)win) * (t2 - t1)));
}