#include "simulation.h"
#include <stdlib.h>

int main(int argc, char* argv[]) {
  Simulation sim;

  if(argc > 1) {
    simulationInit(&sim, atoi(argv[1]));
  }
  else {
    simulationInit(&sim, 2);
  }
  simulationStart(&sim);
}
