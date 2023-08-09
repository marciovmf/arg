#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <wchar.h>
#include "arg.h"

ARGOption* argGetOptionByName(ARGCmdLine*, wchar_t*);
ARGOption* argGetOptionById(ARGCmdLine*, int);

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
    case ARG_TYPE_INTEGER: return L"INTEGER"; break;
    case ARG_TYPE_FLOAT: return L"FLOAT"; break;
    case ARG_TYPE_BOOL: return L"BOOL"; break;
    case ARG_TYPE_STRING: return L"STRING"; break;
    case ARG_TYPE_ANY: return L"ANY"; break;
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
    type = ARG_TYPE_INTEGER;
    parsed = true;
  }

  // check for FLOAT
  if (! parsed)
  {
    double number = wcstod(valueString, &endptr);
    if (valueString != endptr && *endptr == L'\0')
    {
      value.floatValue = number;
      type = ARG_TYPE_FLOAT;
      parsed = true;
    }
  }

  // chek for BOOL
  if (! parsed)
  {
    if (wcscmp(valueString, L"true") == 0)
    {
      type = ARG_TYPE_BOOL;
      value.boolValue = true;
      parsed = true;
    }
    else if (wcscmp(valueString, L"false") == 0)
    {
      type = ARG_TYPE_BOOL;
      value.boolValue = false;
      parsed = true;
    }
  }

  // check for STRING  (fallback case)
  if (! parsed)
  {
    value.stringValue = valueString;
    type = ARG_TYPE_STRING;
    parsed = true;
  }

  // check for type mismatch
  if (expected->type != type)
  {
    fwprintf(stderr, L"Option '%s' expects %s values but %s was passed\n",
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
    cmdLine->numReminders = cmdLine->argc - cmdLine->argi;
    cmdLine->argi = cmdLine->argc;
    return;
  }

  // Validate flags
  ARGOption* option = argGetOptionById(cmdLine, expected->id);
  if (option)
  {
    if (expected->flag == ARG_FLAG_ONCE)
    {
      fwprintf(stderr, L"Option '%s' can't be specified multiple times.\n", optionString);
      cmdLine->valid = false;
      return;
    }
  }
  else
  {
    int optionIndex = cmdLine->numOptions++;
    cmdLine->options = (ARGOption*) realloc(cmdLine->options, sizeof(ARGOption) * cmdLine->numOptions);
    option = &cmdLine->options[optionIndex];
    option->name      = optionString;
    option->values    = 0;
    option->numValues = 0;
    option->type      = expected->type;
    option->layer     = expected->layer;
    option->id        = expected->id;

    if (expected->required)
    {
      cmdLine->numRequiredOptions++;
      cmdLine->baseLayer = expected->layer;
    }
  }

  cmdLine->argi++;

  // minimum argument count
  while (option->numValues < expected->numValuesMin && cmdLine->valid)
  {
    bool noMoreArgs = (cmdLine->argi >= cmdLine->argc);
    if (noMoreArgs || ! argParseValue(cmdLine, option, expected))
    {
      fwprintf(stderr, L"Option '%s' requries at least %d %s arguments.\n", option->name, expected->numValuesMin, argGetTypeName(option->type));
      cmdLine->valid = false;
      return;
    }
  }

  // maximum argument count
  if (expected->numValuesMin != expected->numValuesMax)
  {
    while (cmdLine->argi <= (cmdLine->argc - 1))
    {
      // if there is a maximum value limit, stop parsing values whe whe reach this limit
      if (expected->numValuesMax > 0 && option->numValues == expected->numValuesMax)
        break;

      // if we are parsing an option that takes INTEGER of FLOAT, it is possible to get a negative number.
      // In this case a - might not indicate a new parameter so we have to try parsing it.
      // because of that we need to check if the current argv matches any ARGExpectedOption
      if (cmdLine->argv[cmdLine->argi][0] == '-' && expected->type == ARG_TYPE_INTEGER || expected->type == ARG_TYPE_FLOAT) {
        if (argFindExpectedOption(cmdLine->argv[cmdLine->argi], expectedOptions, numExpectedOptions))
          break;
      }

      if (! argParseValue(cmdLine, option, expected))
        break;
    }
  }

  // if the option is set not to have values, we might need to initialize it's value to a default
  if (option->numValues == 0)
  {
    option->numValues = 1;
    option->values = (ARGValue*) malloc(sizeof(ARGValue));
    if(option->type == ARG_TYPE_INTEGER)
      option->values[0].intValue = 0;
    if(option->type == ARG_TYPE_FLOAT)
      option->values[0].floatValue = 0.0f;
    if(option->type == ARG_TYPE_BOOL)
      option->values[0].boolValue = true;
    if(option->type == ARG_TYPE_STRING)
      option->values[0].stringValue = L"";
  }

  cmdLine->reminderArgi = cmdLine->argi;

  return;
}

