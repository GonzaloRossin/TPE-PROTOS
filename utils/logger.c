#include "../include/logger.h"

LOG_LEVEL current_level = DEBUG;

bool error_flag = false;

void setLogLevel(LOG_LEVEL newLevel) {
	if ( newLevel >= DEBUG && newLevel <= FATAL )
	   current_level = newLevel;
}

char * levelDescription(LOG_LEVEL level) {
    static char * description[] = {"DEBUG", "INFO", "ERROR", "FATAL"};
    if (level < DEBUG || level > FATAL)
        return "";
    return description[level];
}


void print_log(LOG_LEVEL level, const char *fmt, ...){
    if(level >= current_level){
        fprintf (stderr, "%s: ", levelDescription(level)); 
        va_list arg; 
        va_start(arg, fmt); 
        vfprintf(stderr, fmt, arg);
        va_end(arg);
        fprintf(stderr,"\n"); 
    }
	if ( level==FATAL) exit(1);
    if(level==ERROR) error_flag = true; 
}