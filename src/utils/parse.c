#include "parse.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>

int parseConfigObject(cJSON* object, ConfigObject* configObject)
{
  srand(time(NULL));

  // parse type
  const cJSON* configType = cJSON_GetObjectItemCaseSensitive(object, "type");
  const char* typeErrorMessage = "ERROR::CONFIG::INVALID_TYPE: expected \"sphere\", \"cube\", or \"pyramid\" for type of object\n";
  if(!cJSON_IsString(configType))
  {
    printf("%s", typeErrorMessage);
    return 1;
  }

  int match = 0;
  for(int i = 0; i < OBJECT_TYPES; i++)
  {
    if(!strcmp(configType->valuestring, OBJECT_NAMES[i]))
    {
      match = 1;
      break;
    }
  }
  if(!match)
  {
    printf("%s", typeErrorMessage);
    return 1;
  }

  // parse size
  const cJSON* configSize = cJSON_GetObjectItemCaseSensitive(object, "size");
  if(!cJSON_IsNumber(configSize)) {
    printf("ERROR::CONFIG::INVALID_SIZE: expected float for size of object\n");
    return 1;
  }

  // parse mass
  const cJSON* configMass = cJSON_GetObjectItemCaseSensitive(object, "mass");
  if(!cJSON_IsNumber(configMass)) {
    printf("ERROR::CONFIG::INVALID_MASS: expected float for mass of object\n");
  }

  // parse distribution
  const cJSON* configDistribution = cJSON_GetObjectItemCaseSensitive(object, "distribution");
  if(!cJSON_IsString(configDistribution) || (strcmp(configDistribution->valuestring, "uniform") && strcmp(configDistribution->valuestring, "random")))
  {
    printf("ERROR::CONFIG::INVALID_DISTRIBUTION: expected \"random\" or \"uniform\" for distribution of object\n");
    return 1;
  }

  // parse range
  const cJSON* configRange = cJSON_GetObjectItemCaseSensitive(object, "range");
  const char* rangeErrorMessage = "ERROR::CONFIG::INVALID_POSITION: expected float array for position range of object with format [[<low_x>, <high_x>], [<low_y>, <high_y>], [<low_z>, <high_z>]]\n";
  if(!cJSON_IsArray(configRange))
  {
    printf("%s", rangeErrorMessage);
    return 1;
  }
  float range[3][2];
  unsigned int rangeSize = cJSON_GetArraySize(configRange);
  if(rangeSize != 3)
  {
    printf("%s", rangeErrorMessage);
    return 1;
  }

  for(int i = 0; i < 3; i++)
  {
    cJSON* coordRange = cJSON_GetArrayItem(configRange, i);
    if(!cJSON_IsArray(coordRange))
    {
      printf("%s", rangeErrorMessage);
      return 1;
    }

    unsigned int coordRangeSize = cJSON_GetArraySize(coordRange);
    if(coordRangeSize != 2)
    {
      printf("%s", rangeErrorMessage);
      return 1;
    }

    cJSON* lower = cJSON_GetArrayItem(coordRange, 0);
    cJSON* upper = cJSON_GetArrayItem(coordRange, 1);

    if(!cJSON_IsNumber(lower) || !cJSON_IsNumber(upper))
    {
      printf("%s", rangeErrorMessage);
      return 1;
    }

    range[i][0] = lower->valuedouble;
    range[i][1] = upper->valuedouble;
  }

  // parse color
  cJSON* configColor = cJSON_GetObjectItemCaseSensitive(object, "color");
  const char* colorErrorMessage = "ERROR::CONFIG::INVALID_COLOR: expected float array for color of object with format [<red_color>, <blue_color>, <green_color>] where each value is from 0 to 1\n";
  if(!cJSON_IsArray(configColor) || cJSON_GetArraySize(configColor) != 3)
  {
    printf("%s", colorErrorMessage);
    return 1;
  }
  vec3 color;
  for(int i = 0; i < 3; i++)
  {
    cJSON* curColor = cJSON_GetArrayItem(configColor, i);

    if(!cJSON_IsNumber(curColor) || curColor->valuedouble > 1.0 || curColor->valuedouble < 0.0)
    {
      printf("%s", colorErrorMessage);
      return 1;
    }

    color[i] = curColor->valuedouble;
  }

  // parse count
  const cJSON* configCount = cJSON_GetObjectItemCaseSensitive(object, "count");
  if(!cJSON_IsNumber(configCount) || configCount->valueint <= 0)
  {
    printf("ERROR::CONFIG::INVALID_COUNT: expected positive integer for count of objects\n");
    return 1;
  }

  // only populate config object after all fields have been verified

  // populate type
  for(int i = 0; i < 3; i++)
  {
    if(!strcmp(configType->valuestring, OBJECT_NAMES[i]))
    {
      configObject->type = i;
      break;
    }
  }

  // populate size
  configObject->size = configSize->valuedouble;

  // populate mass
  configObject->mass = configMass->valuedouble;

  // populate distribution
  configObject->distribution = configDistribution->valuestring;

  // populate range
  memcpy(configObject->range, range, sizeof(configObject->range));

  // populate color
  glm_vec3_copy(color, configObject->color);

  // populate count
  configObject->count = configCount->valueint;

  return 0;
}

