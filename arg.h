#ifndef ARG_H
#define ARG_H

#include <stdbool.h>
#include <wchar.h>

typedef enum 
{
  INTEGER = 1,
  FLOAT   = 1 << 1,
  BOOL    = 1 << 2,
  STRING  = 1 << 3,
  ANY     = INTEGER | FLOAT | BOOL | STRING,
  HELP
} ARGType;


typedef struct
{
  union
  {
    int intValue;
    float floatValue;
    wchar_t* stringValue;
    bool boolValue;
  } value;

  ARGType type;
} ARGValue;

typedef struct
{
  wchar_t* name;
  ARGValue* values;
  int numValues;
  size_t hash;
} ARGOption;

typedef struct
{
  int numOptions;
  ARGOption* options;
  wchar_t* programName;
} ARGCmdLine;

typedef struct
{
  wchar_t* name;
  wchar_t* valueName;
  wchar_t* help;
  short numValuesMin; // zero or negative means no arguments
  short numValuesMax; // zero or negative means no maximum limit
  ARGType type;
  bool required;
  size_t hash;
} ARGExpectedOption;

ARGCmdLine* argCmdLineParse(int argc, wchar_t** argv);
void argCmdLineFree(ARGCmdLine* cmdLine);
bool argValidate(ARGCmdLine* cmdLine, ARGExpectedOption* expectedArgs, unsigned int numExpectedArgs);

#endif  // ARG_H
