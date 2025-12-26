/*
 * parse.h
 *
 * Helper methods for parsing JSON config file into simulation struct
 */

#ifndef PARSE_H
#define PARSE_H

#include <cglm/cglm.h>

#include "../simulation.h"

// reads JSON config and populates simulation data
unsigned int parseConfig(Simulation* sim, const char* configPath);

// reads file into C-string
char* parseFile(const char* filePath, const char* errorMessage);

#endif

