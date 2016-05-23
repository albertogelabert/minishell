/*Albert Gelabert Mena
 Joan Lluís Serra Rebassa*/

//#define _POSIX_C_SOURCE 200809L
#define _POSIX_C_SOURCE 200112L
#include <stdlib.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "shell.h"


int contProcesos = 0;//Contador de procesos
int foreground = 0;//Marca la espera del proceso padre durante la ejecución de un proceso en foreground
int back = 0;//Indica si un proceso se ejecutará en background o no
pid_t pid_foreground;//Guarda el pid 
pid_t back_pid[MAX_BACK]; //Lista de pids del procesos en background


//Función que lee líneas del teclado, devuelve 0 si no ha leído nada, o el tamaño del string leído en caso contrario 
int leer(char *cadena){//Parámetro: buffer donde se guardan las líneas
	int tam = 0;
	char *linea;
	linea = readline("$>");
	if (linea != NULL){
		add_history(linea);//Añadimos el comando al historial
		tam = strlen(linea);
		strcpy(cadena,linea);//Volcamos línea al buffer
		tam++;
		cadena[tam] = '\0';//Marcamos el buffer con el carácter de final
		free(linea);//Liberamos espacio
	} else {
		printf("Introduzca un comando válido\n");
		exit(0);
	}
	return tam;
}

//Función que separa el buffer leído en los distintos argumentos. No devuelve nada
void parsear (char *buff, char **args){//Parámetros: String buffer con la línea leída, array de String argumentos donde se almacena
	char separador[] = " \n\t\r=";
	int i = 0;
	if(buff){//Si ha leído algo
		args[i] = strtok(buff,separador);
		while (args[i] != NULL){//Separamos mientras encontremos un NULL
			i++;
			args[i] = strtok(NULL,separador);
		}
	}
 }

//Función que interpreta los comandos y sus argumentos. No devuelve nada.
//Parámetros: array de String con los argumentos, array de pid donde se almacena procesos en background.
void interpretar(char **args, pid_t *pid){
	
	char path[1000];//Almacena path para la función cd
	char valor[100];//Usada en export
	char name[100];//Usada en export
	int i = 0;
	pid_t pid_h;//Guarda el pid del proceso hijo
	int k;
	int contador = 0;
	k= 0;
	int lon = sizeof(char*)*1000;//Asignamos un tamaño de lectura
	char lectura[lon];
	int fd;//Usado guardar el descriptor de archivo
	char *redir;//Guarda el nombre de fichero
 
	if(strcmp(args[i], "cd") == 0){
		i++;
		if(args[i]==NULL){
			printf("Error de comando. La sintaxis es: cd path\n");
		} else {
			strcpy(path, args[i]);//copiamos el directorio
			chdir(path);//Ejecutamos el cambio de path
		}
	
	} else if(strcmp(args[i], "help") == 0){
		printf("Este Shell admite los siguientes comandos internos:\n");
		printf("\t cd | exit | help | export | source | jobs\n");
		printf("cd     -- permite cambiar el directorio actual\n");
		printf("exit   -- permite salir del programa - También puede presionar ctrl + c\n");
		printf("help   -- muestra la lista de comandos admitida\n");
		printf("export -- cambia el valor de la variable de entorno especificada\n");
		printf("source -- Ejecuta comandos leídos de un fichero\n");
		printf("jobs   -- Muestra los procesos que se están ejecutando en segundo plano\n");
		printf("Además, este shell admite hasta %d comandos ejecutados en Background.\n",MAX_BACK);
		printf("Introduzca un comando y pulse Enter:\n");
	
	} else if(strcmp(args[i], "export") == 0){
		i++;
		if(args[i]!=NULL){
			strcpy(name,args[i]);//Guardamos el nombre de la variable
			i++;
			strcpy(valor,args[i]);//Guardamos el valor de la variable
			setenv(name,valor,1);//Creamos la nueva variable de entorno con el valor guardado
		} else{
			printf("Error de comando. La sintaxis es: export name valor");
		}
	
	} else if(strcmp(args[i], "source") == 0){
		i++;
		if(args[i]==NULL){
			printf("Error de comando. La sintaxis es: source nombre_archivo");
		} else{
			FILE *f = fopen(args[i], "r");
			if (f!=NULL){
				while(fgets(lectura, 100, f)!=NULL){//Leemos una línea del fichero
					if (lectura[0] !='#'){//Si la línea no es comentario, la ejecutamos
						parsear(lectura,args);
						interpretar(args, pid);
					}
				}	
				fclose(f);
			}
		}
	
	} else if(strcmp(args[i], "jobs") == 0){	
		if(contProcesos==0){
			printf("\t[%d] No se ha encontrado ningún proceso en segundo plano.\n",contador);
		} else{
			printf("El número de procesos en segundo plano es %d:\n",contProcesos);	
			for (k=0; k<10;k++){//Imprimimos los procesos guardados en el array de pid
				if (pid[k] != -1){
					contador++;
					printf("\t[%d] PID = %d\n",contador,pid[k]);
				}
			}
		}
	
	} else if (strcmp(args[i], "exit") == 0){
		exit(0);
	
	} else {
		foreground = 0;//Inicializamos
		back = background(args);//Asignamos a back lo retornado por la función background
		pid_h = fork();
		if(pid_h == 0){//Proceso hijo
			signal(SIGCHLD, SIG_DFL);//Ponemos por defecto los signals
			signal(SIGINT, SIG_DFL);
			redir = redireccion(args);//Asignamos a redir lo retornado por la función redireccion
			if(redir!=NULL){
				fd = open(redir, O_WRONLY|O_CREAT|O_TRUNC,S_IRWXU);//Abrimos fichero con permisos
				if(fd < 0){
					printf("Error al abrir el archivo");
				}
				close(1);//Cierra salida estándar, stdout
				dup(fd);//Reasignamos el descriptor de fichero de la salida estándar
				close(fd);//Cerramos fichero
			}
			if(back == 1 && contProcesos>=MAX_BACK){
				printf("Has excedido el número de procesos en segundo plano");
			} else{
				execvp(*args, args);//Ejecutamos el comando externo
			}
			printf("Error: el comando no existe\n");
			exit(1);//Matamos el proceso con un error
		} else if(pid_h >0){//Proceso padre
			if(back==1){//Si es un 1, es que hay un proceso en background
				if(contProcesos<MAX_BACK){
					ponerLista(back_pid,pid_h);//Ponemos el pid del proceso en la lista
				} else{
					printf("Se está ejecutando el número máximo de procesos en segundo plano disponibles.\nNo se puede ejecutar ninguno más\n");
				}
			} else{//Si back es 0, es que el nuevo proceso es foreground
				pid_foreground = pid_h;
				while(!foreground);//Bucle para poner en espera el padre
			}		
		} else{//fork devuelve un número negativo, que indica error
			printf("Se ha producido un error.\n");
		}
	}
}

