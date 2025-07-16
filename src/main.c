// #include "simulation.h"
#include "simulation.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
  Simulation sim;

  if(argc > 2)
  {
    printf("USAGE: %s <config_path>", argv[0]);
    return 1;
  }

  char* configPath = "../configs/default.json";
  if(argc == 2)
  {
    configPath = argv[1];
  }

  if(simulationInit(&sim, configPath))
  {
    return 1;
  }

  // simulationPrint(&sim);

  simulationStart(&sim);
}
