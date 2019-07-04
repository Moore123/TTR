#ifndef __EXP2MONGO_H 
#define __EXP2MONGO_H

#include "bson.h"
#include "mongo.h"

typedef enum {
    TypeInt =0,
    TypeStr,
    TypeDouble,
    TypeLong,
    TypeDate,
    TypeTime
  } enumMongoType;

typedef struct DataParameter {
   double *vptr;
   long dates;
   long ticket; 
} dataParameter;


typedef struct UrlResp_M{
	int prevstate;
	int nextstate;
	char *smname;
	int smassign;
	
} urlResponse;


typedef struct rsinaOPT {
	char *name;
	enumMongoType type;
	char *value;

	} rsinaOpt;

typedef struc GlobalParameter {
       mongo_connection *conn;
}  globalParameter;

#endif
