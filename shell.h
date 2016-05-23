#define MAX_ARG 1000
#define TAM_BUFFER 1000
#define MAX_BACK 10

int leer(char *cadena);
void parsear (char *buffer, char **argumentos);
void interpretar(char **args, pid_t *pid);
void sen_CHLD();
void sen_INT();
int background(char **args);
void ponerLista(pid_t *lista, int posicion);
void eliminarLista(pid_t *lista, pid_t proceso);
char* redireccion(char **args);
