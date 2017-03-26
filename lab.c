#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>

#define PI 3.14159265358979323846

char *execName; // executable file name

struct TailorProcessInfo {
	pid_t pid;
	int elementIndex;
	long double elementValue;
}; // structure to be written in a file

long double factorial(long double n)
{
   return (n < 2) ? 1 : factorial(n - 1) * n;
}

int main(int argc[], char *argv[])
{
	// get executable file name
	execName = basename(argv[0]);

	if (argv[1] == NULL) {
		perror(basename(execName));
		return 1;
	} else if (argv[2] == NULL) {
		perror(basename(execName));
		return 1;
	}

  const char* TEMPORARY_FILE_PATH = "/tmp/psstore";
  const char* RESULTS_FILE_PATH = "results";

  // get user arguments
  int N = atoi(argv[1]);
  int n = atoi(argv[2]);

  // open file which will be used to save processes PIDs and calculated info
  int processesStoreFile;
  if ((processesStoreFile = open(TEMPORARY_FILE_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0777)) < 0) {
		perror(basename(execName));
		return 1;
	};

  int i;
  int j;

  // process Tailor's decomposition
  for (i = 0; i < N; i++) {
    for (j = 0; j < n; j++) {
      pid_t pid = fork();

      if (pid == 0) {
        struct TailorProcessInfo psInfo;

        psInfo.pid = getpid();
        psInfo.elementIndex = i;

        // long double index = powl(-1, j - 1);
				long double index = (j - 1) % 2 == 0 ? 1 : -1;
        long double fact = factorial(2 * j - 1);
        long double x = powl(2 * PI * i / N, 2 * j - 1);
        psInfo.elementValue = index / fact * x;

        // printf("\npid: %d | i: %d | j: %d | element value: %Le | index: %Le | fact: %Le | x: %Le\n",
        //   psInfo.pid,
        //   psInfo.elementIndex,
        //   j,
        //   psInfo.elementValue,
        //   index,
        //   fact,
        //   x
        // );

        write(processesStoreFile, &psInfo, sizeof(struct TailorProcessInfo));

        exit(0);
      } else if (pid > 0) {
        continue;
      } else {
        perror(execName);
      }
    }
  }

  // wait till all processes end
  while (1) {
      int status;
      pid_t done = wait(&status);
      if (done == -1) {
          if (errno == ECHILD) break;
      } else {
          if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
             	perror(execName);
              exit(1);
          }
      }
  }
  close(processesStoreFile);

  // shaping final results
  FILE* resultsFile = fopen(RESULTS_FILE_PATH, "w+");
  int temporaryFile = open(TEMPORARY_FILE_PATH, O_RDONLY);
  long double* results = (long double*)calloc(N, sizeof(long double));

  struct TailorProcessInfo* resultsArray =
    (struct TailorProcessInfo*)malloc(sizeof(struct TailorProcessInfo) * n * N);

  read(temporaryFile, resultsArray, sizeof(struct TailorProcessInfo) * n * N);
  close(temporaryFile);

  for (i = 0; i < N * n; i++) {
		// check if function value can reach INFINITY or
    if (results[resultsArray[i].elementIndex] + resultsArray[i].elementValue != INFINITY
      && results[resultsArray[i].elementIndex] + resultsArray[i].elementValue != -INFINITY
      && results[resultsArray[i].elementIndex] + resultsArray[i].elementValue
				== results[resultsArray[i].elementIndex] + resultsArray[i].elementValue) {
      results[resultsArray[i].elementIndex] += resultsArray[i].elementValue;
    }
  }

	// format results output to file and terminal
  for (i = 0; i < N; i++) {
    printf("y[%d] = %Lf\n", i, results[i]);
    fprintf(resultsFile, "y[%d] = %Lf\n", i, results[i]);
  }

	fclose(resultsFile);

  exit(0);
}