ConfigObject* parseConfigObjects(cJSON* objects)
{
  unsigned int numObjects = cJSON_GetArraySize(objects);
  const char* objectsErrorMessage = "ERROR::CONFIG::INVALID_OBJECTS: expected non-empty JSON array of objects\n"; 
  if(numObjects == 0)
  {
    printf("%s", objectsErrorMessage);
    return NULL;
  }

  ConfigObject* configObjects = malloc(numObjects * sizeof(ConfigObject));
  for(int i = 0; i < numObjects; i++)
  {
    cJSON* object = cJSON_GetArrayItem(objects, i);
    if(parseConfigObject(object, configObjects + i))
    {
      return NULL;
    }
  }

  return configObjects;
}

char* parseFile(const char* filePath, const char* errorMessage)
{
  FILE* fp;
  char* buffer;
  
  fp = fopen(filePath, "rb");
  if(!fp)
  {
    printf("ERROR::%s::FILE_NOT_SUCCESSFULLY_READ: %s\n", errorMessage, filePath);
    buffer = 0; 
    return buffer;
  }

  // find length of file
  fseek(fp, 0L, SEEK_END);
  long shaderSize = ftell(fp);
  rewind(fp);

  buffer = calloc(1, shaderSize + 1);

  if(!buffer)
  {
    printf("ERROR::%s::FAILED_TO_ALLOCATE_BUFFER: %s\n", errorMessage, filePath);
    buffer = 0;
    return buffer;
  }

  if(fread(buffer, shaderSize, 1, fp) != 1)
  {
    printf("ERROR::%s::FILE_NOT_SUCCESSFULLY_READ: %s\n", errorMessage, filePath);
    buffer = 0; 
    return buffer;
  }

  return buffer;
}

void convertConfigObjectToObjects(ConfigObject* configObject, Object* objects)
{
  // generate a new object for every value in count
  for(unsigned int i = 0; i < configObject->count; i++)
  {
    vec3 position;
    if(strcmp(configObject->distribution, "random") == 0)
    {
      for(unsigned int j = 0; j < 3; j++)
      {
        position[j] = configObject->range[j][0] + ((float) rand() / (float) RAND_MAX) * (configObject->range[j][1] - configObject->range[j][0]);
      }
    }
    else
    {
      for(unsigned int j = 0; j < 3; j++)
      {
        // position is in the middle of the range when only a single object
        if(configObject->count == 1)
        {
          position[j] = (configObject->range[j][0] + configObject->range[j][1]) / 2.0f;
        }
        else
        {
          position[j] = (configObject->range[j][1] - configObject->range[j][0]) / ((float) configObject->count - 1.0f) * (float) i + configObject->range[j][0];
        }
      }
    }

    objectInit(objects + i, configObject->type, configObject->size, configObject->mass, position, configObject->color);
  }
}

void convertConfigObjectsToObjects(int numConfigObjects, ConfigObject* configObjects, unsigned int* objectCounts, Object** objects)
{
  memset(objectCounts, 0, OBJECT_TYPES * sizeof(unsigned int));

  for(int i = 0; i < numConfigObjects; i++)
  {
    objectCounts[configObjects[i].type] += configObjects[i].count;
  }

  for(int i = 0; i < OBJECT_TYPES; i++)
  {
    objects[i] = malloc(objectCounts[i] * sizeof(Object));
  }

  unsigned int indices[] = {0, 0, 0}; // next indices to write the new objects
  for(int i = 0; i < numConfigObjects; i++)
  {
    ObjectType type = configObjects[i].type;
    convertConfigObjectToObjects(configObjects + i, objects[type] + indices[type]);
    indices[type] += configObjects[i].count;
  }
}
