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

	int processCounter = 0;

  // process Tailor's decomposition
  for (i = 0; i < N; i++) {
    for (j = 1; j <= n; j++) {
			if (processCounter >= n) {
				int status;
				pid_t done = wait(&status);
				processCounter--;
			}

			processCounter++;
      pid_t pid = fork();
      if (pid == 0) {
      	struct TailorProcessInfo psInfo;
	      psInfo.pid = getpid();
        psInfo.elementIndex = i;
	      // long double index = powl(-1, j - 1);
			  long double index = (j - 1) % 2 == 0 ? 1 : -1;
	      long double fact = factorial(2 * j - 1);
				long double angle = 2 * PI * i / N;

				if (angle > (PI / 2) && angle < PI) {
					angle = (PI / 2) - (angle - (PI / 2));
				}

				if (angle > PI) {
					angle = (PI - (angle - PI));
					angle = -((PI / 2) - (angle - (PI / 2)));
				}

        long double x = powl(angle, 2 * j - 1);
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
	      printf("\npid: %d | y[%d][%d] | element value: %Le\n",
	        psInfo.pid,
	        i,
					j,
	        psInfo.elementValue
	      );

	      if (write(processesStoreFile, &psInfo, sizeof(struct TailorProcessInfo)) < 0) {
					perror(basename(execName));
					exit(1);
				};

	      exit(0);
	    } else if (pid > 0) {
	      continue;
	    } else {
	      perror(execName);
	    }
			// } else {
			// 	int status;
			// 	pid_t done = wait(&status);
			// 	processCounter--;
			// 	j--;
			// }
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
	if (close(processesStoreFile) < 0) {
		perror(basename(execName));
		return(1);
	};

  // shaping final results
	int temporaryFile;
	if ((temporaryFile = open(TEMPORARY_FILE_PATH, O_RDONLY)) < 0) {
		perror(basename(execName));
		return 1;
	};

  FILE* resultsFile = fopen(RESULTS_FILE_PATH, "w+");

	if (resultsFile == NULL) {
		perror(basename(execName));
		return(1);
	}

  long double* results = (long double*)calloc(N, sizeof(long double));

  struct TailorProcessInfo* resultsArray =
    (struct TailorProcessInfo*)malloc(sizeof(struct TailorProcessInfo) * n * N);

	if (read(temporaryFile, resultsArray, sizeof(struct TailorProcessInfo) * n * N) < 0) {
		perror(basename(execName));
		return(1);
	};
	if (close(temporaryFile) < 0) {
		perror(basename(execName));
		return(1);
	};

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

	if (fclose(resultsFile) == EOF) {
		perror(basename(execName));
		return(1);
	};

  exit(0);
}
