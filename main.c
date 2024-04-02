#include <stdlib.h>

#include "modes.h"
#include "commands.h"

int main(int argc, char *argv[]) {
	if (argc == 1) {
		interactiveMode();
	}
	else if (argc == 2) {
		batchMode(argv[1]);
	}
	else {
        printError();
		exit(1);
	}
	return 0;
}