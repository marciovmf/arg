#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <stdbool.h>
#include <stdio.h>
#include <wctype.h>
#include "arg.h"

int wmain(int argc, wchar_t **argv)
{
  enum
  {
    OPTION_HELP   = 0,
    OPTION_SUM    = 1,
    OPTION_DEBUG  = 3,
  };

  ARGExpectedOption options[] =
  {
    { OPTION_HELP,  L"-h", L"", L"Displays this help", ARG_TYPE_STRING, 0, 0, true, ARG_LAYER_0, ARG_FLAG_ONCE},
    { OPTION_SUM, L"-sum", L"n", L"Specify values to sum", ARG_TYPE_INTEGER, 2, -1, true, ARG_LAYER_1, ARG_FLAG_MULTIPLE},
    { OPTION_DEBUG,  L"-debug", L"", L"Display debug information about parsed options", ARG_TYPE_BOOL, 0, 0, false, ARG_LAYER_0 | ARG_LAYER_1, ARG_FLAG_ONCE},
  };

  int numOptions = sizeof(options) / sizeof(options[0]);
  ARGCmdLine cmdLine = argParseCmdLine(argc, argv, options, numOptions);

  if (!cmdLine.valid)
    return 1;

  // Help ?
  ARGOption* option = argGetOptionById(&cmdLine, OPTION_HELP);
  if (option && option->values[0].boolValue)
  {
    argShowUsage(L"cmdlineparse", options, numOptions);
  }

  // SUM ?
  option = argGetOptionById(&cmdLine, OPTION_SUM);
  if (option)
  {
    int result = 0;
    for (int i = 0; i < option->numValues; i++)
    {
      result += option->values[i].intValue;
    }

    printf("The sum is %d\n", result);
  }

  // debug ?
  option = argGetOptionById(&cmdLine, OPTION_DEBUG);
  if (option && option->values[0].boolValue)
  {
    printf("\n------------------ CMDLINE DEBUG INFO ------------------------\n");
    for (int i = 0; i < cmdLine.numOptions; i++)
    {
      ARGOption* option = &cmdLine.options[i];

      wprintf(L"%s (%s) = ", option->name,
          option->type == ARG_TYPE_INTEGER ? L"INTEGER" :
          option->type == ARG_TYPE_BOOL    ? L"BOOL" :
          option->type == ARG_TYPE_FLOAT   ? L"FLOAT" :
          L"STRING");

      for (int j = 0; j < option->numValues; j++)
      {
        wchar_t* separator = j == option->numValues - 1 ? L"\n" : L",";
        if (cmdLine.options[i].type == ARG_TYPE_FLOAT)
        {
          wprintf(L"%f%s", option->values[j].floatValue, separator);
        }
        else if (cmdLine.options[i].type == ARG_TYPE_INTEGER)
        {
          wprintf(L"%d%s", option->values[j].intValue, separator);
        }
        else if (cmdLine.options[i].type == ARG_TYPE_BOOL)
        {
          wprintf(L"%s%s", option->values[j].boolValue ? L"true" : L"false", separator);
        }
        else
        {
          wprintf(L"\"%s\"%s", option->values[j].stringValue, separator);
        }
      }
    }

    wchar_t** reminders = cmdLine.argv + cmdLine.reminderArgi;
    for (int i = 0; i < cmdLine.numReminders; i++)
    {
      wprintf(L"Reminder %d = %s\n",i, reminders[i]);
    }
  }

  argFreeCmdLine(&cmdLine);
  return 0;
}
