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
#include "R.h"
#include "Rdefines.h"
#include "Rinternals.h"

#include "R_ext/Arith.h" 
#include "R_ext/Error.h"
#include "Rversion.h"
#include <dirent.h>

#include "cfg.h"
#include "bson.h"
#include "gridfs.h"
#include "mongo.h"

#ifndef __MONGO_CFG_H
#define __MONGO_CFG_H

#define setConn 1
#define setDB  1<<1
#define setCollection  1<<2
#define setBsonInited 1<<3
#define setCols 1<<4
#define setV 1<<5
#define setGridfs 1<<6 
#define setNoBQ   1<<7 
#define setREMOVE   1<<8 
#define setgfs   1<<9 
#define setgfile 1<<10 


typedef enum {
    ID = 0,
    STK,
    TO,
    YC,
    NW,
    HI,
    LO,
    NWB,
    NWO,
    VL=9,
    AM=10,
    B1V,
    B1P,
    B2V,
    B2P,
    B3V,
    B3P,
    B4V,
    B4P,
    B5V,
    B5P=20,
    O1V=21,
    O1P,
    O2V,
    O2P,
    O3V,
    O3P,
    O4V,
    O4P,
    O5V,
    O5P,
    D,
    T } Elements;

typedef enum {
    EleTime = 0,
	EleHigh,
	EleLow,
	EleAveragePrice,
	EleTickPrice,
	EleTickAm=5,
	EleBidPrice,
	EleAskPrice,
	EleBid,
	EleAsk,
	EleBidInc=10,
	EleBidDec,
	EleAskInc,
	EleAskDec,
	ElePassBid,
	EleActBid=15,
	ElePassAsk,
	EleActAsk,

	EleFlatFinger,
	EleType
} TradeElements;

typedef enum Trade_State {
	T_balance = 0,
	T_rise,
	T_rise1,
	T_rise2,
	T_down,	
	T_down1,	
	T_down2,	
	T_rise_limit,
	T_down_limit,
	T_flat,
	T_flat1,
	T_unset
} TRADE_STATE;

//(gp->setStatus & MASKSET ) ^ (setConn | setDB | setCollection | setBsonInited)) == 0
#define MASKSET 0xF

#define CHUNKSIZE  (8<<20)

#define METHOD_BRIEF  1
#define METHOD_DEPTRANS 2
#define METHOD_TICKTR  3

#define CONVERSLEN 20
#define MINIVOLVAR   199
#define EMPTYCOLS 33
#define MINITICK  60

#define HUGEAM 100000

#define MiniHeadSize (sizeof(unsigned int)*3)

extern int ndebug(char *fmt,...);
extern void tic(struct timeval *);
extern void toc(struct timeval *);

extern int time2num(char *);
extern int name2num(char *);
extern void digest2hex(unsigned char *, char *);

typedef struct MongoServers {
   char *MongoServer;
   int Mongoport;
   int id;
   struct MongoServers *next;
} mongoServer;

typedef struct QueryString {
   char *key;
   char *opt;
   float v;
   int skip;
   int limits;

} queryString;

typedef struct QueryValueCols {
   char *dm;
   struct QueryValueCols *next;
} qVdms; 

typedef struct QueryValue{
   unsigned int nCols,nRows;
   double *v;
   struct QueryValue *next;
} qV; 

typedef qV TickRaw;
 
typedef struct MongoParam {
   mongo *conn;
   MONGO_EXPORT int status;
   mongoServer *Servers;
   char *MongoDB;
   char *gridFileName;
   int nServer;

   u_int32_t setStatus;

   queryString *qs;
   mongo_cursor *cursor;

   char *strValue;
   char *ns;

   qV *qVlist,*qVcurr;
   SEXP ColNames;
   int nCols,nRows;

   long date;  

   SEXP result;

   struct timeval timer;

} mongoParam;

typedef struct TransOptions {
   u_int32_t  methods;
   long       gaint;
   u_int32_t  *dims;
   u_int32_t  *tims;
   void    *data_ptr;
   void    *out_ptr;
}  transOptions;

extern mongoParam *getGPRObject(SEXP);
extern TickRaw *tick2raw(qV *, int );
extern TickRaw *tick2detail(qV *, int );
extern double diffTime(double , double );
extern int Tistrade(double );
extern int setDim(SEXP , int , int );
#endif
