/* 
 * File:   logger.h
 * Author: banshee
 *
 * Created on January 23, 2014, 10:23 PM
 */

#ifndef LOGGER_H
#define	LOGGER_H

#include <time.h>

#define LOG_INFO  0
#define LOG_DEBUG 1
#define LOG_WARN  2
#define LOG_ERROR 3
#define LOG_TRACE 4

#define MAXSIZEMSG 2048

char* getCurrentDate();
void  blog(int level, char *format, ...);


#endif	/* LOGGER_H */


