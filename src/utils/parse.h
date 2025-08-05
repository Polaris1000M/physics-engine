/*
 * parse.h
 *
 * Helper methods for parsing JSON config file into simulation struct
 */ 

#ifndef PARSE_H
#define PARSE_H

#include "cJSON.h"
#include <cglm/cglm.h>
#include "../simulation.h"
#include "../physics/object.h"

// parses a single JSON into a ConfigObject
unsigned int parseConfigObject(cJSON* configObject, Object* object);

// parses cJSON array into array of ConfigObjects
unsigned int parseConfigObjects(cJSON* configObjects, unsigned int* objectCounts, Object** objects);

unsigned int parseConfig(Simulation* sim, const char* configPath);

// reads file into C-string
char* parseFile(const char* filePath, const char* errorMessage);

#endif
