#ifndef __logger_h_
#define __logger_h_

#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <limits.h>  /* LONG_MIN et al */
#include <stdio.h>   /* for printf */
#include <stdlib.h>  /* for exit */
#include <string.h>  /* memset */
#include <sys/socket.h> // socket
#include <sys/types.h>  // socket
#include <stdarg.h> //para el parametro ...


typedef enum {DEBUG=0, INFO, ERROR, FATAL} LOG_LEVEL;

extern LOG_LEVEL current_level;
extern bool error_flag;

#define SB_FAILURE				-1
#define SB_MAX_FRAG_LENGTH		4096

typedef struct _StringFragment {
	struct _StringFragment	*next;
	int						length;
	char					*str;
} StringFragment;

typedef struct _StringBuilder {
	struct _StringFragment	*root;
	struct _StringFragment	*trunk;
	int						length;
} StringBuilder;

StringBuilder	*sb_create();
int				sb_empty(StringBuilder *sb);
int				sb_append(StringBuilder *sb, const char *str);
int				sb_appendf(StringBuilder *sb, const char *format, ...);
char			*sb_concat(StringBuilder *sb);
void 			sb_reset(StringBuilder *sb);
void			sb_free(StringBuilder *sb);

/**
*  Minimo nivel de log a registrar. Cualquier llamada a log con un nivel mayor a newLevel sera ignorada
**/
void setLogLevel(LOG_LEVEL newLevel);

void print_log(LOG_LEVEL level, const char *fmt, ...);


// Debe ser una macro para poder obtener nombre y linea de archivo. 
/*
char * levelDescription(LOG_LEVEL level);

#define log_print(level, fmt, ...)   {if(level >= current_level) {\
	fprintf (stderr, "%s: %s:%d, ", levelDescription(level), __FILE__, __LINE__); \
	fprintf(stderr, fmt, ##__VA_ARGS__); \
	fprintf(stderr,"\n"); }\
	if ( level==FATAL) exit(1);}
*/

#endif

