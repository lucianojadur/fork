#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int primes(int pipe_l[2]);


int
main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr,
		        "Error de invocación. Uso: $ ./primes <max_num>\n");
		return -1;
	}
	int max = atoi(argv[1]);
	int pipe_r[2];
	if (pipe(pipe_r) == -1) {
		fprintf(stderr, "Error al crear el pipe derecho inicial\n");
		return 1;
	}

	int pid = fork();
	if (pid > 0){
		close(pipe_r[0]);
		for (int i = 2; i <= max; i++){
			if (write(pipe_r[1], &i, sizeof(int)) == -1) {
				fprintf(stderr, "Error al escribir en el buffer pipe inicial\n");
				return 1;
			}
		}
		//close(pipe_r[1]);
		wait(&pid);
	}
	else if (pid == 0){
		close(pipe_r[1]);
		if (!primes(pipe_r))
			return 1;
	}

	exit(0);
	return 0;
}


int
primes(int pipe_l[2])
{
	int p = 0;
	// leo el primer número del buffer, que es primo
	if (read(pipe_l[0], &p, sizeof(int)) == -1) {
		fprintf(stderr, "Error al leer el pipe izquierdo\n");
		close(pipe_l[0]);
		close(pipe_l[1]);
		return 0;
	}

	printf("primo: %d\n", p);

	int pipe_r[2];
	if (pipe_l[0] != EOF){ 
		if (pipe(pipe_r) == -1) {
			fprintf(stderr,
			        "Error al crear el pipe derecho del filtro de %d\n",
			        p);
			return 0;
		}
	}
	if (pipe_l[0] == EOF){
		exit(0);
	}
	int pid = fork();
	if (pid > 0) {
		// PARENT process (fork returns child's PID to parent, and 1 to child')
		close(pipe_r[0]);
		while (pipe_l[0] != EOF) {
			int n = 0;
			if (read(pipe_l[0], &n, sizeof(int)) == -1) {
				fprintf(stderr,
				        "Error al leer el pipe izquierdo\n");
				close(pipe_l[0]);
				return 0;
			}
			if (n % p) {  // si n no es divisible por p, viaja
				if (write(pipe_r[1], &n, sizeof(int)) == -1) {
					fprintf(stderr, "Error al escribir en el pipe derecho del filtro %d", p);
					return 0;
				}
			}
		}
		//close(pipe_r[1]);	// terminé de leer, cierro el pipe izquierdo
		wait(&pid);	
	} else if (pid == 0) {
		// CHILD process (fork returns 0); creates right filter and pipe pointing to it
		close(pipe_r[1]);	// cierro la escritura del pipe izquierdo
		return primes(pipe_r);
	} else {
		fprintf(stderr, "Error al crear el fork en el filtro de %d\n", p);
		return 0;
	}
	exit(0);

	return 1;
}
