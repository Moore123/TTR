#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <netdb.h>
#include <errno.h>
#include <syslog.h>
#include <signal.h>
#include <semaphore.h>

#include <unistd.h>
#include <stdarg.h>

#include <stdbool.h>
#include <curl/curl.h>
#include <unistd.h>
#include <string.h>

#include <dirent.h>

#include "indexproc.h"

#ifndef __INDP_CFG_H
#define __INDP_CFG_H

extern int ndebug(char *fmt,...);

extern int time2num(char *);
extern int name2num(char *);

extern int cntComma1(char *);
extern int cntComma2(char *);
extern int cntCommaX(char *);
extern int cntSTK(int);
extern int ordSTK(char *);
extern int  Lfread(long *,int ,int ,FILE *);
extern int Tistrade(double );

#endif
