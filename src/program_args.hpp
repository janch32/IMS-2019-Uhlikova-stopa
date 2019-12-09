#pragma once
#include <stdexcept>
#include <getopt.h>

struct ProgramArgs{
    double tt;
	double K;
	double P;
	double nP;
	double V;
	int startWork;
	int endWork;
	bool nightSpecified;
};

void parseArgs(int argc, char *const *argv, ProgramArgs *args);
