#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <stdbool.h>
#include <stdio.h>
#include <wctype.h>
#include "arg.h"

int wmain(int argc, wchar_t **argv)
{
  ARGExpectedOption optionA = { L"-a", L"number", L"Specify zero or one integer value", INTEGER, 1, 1, true, ARG_LAYER_0, ARG_ONCE };
  ARGExpectedOption optionB = { L"-b", L"", L"Specify a flag with no arguments", BOOL, 0, 0, true, ARG_LAYER_0, ARG_ONCE };
  ARGExpectedOption optionD = { L"-f", L"float-number", L"Specify a float value", FLOAT, 1, 1, false, ARG_LAYER_0, ARG_ONCE };
  ARGExpectedOption optionC = { L"-c", L"param", L"Specify a list of one or more strings",  STRING, 1, -1, false, ARG_LAYER_0, ARG_MULTIPLE};
  ARGExpectedOption optionHelp = { L"-h", L"param", L"Displays this help.", STRING, 0, 0, true, ARG_LAYER_1, ARG_ONCE };
  ARGExpectedOption optionReminder0 = { L"Input files", L"param", L"A list of files to process", REMINDER, 0, 0, false, ARG_LAYER_0, ARG_ONCE};

  ARGExpectedOption options[] = { optionA, optionB, optionC, optionD, optionHelp, optionReminder0};
  int numOptions = sizeof(options) / sizeof(options[0]);
  ARGCmdLine cmdLine = argParseCmdLine(argc, argv, options, numOptions);

  if (cmdLine.valid)
  {
    if (cmdLine.numOptions > 0 && wcscmp(cmdLine.options[0].name, L"-h") == 0)
    {
      argShowUsage(L"cmdlineparse", options, numOptions);
      return 0;
    }

    for (int i = 0; i < cmdLine.numOptions; i++)
    {
      ARGOption* option = &cmdLine.options[i];

      wprintf(L"%s (%s) = ", option->name,
          option->type == INTEGER ? L"INTEGER" :
          option->type == BOOL    ? L"BOOL" :
          option->type == FLOAT   ? L"FLOAT" :
          L"STRING");

      for (int j = 0; j < option->numValues; j++)
      {
        wchar_t* separator = j == option->numValues - 1 ? L"\n" : L",";
        if (cmdLine.options[i].type == FLOAT)
        {
          wprintf(L"%f%s", option->values[j].floatValue, separator);
        }
        else if (cmdLine.options[i].type == INTEGER)
        {
          wprintf(L"%d%s", option->values[j].intValue, separator);
        }
        else if (cmdLine.options[i].type == BOOL)
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
  else
  {
    printf("invalid!\n");
  }

  argFreeCmdLine(&cmdLine);
  return 0;
}
