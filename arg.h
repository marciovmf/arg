#ifndef ARG_H
#define ARG_H

#include <stdbool.h>
#include <wchar.h>

typedef enum
{
  ARG_TYPE_INTEGER   = 1,
  ARG_TYPE_FLOAT     = 1 << 1,
  ARG_TYPE_BOOL      = 1 << 2,
  ARG_TYPE_STRING    = 1 << 3,
  ARG_TYPE_ANY       = ARG_TYPE_INTEGER | ARG_TYPE_FLOAT | ARG_TYPE_BOOL | ARG_TYPE_STRING,
  ARG_TYPE_REMINDER,
} ARGType;

typedef enum
{
  ARG_FLAG_ONCE       = 0,
  ARG_FLAG_MULTIPLE   = 1,
} ARGFlag;

typedef enum
{
  ARG_LAYER_0 = 1 << 0,
  ARG_LAYER_1 = 1 << 1,
  ARG_LAYER_2 = 1 << 2,
  ARG_LAYER_3 = 1 << 3,
  ARG_LAYER_4 = 1 << 4,
  ARG_LAYER_5 = 1 << 5,
  ARG_LAYER_6 = 1 << 6,
  ARG_LAYER_7 = 1 << 7,
  ARG_LAYER_MAX = 8
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
  ARGValue* values;
  int       numValues;
  ARGType   type;
  ARGLayer  layer;
  int       id;
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
  int         numReminders;
  int         numRequiredOptions;
  ARGLayer    baseLayer;
} ARGCmdLine;

typedef struct
{
  int       id;           // a numeric ID to identify the option
  wchar_t*  name;
  wchar_t*  valueName;
  wchar_t*  help;
  ARGType   type;
  short     numValuesMin; // zero or negative means no arguments
  short     numValuesMax; // zero or negative means no maximum limit
  bool      required;
  ARGLayer  layer;
  ARGFlag   flag;
} ARGExpectedOption;


ARGCmdLine argParseCmdLine(int argc, wchar_t **argv, ARGExpectedOption* expectedOptions, int numExpectedOptions);
void argShowUsage(wchar_t* programName, ARGExpectedOption* expectedOptions, unsigned int numExpectedOptions);
ARGOption* argGetOptionByName(ARGCmdLine* cmdLine, wchar_t* name);
ARGOption* argGetOptionById(ARGCmdLine* cmdLine, int id);
void argFreeCmdLine(ARGCmdLine *cmdLine);

#endif  // ARG_H
