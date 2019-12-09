#pragma once
#include <iostream>
#include <string>
#include <getopt.h>
#include "program_args.hpp"
#include "simulation.hpp"

using namespace std;

int main(int argc, char *const *argv);

void dumpData(Simulation data);
