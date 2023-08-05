# arg
A minimalist command line parser written in C

# Why
I cought myself writing command line utilities quite often and having to rely on positional arguments.

# How to use
There are only 2 public functions.
``` 
ARGCmdLine* argCmdLineParse(int argc, wchar_t** argv);
```
and
``` 
void argCmdLineFree(ARGCmdLine* cmdLine);
```

Just forward you main arguments to `argCmdLineParse` and it will return an `ARGCmdLine` structure filled with parsed information about your command line.

# Features
 - Options starts with '-'
 - Options can be followed by a value
   - supported values are:
     - integers
     - floats
     - booleans (true or false)
     - strings (with or without quotes)
 - Options can be followed by multiple values of different types
 - The parser ignores arguments at the start that does not begin with a '-'. This allows programs to require positional arguments.

# Example
````
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
````

## Example command line
A programm called `cmdlineparse.exe` using the code above would display the following output based on the provided arguments:
```cmdlineparse.exe -foo bar -number 100 -someFloat 32.9374 -muli "Hello, Sailor!" /usr/bin/foo 42 --whatever true
cmdlineparse "I'm a positional argument and will be ignored!"  -foo bar -number 100 -someFloat 32.9374 -multi "Hello, Sailor!" /usr/bin/foo 42 --whatever true
Option: foo
Value 1: bar (String)
Option: number
Value 1: 100 (Integer)
Option: someFloat
Value 1: 32.937401 (Float)
Option: muli
Value 1: Hello, Sailor! (String)
Value 2: /usr/bin/foo (String)
Value 3: 42 (Integer)
Option: -whatever
Value 1: true (Boolean)
```


