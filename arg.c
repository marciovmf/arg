#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <wchar.h>
#include "arg.h"

static size_t arg_stringToHash(const wchar_t* str)
{
  size_t stringLen = wcslen((wchar_t*)str);
  size_t hash = 0;

  for(; *str; ++str)
  {
    hash += *str;
    hash += (hash << 10);
    hash ^= (hash >> 6);
  }

  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash += (hash << 15);
  hash ^= stringLen;

  return hash;
}

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

static ARGOption arg_parseOption(wchar_t* name, wchar_t** values, int numValues)
{
  ARGOption cmdOption;
  cmdOption.name = name;
  cmdOption.numValues = numValues;
  cmdOption.hash = arg_stringToHash(name);

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

ARGOption* arg_findOption(ARGCmdLine* cmdLine, wchar_t* name, size_t hash)
{
  for (int i = 0; i < cmdLine->numOptions; i++)
  {
    ARGOption* option = &cmdLine->options[i];
    if (option->hash == hash && wcscmp(option->name, name) == 0)
      return option;
  }

  return (ARGOption*) 0;
}

bool arg_isExpectedOption(ARGOption* option, ARGExpectedOption* expectedOptions, unsigned int numExpectedOptions)
{
  for (int i = 0; i < numExpectedOptions; i++)
  {
    ARGExpectedOption* expected = &expectedOptions[i];
    if (option->hash == expected->hash && wcscmp(option->name, expected->name + 1) == 0) // +1 to skip the dash
      return true;
  }

  return false;
}

void arg_showUsage(wchar_t* programName, ARGExpectedOption* expectedOptions, unsigned int numExpectedOptions)
{
  fwprintf(stdout, L"Usage\n\n  %s ", programName);

  for( int i = 0; i < numExpectedOptions; i++)
  {
    ARGExpectedOption* option = &expectedOptions[i];
    wchar_t* fmt = option->required ? L"%s %s " : L"[%s %s] ";
    fwprintf(stdout, fmt, option->name, option->numValuesMin > 0 ? option->valueName : L"");
  }

  fwprintf(stdout, L"\n\n");
  for(int i = 0; i < numExpectedOptions; i++)
  {
    ARGExpectedOption* option = &expectedOptions[i];
    int numSpaces = -30 + wcslen(option->name); // + wcslen(option->valueName) + 40;
    wprintf(L"  %s %*s = %s\n", option->name, numSpaces, option->valueName, option->help);
  }
}

ARGCmdLine* argCmdLineParse(int argc, wchar_t** argv)
{
  ARGCmdLine* cmdLine = (ARGCmdLine*) malloc(sizeof(ARGCmdLine));
  cmdLine->numOptions = 0;
  cmdLine->options = (ARGOption*) malloc((argc - 1)* sizeof(ARGOption));
  cmdLine->programName = argv[0];

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

bool argValidate(ARGCmdLine* cmdLine, ARGExpectedOption* expectedOptions, unsigned int numExpectedOptions)
{
  bool result = true;

  // compute hashes for each expected option
  for (int i = 0; i < numExpectedOptions; i++)
  {
    ARGExpectedOption* expected = &expectedOptions[i];
    expected->hash = arg_stringToHash(expected->name + 1); // +1 to skip the dash
  }


  // Handle the default HELP cmd line
  if (cmdLine->numOptions == 1 && cmdLine->options[0].numValues == 0)
  {
    for (int i = 0; i < numExpectedOptions; i++)
    {
      ARGExpectedOption* helpOption = &expectedOptions[i];
      if (helpOption->type == HELP
          && helpOption->numValuesMax == 0
          && helpOption->numValuesMin == 0
          && helpOption->hash == cmdLine->options[0].hash
          && wcscmp(helpOption->name, cmdLine->options[i].name) == 0)
      {
        arg_showUsage(cmdLine->programName, expectedOptions, numExpectedOptions);
        return true;
      }
    }
  }

  // check if command line matches the expected arguments
  for (int i = 0; i < numExpectedOptions; i++)
  {
    ARGExpectedOption* expected = &expectedOptions[i];
    ARGOption* option = arg_findOption(cmdLine, expected->name + 1, expected->hash); // +1 to skip the dash

    // validate required options
    if (option == 0)
    {
      if (expected->required)
      {
        fwprintf(stderr, L"Option '%s' is required but was not specified.\n", expected->name);
        result = false;
        continue;
      }
    }
    else  // validate number of values
    {
      // exact amount of values
      if (expected->numValuesMin == expected->numValuesMax)
      {
        if (expected->numValuesMin <= 0 && option->numValues > 0)
        {
          fwprintf(stderr, L"Option '%s' does not expect values.\n", expected->name);
          result = false;
        }
      }
      else
      {
        if (expected->numValuesMin > option->numValues || expected->numValuesMax < option->numValues)
        {
          fwprintf(stderr, L"Option '%s' expects from %d to %d values but %d was provided.\n", expected->name,
              expected->numValuesMin, expected->numValuesMax, option->numValues);
          result = false;
        }
      }

      // validate type of values
      for (int j = 0 ; j < option->numValues; j++)
      {
        ARGValue* value = &option->values[j];
        if ((value->type & expected->type) != value->type)
        {
          fwprintf(stderr, L"Option '%s' received incompatible value type.\n", expected->name);
          result = false;
          break;
        }
      }
    }
  }

  // check for unknown options
  for (int i = 0; i < cmdLine->numOptions; i++)
  {
    ARGOption* option = &cmdLine->options[i];
    if (! arg_isExpectedOption(option, expectedOptions, numExpectedOptions))
    {
      fwprintf(stderr, L"Unknown option '-%s'\n", option->name);
      result = false;
    }
  }

  return result;
}