//Función que pone en la lista de pid los nuevos procesos en background. No devuelve nada.
//Parámetros: array de pid donde se almacena procesos en background, el pid del nuevo proceso.
//Usamos -1 para indicar que la posición del array está vacía.
void ponerLista(pid_t *lista, pid_t proceso){
	int k;
	k = 0;
	while (lista[k] != -1 && k<MAX_BACK){//Busca la primera posición vacía, es decir, -1
		k++;
	}
	if(lista[k] == -1){
		lista[k] = proceso;
		contProcesos++;
	}
}

//Función que elimina de la lista de pid el proceso en background terminado. No devuelve nada.
//Parámetros: array de pid donde se almacena procesos en background, el pid del proceso a eliminar.
//Usamos -1 para indicar que la posición del array está vacía.
void eliminarLista(pid_t *lista, pid_t proceso){
	int k;
	k = 0;
	while (lista[k] != proceso && k<MAX_BACK){
		k++;
	}
	if(lista[k] == proceso){
		lista[k] = -1;
		contProcesos--;
	}
}

//Función que comprueba si el comando introducido contiene el símbolo '>', es decir, si hay redireccionamiento.
//Devuelve NULL o el nombre del fichero donde se va a redireccionar. Parámetros: array de String con los argumentos.
char* redireccion(char **args){
	char *salida;
	int k = 0;
	while(args[k] != NULL){
		if (args[k][0] =='>'){
			args[k] = NULL;
			k++;
			salida = args[k];
			return salida;
		}
		k++;
	}
	return NULL;
}	

//Función que comprueba si el comando introducido contiene el símbolo '&', es decir, si el nuevo proceso es en background.
//Devuelve 1 en caso de que aparezca, o 0 si no es un proceso en background. Parámetros: array de String con los argumentos.
int background(char **args){
	int k;
	k= 1;
	while(args[k] != NULL){
		k++;
	}
	k--;
	int tam = strlen(args[k])-1;
	if (args[k][tam] == '&'){
		args[k] = NULL;
		return 1;	
	}
	return 0;
}

//Función que envía la señal de un proceso hijo al proceso padre indicando que ha acabado.
//No devuelve nada. No tiene parámetros.
void sen_CHLD(){
	signal(SIGCHLD, sen_CHLD);
	pid_t ended;
	ended = waitpid(-1, NULL, WNOHANG);//Guarda el pid del proceso finalizado
	if(back){
		eliminarLista(back_pid, ended);
	} else{
		foreground = 1;//Termina el bucle del proceso padre para que deje de esperar
	}
}

//Función que envía la señal de un proceso hijo al proceso padre indicando que ha acabado.
//No devuelve nada. No tiene parámetros.	
void sen_INT(){
	signal(SIGINT, sen_INT);
	if(foreground == 0){//Proceso hijo es foreground
		kill(pid_foreground,9);//Matamos el proceso con el 9
	} else{
		if(back == 1){
			printf("CTRL+C está desactivado para procesos en background");
		} else{
			printf("My_Shell se va a cerrar...");
			exit(0);
		}
	}
}

int main (){

	signal(SIGCHLD, sen_CHLD);//Inicializa las señales
	signal(SIGINT, sen_INT);
	char *argumentos[MAX_ARG+1];//Guarda los argumentos
	char buffer[TAM_BUFFER];//Guarda la línea leída
	int k;
	for (k=0; k<10;k++){//Inicializa la lista de procesos en background a -1 para indicar que está vacía
		back_pid[k] = -1;
	}
	printf("\n\t*************BIENVENIDO A MY_SHELL************\n");
	printf("Si no sabe como utilizar este programa, teclee help y pulse Enter\n");
	printf("Este shell admite hasta %d comandos ejecutados en Background.\n",MAX_BACK);
	while (leer(buffer)!=0){
		parsear(buffer,argumentos);
		if(argumentos[0]!=NULL)
			interpretar(argumentos, back_pid);
		}
	return 0;
}
