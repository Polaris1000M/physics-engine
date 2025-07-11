#include "simulation.h"

int main() {
  Simulation sim;
  simulationInit(&sim, 5);
  simulationStart(&sim);
}
