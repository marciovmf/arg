#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <wchar.h>
#include "arg.h"

static ARGExpectedOption* argFindExpectedOption(wchar_t* name, ARGExpectedOption* options, int numOptions)
{
  for (int i = 0; i < numOptions; i++)
  {
    if (wcscmp(name, options[i].name) == 0)
    {
      return &options[i];
    }
  }

  return (ARGExpectedOption*) 0;
}

static wchar_t* argGetTypeName(ARGType type)
{
  switch (type)
  {
    case INTEGER: return L"INTEGER"; break;
    case FLOAT: return L"FLOAT"; break;
    case BOOL: return L"BOOL"; break;
    case STRING: return L"STRING"; break;
    case ANY: return L"ANY"; break;
    default: return L"UNKNOWN"; break;
  }
}

static bool argParseValue(ARGCmdLine* cmdLine, ARGOption* option, ARGExpectedOption* expected)
{
  if (cmdLine->argi >= cmdLine->argc)
    return false;

  ARGValue value;
  wchar_t *endptr;
  wchar_t *valueString = cmdLine->argv[cmdLine->argi];
  ARGType type;
  bool parsed = false;

  // check for INTGEGER
  int number = wcstol(valueString, &endptr, 10);
  if (valueString != endptr && *endptr == L'\0')
  {
    value.intValue = number;
    type = INTEGER;
    parsed = true;
  }

  // check for FLOAT
  if (! parsed)
  {
    double number = wcstod(valueString, &endptr);
    if (valueString != endptr && *endptr == L'\0')
    {
      value.floatValue = number;
      type = FLOAT;
      parsed = true;
    }
  }

  // chek for BOOL
  if (! parsed)
  {
    if (wcscmp(valueString, L"true") == 0)
    {
      type = BOOL;
      value.boolValue = true;
      parsed = true;
    }
    else if (wcscmp(valueString, L"false") == 0)
    {
      type = BOOL;
      value.boolValue = false;
      parsed = true;
    }
  }

  // check for STRING  (fallback case)
  if (! parsed)
  {
    value.stringValue = valueString;
    type = STRING;
    parsed = true;
  }

  // check for type mismatch
  if ((expected->type & type) != type)
  {
    fwprintf(stderr, L"Option '%s' expects %s values but %s was passed",
        option->name,
        argGetTypeName(expected->type),
        argGetTypeName(type));

    cmdLine->valid = false;
    return false;
  }

  int index = option->numValues++;
  option->values = (ARGValue*) realloc(option->values, sizeof(ARGValue) * option->numValues);
  option->values[index] = value;
  cmdLine->argi++;
  return true;
}

static void argParseOption(ARGCmdLine* cmdLine, ARGExpectedOption* expectedOptions, int numExpectedOptions)
{
  if (cmdLine->argi >= cmdLine->argc)
    return;

  wchar_t* optionString = cmdLine->argv[cmdLine->argi];
  ARGExpectedOption* expected = argFindExpectedOption(optionString, expectedOptions, numExpectedOptions);

  if (! expected)
  {
    if (optionString[0] == '-')
    {
      fwprintf(stderr, L"Unknown option '%s'\n", optionString);
      cmdLine->valid = false;
      return;
    }

    // we force the end of the parsing. 
    cmdLine->reminderArgi = cmdLine->argi;
    cmdLine->argi = cmdLine->argc;
    return;
  }

  int optionIndex = cmdLine->numOptions++;
  cmdLine->options = (ARGOption*) realloc(cmdLine->options, sizeof(ARGOption) * cmdLine->numOptions);
  ARGOption* option = &cmdLine->options[optionIndex];
  option->name = optionString;
  option->numValues = 0;
  option->values = 0;
  option->hash = 0;
  option->type = expected->type;
  option->layer = expected->layer;
  cmdLine->argi++;

  // minimum argument count
  while (option->numValues < expected->numValuesMin)
  {
    bool noMoreArgs = (cmdLine->argi >= cmdLine->argc);
    if (noMoreArgs || ! argParseValue(cmdLine, option, expected))
    {
      fwprintf(stderr, L"Option '%s' requries at least %d arguments.\n", option->name, expected->numValuesMin);
      cmdLine->valid = false;
      return;
    }
  }

  // maximum argument count
  if (expected->numValuesMin != expected->numValuesMax)
  {
    while (cmdLine->argi <= (cmdLine->argc - 1) && cmdLine->argv[cmdLine->argi][0] != '-')
    {
      // if there is a maximum value limit, stop parsing values whe whe reach this limit
      if (expected->numValuesMax > 0 && option->numValues == expected->numValuesMax)
        break;

      if (! argParseValue(cmdLine, option, expected))
        break;
    }
  }

  // if the option is set not to have values, we might need to initialize it's value to a default
  if (option->numValues == 0)
  {
    option->numValues = 1;
    option->values = (ARGValue*) malloc(sizeof(ARGValue));
    if(option->type == INTEGER)
      option->values[0].intValue = 0;
    if(option->type == FLOAT)
      option->values[0].floatValue = 0.0f;
    if(option->type == BOOL)
      option->values[0].boolValue = true;
    if(option->type == STRING)
      option->values[0].stringValue = L"";
  }

  cmdLine->reminderArgi = cmdLine->argi;

  return;
}

