#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>
#include <iso646.h>

/*
Kill process listening on the specified TCP port,
and all the processes established on that port in Linux.

Suitable for killing applications using multiprocess like socat.

Author:
CHEN Qingcan, 2024 spring, Foshan Nanhai China.

Build:
cc -std=gnu11 -Wall -s -o killport killport.c
*/

//----------------------------------------------------------------------------
void die (const char* say) {
	puts (say);
	exit (EXIT_FAILURE);
}

//----------------------------------------------------------------------------
void usage () {
	puts ("Usage: killport {port}");
	exit (EXIT_SUCCESS);
}

//----------------------------------------------------------------------------
// return: port.
long parseArgs (int argc, const char** argv) {
	if (argc < 2) usage ();
	long port = atol (argv[1]);
	if (port <= 0) usage ();
	return port;
}

//----------------------------------------------------------------------------
char* getExecOutput1Line (const char* cmd, char* output1line, const size_t len) {
	if (cmd == NULL or output1line == NULL or len <= 0) die ("NULL getExecOutput1Line");
	puts (cmd);

	FILE* output = popen (cmd, "r");
	if (output == NULL) perror ("popen");

	output1line[0] = '\0';
	if (fgets (output1line, len, output) == NULL) puts ("(NULL output)");
	fputs (output1line, stdout);

	if (ferror (output)) perror ("ferror");
	if (pclose (output) == -1) perror ("pclose");
	return output1line;
}

//----------------------------------------------------------------------------
long getPIDfromNetstat (char* netstat) {
	if (netstat == NULL) die ("NULL getPIDfromNetstat");
	char *token, *saveptr;

	token = strtok_r (netstat, " ", &saveptr);
	for (int column = 1 ; column < 7 and token != NULL ; column++) token = strtok_r (NULL, " ", &saveptr);
	if (token == NULL) die ("NULL strtok_r");

	long pid = atol (token);
	printf ("PID = %ld\n", pid);
	if (pid == 0) die ("PID 0");
	return pid;
}

//----------------------------------------------------------------------------
bool isEmpty (const char* s) {
	if (s == NULL) return true;
	return strspn (s, " \t\r\n") == strlen (s);
}

//----------------------------------------------------------------------------
// state:  LISTEN or ESTABLISHED
// return: retry count, 0 means no process found.
int killPort (const long port, const char* state) {
	char cmd [BUFSIZ], line [BUFSIZ];
	sprintf (cmd, "netstat -pan | grep ':%ld .*\\s*%s'", port, state);

	int retry = 0;
	long lastPID = 0;
	for (;;sleep (3)) {
		getExecOutput1Line (cmd, line, sizeof (line));
		if (isEmpty (line)) {
			puts ("killed");
			break;
		}

		long pid = getPIDfromNetstat (line);
		if (pid == lastPID) {
			retry++;
		} else {
			retry = 0;
			lastPID = pid;
		}

		int killed = 0;
		if (retry < 9) {
			printf ("TERM %ld \n", pid);
			killed = kill (pid, SIGTERM);
		} else {
			printf ("KILL %ld \n", pid);
			killed = kill (pid, SIGKILL);
		}
		if (killed != 0) perror ("kill");
	}
	return retry;
}

//----------------------------------------------------------------------------
int main (int argc, const char** argv) {
	long port = parseArgs (argc, argv);
	while (killPort (port, "LISTEN"     ) > 0) {}
	while (killPort (port, "ESTABLISHED") > 0) {}
	return EXIT_SUCCESS;
}
