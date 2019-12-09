#include "program_args.hpp"

using namespace std;

void parseArgs(int argc, char *const *argv, ProgramArgs *args)
{
	bool ist = false;
	bool isK = false;
	bool isP = false;
	bool isV = false;
	bool iss = false;
	bool ise = false;
	bool isn = false;

	args->nightSpecified = false;

	int opt;
	while((opt = getopt(argc, argv, "-:t:K:P:V:s:e:n:")) != -1)  
    {  
        switch(opt)
        {
            case 't':
				if (ist) throw invalid_argument( "Duplikován přepínač -t" );

				args->tt = stod(optarg);

				ist = true;
				break;

            case 'K': 
				if (isK) throw invalid_argument( "Duplikován přepínač -K" );

				args->K = stod(optarg);
				isK = true;
                break;

			case 'P': 
				if (isP) throw invalid_argument( "Duplikován přepínač -P" );

				args->P = stod(optarg);
				isP = true;
                break;

			case 'V': 
				if (isV) throw invalid_argument( "Duplikován přepínač -V" );

				args->V = stod(optarg);
				isV = true;
                break;
			
			case 's': 
				if (iss) throw invalid_argument( "Duplikován přepínač -s" );

				args->startWork = stoi(optarg);
				iss = true;
                break;

			case 'e': 
				if (ise) throw invalid_argument( "Duplikován přepínač -e" );

				args->endWork = stoi(optarg);
				ise = true;
                break;

			case 'n': 
				if (isn) throw invalid_argument( "Duplikován přepínač -n" );

				args->nP = stoi(optarg);
				args->nightSpecified = true;
				isn = true;
                break;

			case ':':
                throw invalid_argument( "Parametru chybí hodnota" );

			default:
				throw invalid_argument( "Nesrozumitelné argumenty" );
        }
    }

	if (!ist || !isK || !isP || !isV) 
		throw invalid_argument( "Všechny čtyři přepínače (-t, -V, -P, -K) jsou povinné." );

	if (!(iss && ise && isn) && !(!iss && !ise && !isn)) 
		throw invalid_argument( "Přepínače -s, -e a -n je třeba použít všechny, nebo žádný." );

	if (args->startWork > args->endWork)
		throw invalid_argument( "Pracovní doba lze udat pouze v rámci jednoho dne." );
}
