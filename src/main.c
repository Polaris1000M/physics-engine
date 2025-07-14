#include "simulation.h"
#include <stdlib.h>

int main(int argc, char* argv[])
{
  Simulation sim;

  if(argc > 2) {
    printf("USAGE: %s <config_path>", argv[0]);
    return 1;
  }
  else if(argc == 2) {
    simulationInit(&sim, argv[1]);
  }
  else {
    simulationInit(&sim, "../configs/default.json");
  }

  // simulationStart(&sim);
}
