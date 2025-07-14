#ifndef PARSE_H
#define PARSE_H

#include "cJSON.h"
#include <cglm/cglm.h>
#include "../physics/object.h"

typedef struct ConfigObject {
  ObjectType type;
  float size;
  float mass;
  char* distribution;
  float range[3][2];
  vec3 color;
  unsigned int count;
} ConfigObject;

// parses a single JSON into a ConfigObject
int parseConfigObject(cJSON* object, ConfigObject* configObject);

// parses cJSON array into array of ConfigObjects
ConfigObject* parseConfigObjects(cJSON* objects);

// reads file into C-string
char* parseFile(const char* filePath, const char* errorMessage);

// converts a ConfigObject type into 1 or more corresponding objects
void convertConfigObjectToObjects(ConfigObject* configObject, Object* objects);

// converts array of ConfigObjects into corresponding objects, stores objects into array of different object types
void convertConfigObjectsToObjects(int numConfigObjects, ConfigObject* configObjects, unsigned int* objectCounts, Object** objects);

#endif