ARGCmdLine argParseCmdLine(int argc, wchar_t **argv, ARGExpectedOption* expectedOptions, int numExpectedOptions)
{
  ARGCmdLine cmdLine    = { 0 };
  cmdLine.valid         = true;
  cmdLine.numOptions    = 0;
  cmdLine.argc          = argc;
  cmdLine.argv          = argv;
  cmdLine.argi          = 1;      // we skip program name
  cmdLine.reminderArgi  = argc;
  cmdLine.numReminders  = 0;

  ARGLayer usedLayer = 0;

  do
  {
    // Parse the option and it's arguments
    argParseOption(&cmdLine, expectedOptions, numExpectedOptions);

    // check if the last parsed option layer conflicts with previous options layers
    if (cmdLine.numOptions > 0 && cmdLine.valid)
    {
      ARGOption* lastParsedOption = &cmdLine.options[cmdLine.numOptions - 1];
      ARGLayer cmdLayer = lastParsedOption->layer;

      if (!usedLayer)
      {
        usedLayer = cmdLayer;
      }
      else
      {
        if ((cmdLine.numRequiredOptions > 0 && (cmdLine.baseLayer & cmdLayer) == 0) || (usedLayer & cmdLayer) == 0)
        {
          for (int j = 0; j < cmdLine.numOptions - 1; j++)
          {
            if (cmdLine.options[j].layer <= usedLayer)
            {
              fwprintf(stderr, L"Option '%s' can not be used along with '%s'\n",
                  lastParsedOption->name, cmdLine.options[j].name);
            }
          }

          cmdLine.valid = false;
          break;
        }

        if (cmdLayer < usedLayer)
          usedLayer = cmdLayer;

      }
    }
  } while (cmdLine.argi < cmdLine.argc && cmdLine.valid);

  // check for missing required options for the layer used
  // But, if multiple layers were used and no required option was passed, just
  // ignore because we can not know which layer required options to require.
  if (cmdLine.valid && cmdLine.numOptions > 0 && cmdLine.numRequiredOptions > 0)
  {
    for (int i = 0; i < cmdLine.numOptions; i++)
    {
      if (expectedOptions[i].required)
      {
        if ((expectedOptions[i].layer & cmdLine.baseLayer) == 0)
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

        if (! found)
        {
          fwprintf(stderr, L"Option '%s' is required but was not provided.\n", expectedOptions[i].name);
          cmdLine.valid = false;
          break;
        }
      }

    }
  }


  return cmdLine;
}

ARGOption* argGetOptionByName(ARGCmdLine* cmdLine, wchar_t* name)
{
  for (int i = 0; i < cmdLine->numOptions; i++)
  {
    if (wcscmp(cmdLine->options[i].name, name) == 0)
    {
      return &cmdLine->options[i];
    }
  }
  return 0;
}

ARGOption* argGetOptionById(ARGCmdLine* cmdLine, int id)
{
  for (int i = 0; i < cmdLine->numOptions; i++)
  {
    if (cmdLine->options[i].id == id)
    {
      return &cmdLine->options[i];
    }
  }
  return 0;
}

void argShowUsage(wchar_t* programName, ARGExpectedOption* expectedOptions, unsigned int numExpectedOptions)
{
  fwprintf(stdout, L"Usage\n\n");

  // Options if any
  for (int layer = 0; layer < ARG_LAYER_MAX; layer++)
  {
    bool printedProgramName = false;
    for(int i = 0; i < numExpectedOptions; i++)
    {
      ARGExpectedOption* option = &expectedOptions[i];

      // is this option in this layer ?
      if ((option->layer & (1 << layer)) == 0)
        continue;

      if (option->type == ARG_TYPE_REMINDER)
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
    for (int i = 0; i < numExpectedOptions; i++)
    {
      ARGExpectedOption* option = &expectedOptions[i];

      if (option->type != ARG_TYPE_REMINDER)
        continue;

      // is this option in this layer ?
      if ((option->layer & (1 << layer)) == 0)
        continue;

      wchar_t* fmt = option->required ? L"<%s>" : L"[<%s>] ";
      fwprintf(stdout, fmt, option->name);
    }

    if (printedProgramName)
      fwprintf(stdout, L"\n");
  }

  fwprintf(stdout, L"\n\nOptions\n");
  for (int i = 0; i < numExpectedOptions; i++)
  {
    ARGExpectedOption* option = &expectedOptions[i];
    int numSpaces = -30 + wcslen(option->name);
    wprintf(L"  %s %*s = %s\n", option->name, numSpaces, option->valueName, option->help);
  }
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

