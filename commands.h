void printError();
void changeDirectory(char *dir);
void redirect(char (*path)[200], char *arg[], int arg_count);
void executeParallelCmd(char *args[], int arg_count, char (*path)[200]);
int executeAllOtherCommands(char (*path)[200], char *args[], int i, int arg_count);
void executeCommands(char *args[], int arg_count, char (*path)[200]);