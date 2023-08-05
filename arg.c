#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "arg.h"

static ARGValue arg_parseOptionValue(wchar_t* value)
{
  ARGValue optionValue;

  if (wcscmp(value, L"true") == 0 || wcscmp(value, L"false") == 0)
  {
    optionValue.type = BOOL;
    optionValue.value.boolValue = (wcscmp(value, L"true") == 0);
  }
  else
  {
    wchar_t* endptr;
    int intValue = wcstol(value, &endptr, 10);
    if (*endptr == '\0')
    {
      optionValue.type = INTEGER;
      optionValue.value.intValue = intValue;
    }
    else
    {
      float floatValue = wcstof(value, &endptr);
      if (*endptr == '\0')
      {
        optionValue.type = FLOAT;
        optionValue.value.floatValue = floatValue;
      } else
      {
        optionValue.type = STRING;
        optionValue.value.stringValue = _wcsdup(value);
      }
    }
  }

  return optionValue;
}

static ARGOption arg_parseOption(wchar_t* option, wchar_t** values, int numValues)
{
  ARGOption cmdOption;
  cmdOption.option = option;
  cmdOption.numValues = numValues;

  if (values == NULL)
  {
    cmdOption.numValues = 0;
    cmdOption.values = NULL;
    return cmdOption;
  }

  cmdOption.values = (ARGValue*)malloc(numValues * sizeof(ARGValue));

  for (int i = 0; i < numValues; i++)
  {
    cmdOption.values[i] = arg_parseOptionValue(values[i]);
  }

  return cmdOption;
}

void argCmdLineFree(ARGCmdLine* cmdLine)
{
  int numOptions = cmdLine->numOptions;
  for (int i = 0; i < numOptions; i++)
  {
    for (int j = 0; j < cmdLine->options[i].numValues; j++)
    {
      if (cmdLine->options[i].values[j].type == STRING)
      {
        free(cmdLine->options[i].values[j].value.stringValue);
      }
    }
    free(cmdLine->options[i].values);
  }

  free(cmdLine->options);
  free(cmdLine);
}

ARGCmdLine* argCmdLineParse(int argc, wchar_t** argv)
{
  ARGCmdLine* cmdLine = (ARGCmdLine*) malloc(sizeof(ARGCmdLine));
  cmdLine->numOptions = 0;
  cmdLine->options = (ARGOption*) malloc((argc - 1)* sizeof(ARGOption));

  // First loop to count the number of values for each option
  for (int i = 1; i < argc; i++)
  {
    if (argv[i][0] == '-')
    {
      wchar_t* option = argv[i] + 1;
      int numValues = 0;
      for (int j = i + 1; j < argc && argv[j][0] != '-'; j++)
      {
        numValues++;
      }
      ARGOption cmdOption = arg_parseOption(option, &argv[i + 1], numValues);
      cmdLine->options[cmdLine->numOptions++] = cmdOption;
      i += numValues; // Skip the next arguments, as they were processed as values for the current option
    }
  }
  return cmdLine;
}
