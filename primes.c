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

	pid_t pid = fork();

	if(pid > 0){
		close(pipe_r[0]);
		for (int i = 2; i <= max; i++){
			if (write(pipe_r[1], &i, sizeof(int)) == -1) {
				fprintf(stderr, "Error al escribir en el buffer pipe inicial\n");
				return 1;
			}
		}
		close(pipe_r[1]);
		printf("WR del proceso inicial cerrado\n");
		wait(0);
	}
	else if (pid == 0){
		close(pipe_r[1]);
		if (!primes(pipe_r))
			return 1;
		close(pipe_r[0]);
	}
	else
		return 1;

	return 0;
}


int
primes(int pipe_l[2])
{	
	//printf("inicio lectura y filtro en proceso con PID = %d\n", getpid());
	close(pipe_l[1]);
	int p = 0;
	// leo el primer número del buffer, que es primo
	int status = read(pipe_l[0], &p, sizeof(int));
	if (status <= 0) {
		if (status == -1)
			fprintf(stderr, "Error al leer el pipe izquierdo\n");
		close(pipe_l[0]);
		exit(0);
		return 0;
	}
	printf("primo %d\n", p);
	//
	// creo el pipe derecho
	int pipe_r[2];
	if (pipe(pipe_r) == -1) {
		fprintf(stderr,
		        "Error al crear el pipe derecho del filtro de %d\n",
		        p);
		return 0;
	}
	
	pid_t pid = fork();
	if (pid > 0) {
		close(pipe_r[0]);
		int n = 0;
		while ((status = read(pipe_l[0], &n, sizeof(int))) > 0){
			if (n % p) { 
				if (write(pipe_r[1], &n, sizeof(int)) == -1) {
					fprintf(stderr, "Error al escribir en el pipe derecho del filtro %d", p);
					return 0;
				}
	//			printf("escribo %d en el pipe derecho\n", n);
			}
		}
		if (status == -1){
			fprintf(stderr,
				    "Error al leer el pipe izquierdo\n");
			close(pipe_l[0]);
			close(pipe_r[1]);
			return 0;
		}
	//	printf("llegué al EOF en proceso con PID = %d\n", getpid());
		
		close(pipe_l[0]);	// ya no leo
		close(pipe_r[1]);	// ya no escribo
		wait(NULL);
	} else if (pid == 0) {
		close(pipe_r[1]);
		return primes(pipe_r);
	} else {
		fprintf(stderr, "Error al crear el fork en el filtro de %d\n", p);
		return 0;
	}
	exit(0);
	return 1;
}
