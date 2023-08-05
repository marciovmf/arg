#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "arg.h"


int wmain(int argc, wchar_t** argv)
{
  // Parse the command line
	ARGCmdLine* cmdLine = argCmdLineParse(argc, argv);

	// Do stuff with the parsed command line
	for (int i = 0; i < cmdLine->numOptions; i++)
	{
		wprintf(L"Option: %s\n", cmdLine->options[i].option);
    ARGOption* option = &cmdLine->options[i];

		for (int j = 0; j < option->numValues; j++)
		{
			switch (cmdLine->options[i].values[j].type)
			{
				case INTEGER:
					printf("Value %d: %d (Integer)\n", j + 1, option->values[j].value.intValue);
					break;
				case FLOAT:
					printf("Value %d: %f (Float)\n", j + 1, option->values[j].value.floatValue);
					break;
				case BOOL:
					printf("Value %d: %s (Boolean)\n", j + 1, option->values[j].value.boolValue ? "true" : "false");
					break;
				case STRING:
					wprintf(L"Value %d: %s (String)\n", j + 1, option->values[j].value.stringValue);
					break;
				default:
					printf("Value %d: None\n", j + 1);
					break;
			}
		}
	}

	// Free command line object
	argCmdLineFree(cmdLine);

	return 0;
}
