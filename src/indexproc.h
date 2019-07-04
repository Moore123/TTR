#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>

#include <R.h>
#include <Rinternals.h>
#include <Rinterface.h>

#include <R_ext/Arith.h>
#include <R_ext/Error.h>
#include <Rdefines.h>
#include "R_ext/Rdynload.h"

#ifndef __INDEP_H__
#define __INDEP_H__

#define SZRSV 4000
#define INNRSV 2000
#define SHRSV 5000

#define STKSPACE SZRSV+INNRSV+SHRSV

#define MAX_TICKET 240*10+40

#define IndexFileName "indexhft.idx"
#define IDXMARK "RealTimeHFT0.1"

#define normalComma  31
#define TRIEST  800
#define normalTail 11

#define strHead "var hq_str_s"

#endif
