#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#ifndef NARGS
#define NARGS 4
#endif

int
main(int argc, char *argv[])
{
	char command[100];
	char *file = malloc(1);
	size_t len = 0;
	char *argv2[NARGS + 1];
	int nread;

	// COMMAND PARSE
	for (int i = 1; i < argc; i++) {
		strcat(command, argv[i]);
		strcat(command, " ");
	}
	command[strlen(command) - 1] = '\0';
	//	printf("command to execute: --%s--\n", command);

	// ARGUMENTS/FILES PARSE
	argv2[0] = argv[0];

	size_t i = 1;
	while ((nread = getline(&file, &len, stdin)) != -1) {
		char *file_cpy = malloc(strlen(file));  //"\0";
		strncpy(file_cpy,
		        file,
		        (file[strlen(file) - 1] == '\n') ? strlen(file) - 1
		                                         : strlen(file));
		// printf("file copy = --%s--\n", file_cpy);
		argv2[i] = file_cpy;
		i++;
	}
	free(file);
	argv2[i] = NULL;

	// for (size_t n = 0; n <= i; n++){
	//	printf("argv[%lu] = %s\n", n, argv2[n]);
	// }

	execvp(argv[1], argv2);

	printf("error, no se ejecutó exec\n");
	return 0;
}