ARGCmdLine argParseCmdLine(int argc, wchar_t **argv, ARGExpectedOption* expectedOptions, int numExpectedOptions)
{
  ARGCmdLine cmdLine  = { 0 };
  cmdLine.valid       = true;
  cmdLine.numOptions  = 0;
  cmdLine.reminderArgi = argc;
  cmdLine.argc = argc;
  cmdLine.argv = argv;
  cmdLine.argi = 1; // we skip program name
  ARGLayer usedLayer = -1;

  do
  {
    // Parse the option and it's arguments
    argParseOption(&cmdLine, expectedOptions, numExpectedOptions);

    // check if the layer of the last option conflicts with previous layers
    if (cmdLine.numOptions > 0 && cmdLine.valid)
    {
      ARGLayer cmdLayer = cmdLine.options[cmdLine.numOptions - 1].layer;
      if (usedLayer == -1)
        usedLayer = cmdLayer;
      else
      {
        if ((usedLayer & cmdLayer) != cmdLayer)
        {
          for (int j = 0; j < cmdLine.numOptions - 1; j++)
            fwprintf(stderr, L"Option '%s' can not be used along with '%s'\n",
                cmdLine.options[cmdLine.numOptions - 1].name, cmdLine.options[j].name);

          cmdLine.valid = false;
          break;
        }
        usedLayer |= cmdLayer;
      }
    }

    // check for missing required options for the layer used
    if (cmdLine.valid && cmdLine.numOptions > 0)
    {
      for (int i = 0; i < numExpectedOptions; i++)
      {
        if (expectedOptions[i].required)
        {
          if ((expectedOptions[i].layer & usedLayer) != expectedOptions[i].layer)
            continue;

          bool found = false;
          for (int j = 0; j < cmdLine.numOptions; j++)
          {
            if (wcscmp(expectedOptions[i].name, cmdLine.options[j].name) == 0)
            {
              found = true;
              break;
            }
          }

          if (!found)
          {
            fwprintf(stderr, L"Option '%s' is required but was not provided.\n", expectedOptions[i].name);
          }
        }
      }
    }

  } while (cmdLine.argi < cmdLine.argc && cmdLine.valid);

  return cmdLine;
}

void argFreeCmdLine(ARGCmdLine *cmdLine)
{
  for (int i = 0; i < cmdLine->numOptions; i++)
  {
    int numValues = cmdLine->options[i].numValues;
    free(cmdLine->options[i].values);
  }

  free(cmdLine->options);
}

void argShowUsage(wchar_t* programName, ARGExpectedOption* expectedOptions, unsigned int numExpectedOptions)
{
  fwprintf(stdout, L"Usage\n\n");

  // Options if any
  for (int layer = 0; layer < LAYER_MAX; layer++)
  {
    bool printedProgramName = false;
    for(int i = 0; i < numExpectedOptions; i++)
    {
      ARGExpectedOption* option = &expectedOptions[i];

      // is this option in this layer ?
      if ((option->layer & (1 << layer)) == 0)
        continue;

      if (option->type == REMINDER)
        continue;

      if (!printedProgramName)
      {
        printedProgramName = true;
        fwprintf(stdout, L"  %s ", programName);
      }

      wchar_t* fmt = option->required ? L"%s %s " : L"[%s %s] ";
      fwprintf(stdout, fmt, option->name, option->numValuesMin > 0 ? option->valueName : L"\b");
    }

    // Reminder positional arguments if any
    for( int i = 0; i < numExpectedOptions; i++)
    {
      ARGExpectedOption* option = &expectedOptions[i];

      // is this option in this layer ?
      if ((option->layer & (1 << layer)) == 0)
        continue;

      if (option->type != REMINDER)
        continue;

      wchar_t* fmt = option->required ? L"<%s>" : L"[<%s>] ";
      fwprintf(stdout, fmt, option->name);
    }

    if (printedProgramName)
      fwprintf(stdout, L"\n");
  }

  fwprintf(stdout, L"\n\nOptions\n");
  for(int i = 0; i < numExpectedOptions; i++)
  {
    ARGExpectedOption* option = &expectedOptions[i];
    int numSpaces = -30 + wcslen(option->name);
    wprintf(L"  %s %*s = %s\n", option->name, numSpaces, option->valueName, option->help);
  }
}

