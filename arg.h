#ifndef ARG_H
#define ARG_H

#include <stdbool.h>
#include <wchar.h>

typedef enum 
{
	INTEGER,
	FLOAT,
	BOOL,
	STRING,
	NONE
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
	wchar_t* option;
	ARGValue* values;
	int numValues;
} ARGOption;

typedef struct
{
	int numOptions;
	ARGOption* options;
} ARGCmdLine;

ARGCmdLine* argCmdLineParse(int argc, wchar_t** argv);
void argCmdLineFree(ARGCmdLine* cmdLine);

#endif  // ARG_H
