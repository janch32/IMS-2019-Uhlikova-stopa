#include "main.hpp"

int main(int argc, char *const *argv)
{
	double t = 0.0;
	double K = 0.0;
	double P = 0.0;
	double V = 0.0;

	try
	{
		parseArgs(argc, argv, &t, &K, &P, &V);
	}
	catch(const std::exception& e)
	{
		cerr << "Exception: " << e.what() << endl;
		return -1;
	}

	return 0;
}

double calcTemperature(double ti, double to, double t, double K, double P, double V)
{
	double m; 		// (Kg) Hmotnost vzduchu v místnosti
	double c;		// 		Měrná tepelná kapacita vzduchu

	double Qt;		// (J) 	Tepelný zisk topením
	double Qz;		// (J) 	Tepelná ztráta zvenku

	c = 1000.0;
	m = 1.2 * V;
	Qt = P * t;
	Qz = K * (ti - to) * t;

	return (ti + (Qt - Qz) / (c * m));  // Výsledná teplota místnosti ti po časovém intervalu t
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