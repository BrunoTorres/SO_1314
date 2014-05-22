#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

int main()
{
	int cts;
	char *myfifo = "/tmp/cts";

	char str[BUFSIZ];
	while (1){
		printf("> ");
		fflush(stdin);
		fgets(str, sizeof str, stdin);

		cts = open(myfifo, O_WRONLY);
		write(cts, str, sizeof(str));

		memset(str, 0, sizeof(str));
	}
	close(cts);
	return 0;
}