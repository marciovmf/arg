#ifndef ARG_H
#define ARG_H

#include <stdbool.h>
#include <wchar.h>

typedef enum
{
  INTEGER   = 1,
  FLOAT     = 1 << 1,
  BOOL      = 1 << 2,
  STRING    = 1 << 3,
  ANY       = INTEGER | FLOAT | BOOL | STRING,
  REMINDER,
} ARGType;

typedef enum
{
  LAYER_0 = 1 << 0,
  LAYER_1 = 1 << 1,
  LAYER_2 = 1 << 2,
  LAYER_3 = 1 << 3,
  LAYER_4 = 1 << 4,
  LAYER_5 = 1 << 5,
  LAYER_6 = 1 << 6,
  LAYER_7 = 1 << 7,
  LAYER_MAX = 8
} ARGLayer;

typedef struct
{
  union
  {
    int       intValue;
    float     floatValue;
    wchar_t*  stringValue;
    bool      boolValue;
  };

} ARGValue;

typedef struct
{
  wchar_t*  name;
  size_t    hash;
  int       numValues;
  ARGType   type;
  ARGValue* values;
  ARGLayer  layer;
} ARGOption;

typedef struct
{
  int         numOptions;
  bool        valid;
  ARGOption*  options;
  int         reminderArgi;
  int         argi;
  int         argc;
  wchar_t**   argv;
} ARGCmdLine;

typedef struct
{
  wchar_t*  name;
  wchar_t*  valueName;
  wchar_t*  help;
  ARGType   type;
  short     numValuesMin; // zero or negative means no arguments
  short     numValuesMax; // zero or negative means no maximum limit
  bool      required;
  ARGLayer  layer;
} ARGExpectedOption;


ARGCmdLine argParseCmdLine(int argc, wchar_t **argv, ARGExpectedOption* expectedOptions, int numExpectedOptions);
void argShowUsage(wchar_t* programName, ARGExpectedOption* expectedOptions, unsigned int numExpectedOptions);
void argFreeCmdLine(ARGCmdLine *cmdLine);

#endif  // ARG_H
