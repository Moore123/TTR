/*
 * =====================================================================================
 *
 *       Filename:  Rmongo.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2011年05月16日 14时07分01秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Michael Moore (lyxmoo), lyxmoo@gmail.com
 *        Company:  who's who
 *
 * =====================================================================================
 */

#include "Rmongo.h"

typedef enum MongoOpt {
	defmHost = 0,
	defmPort,
	defnServer,
	defServer,
	defPort,
	defQuery,
	defQueryOne,
	defDB,
	defDoConnect,
	defOther,
	defNull
} mongoOpt;

typedef struct ServerList {
	char *optName;
	mongoOpt opType;
	char *operation;
} serverList;

serverList srvOpt[] = {
	{"mongoHost=%s", defmHost, NULL},
	{"mHost=%s", defmHost, NULL},
	{"mongoPort=%d", defmPort, NULL},
	{"mPort=%d", defmPort, NULL},
	{"Server%d=%s", defServer, NULL},
	{"Port%d=%d", defPort, NULL},
	{"nServers=%d", defnServer, NULL},
	{"DataBase=%d", defDB, NULL},
	{"DB=%d", defDB, NULL},
	{"DoConnect=%u", defDoConnect, NULL},
	{"doConnect=%u", defDoConnect, NULL},
	{NULL, defNull, NULL}
};

#define lSLen 8
#define lServer 6
#define lPort 4

void R_finalizeGP(SEXP);

void digest2hex(unsigned char digest[16], char hex_digest[33])
{
	static const char hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	int i;
	for (i = 0; i < 16; i++) {
		hex_digest[2 * i] = hex[(digest[i] & 0xf0) >> 4];
		hex_digest[2 * i + 1] = hex[digest[i] & 0x0f];
	}
	hex_digest[32] = '\0';
}

int freeServerList(mongoServer * list)
{
	int retval;
	mongoServer *ServerCur, *ServerPtr;

	retval = 0;
	ServerCur = list;
	while (ServerCur != NULL) {
		ServerPtr = ServerCur;
		ServerCur = ServerCur->next;

		if (ServerPtr->MongoServer != NULL)
			free(ServerPtr->MongoServer);
		free(ServerPtr);

	}

	return (retval);

}

int insServer(mongoServer * hd, int id, char *s)
{
	int retval;
	retval = 0;

	mongoServer *serverPtr;
	serverPtr = hd;

	while (serverPtr != NULL) {
		if (serverPtr->id == id) {
			serverPtr->MongoServer = Calloc(STR_BUFF_LEN, char);
			snprintf(serverPtr->MongoServer, STR_BUFF_LEN, "%s", s);

			break;
		}
		serverPtr = serverPtr->next;
	}

	return (retval);
}

int insPort(mongoServer * hd, int id, int port)
{
	int retval;
	retval = 0;

	mongoServer *serverPtr;
	serverPtr = hd;

	while (serverPtr != NULL) {
		if (serverPtr->id == id) {
			serverPtr->Mongoport = port;
			break;
		}
		serverPtr = serverPtr->next;
	}

	return (retval);

}

int doConnect(mongoParam * mp)
{
	int retval;
	retval = 0;

	mongoServer *serverPtr;

	serverPtr = mp->Servers;

	mp->conn = Calloc(1, mongo);

	if (mp->nServer >= 2) {

		mongo_replset_init(mp->conn, "replset-my-app");
		while (serverPtr->next != NULL) {
			if ((serverPtr->MongoServer != NULL) && (serverPtr->Mongoport != 0))
				mongo_replset_add_seed(mp->conn, serverPtr->MongoServer, serverPtr->Mongoport);
			serverPtr = serverPtr->next;
		}

		mp->status = mongo_replset_connect( mp->conn );

	} else if (mp->nServer == 1) {

		mp->status = mongo_connect(mp->conn, mp->Servers->MongoServer, mp->Servers->Mongoport);

	}

	if (mp->status == MONGO_OK)
		mp->setStatus |= setConn;

	return (retval);

}

int ProcessPara(mongoParam * mp, SEXP list)
{

	SEXP pname, values;
	int retval, i, n, j, sId, sPort;
	char *sIp;
	char *lptr;

	mongoServer *ServerCur, *ServerPtr;
	int found;

	retval = 0;

	pname = getAttrib(list, R_NamesSymbol);
	i = n = 0;

	while (1) {

		if (n >= LENGTH(list))
			break;
		values = VECTOR_ELT(list, n);

		found = FALSE;
		i = 0;

		lptr =strdup( CHAR(STRING_ELT(pname, n)) );

		while ((srvOpt[i].optName != NULL) && (found != TRUE)) {

			if (srvOpt[i].opType == defServer) {

				if (strncmp(srvOpt[i].optName, lptr, lServer) == 0) {
					sIp = Calloc(STR_BUFF_LEN, char);
					snprintf(sIp, STR_BUFF_LEN, "%s", CHAR(STRING_ELT(values, 0)));
					sscanf(lptr, "Server%d=", &sId);
					insServer(mp->Servers, sId, sIp);
					free(sIp);
					i++;
					break;
				}

			} else if (srvOpt[i].opType == defPort) {
				if (strncmp(srvOpt[i].optName, lptr, lPort) == 0) {
					sPort = (int)REAL(VECTOR_ELT(list, n))[0];
					sscanf(lptr, "Port%d=", &sId);
					insPort(mp->Servers, sId, sPort);
					i++;
					break;
				}
			}

			if (strncmp(srvOpt[i].optName, lptr, strlen(lptr)) != 0) {
				i++;
				continue;
			}

			found = TRUE;

			switch (srvOpt[i].opType) {

			case defnServer:

				mp->nServer = (int)REAL(VECTOR_ELT(list, n))[0];

				j = 0;
				ServerPtr = mp->Servers;

				if (mp->Servers != NULL) {

					while (ServerPtr->next != NULL) {
						ServerPtr = ServerPtr->next;
					}
				}

				while (j < mp->nServer) {

					ServerCur = Calloc(1, mongoServer);
					if (mp->Servers == NULL) {
						mp->Servers = ServerCur;
						ServerPtr = mp->Servers;
					} else {
						ServerPtr->next = ServerCur;
						ServerPtr = ServerPtr->next;
					}
					ServerCur->MongoServer = NULL;
					ServerCur->Mongoport = 0;
					ServerCur->next = NULL;
					ServerCur->id = j + 1;
					j++;
				}
				break;

			case defmHost:
				mp->nServer = (int)REAL(VECTOR_ELT(list, n))[0];
				break;

			case defmPort:
				mp->nServer = (int)REAL(VECTOR_ELT(list, n))[0];
				break;

			case defDB:
				mp->MongoDB = strdup(CHAR(STRING_ELT(values, 0)));
				mp->setStatus |= setDB;

				break;

			case defDoConnect:
				if (LOGICAL(VECTOR_ELT(list, n))[0] == TRUE)
					doConnect(mp);

				break;
			case defOther:
			case defNull:
			default:
				break;
			}
		}
        free(lptr);
		n++;
	}

	return (retval);

}

SEXP makeGPRObject(mongoParam * obj, int addFinalizer)
{
	SEXP ans, klass, ref;
	int p;

	p = 0;

	if (!obj) {
		PROBLEM "NULL MangoApi Interface handle being returned\n" ERROR;
	}
	PROTECT(klass = MAKE_CLASS("MongoHandle"));
	p++;
	PROTECT(ans = NEW(klass));
	p++;
	PROTECT(ref = R_MakeExternalPtr((void *)obj, Rf_install("MongoHandle"), R_NilValue));
	p++;

	if (addFinalizer == TRUE)
		R_RegisterCFinalizerEx(ref, R_finalizeGP, (Rboolean) TRUE);
	ans = SET_SLOT(ans, Rf_install("ref"), ref);

	UNPROTECT(p);

	return (ans);
}

void R_finalizeGP(SEXP Rgp)
{

	mongoParam *gp;

	gp = getGPRObject(Rgp);

	if (gp) {

		if (gp->MongoDB != NULL)
			free(gp->MongoDB);
		if (gp->strValue != NULL)
			free(gp->strValue);

		if (gp->status == MONGO_OK ) {
			mongo_disconnect(gp->conn);
			mongo_destroy(gp->conn);
			gp->status = MONGO_ERROR;
			free(gp->qs);
		}
		freeServerList(gp->Servers);
	}
	return;
}

mongoParam *getGPRObject(SEXP obj)
{

	mongoParam *retv;
	SEXP ref;

	if (TYPEOF(obj) != EXTPTRSXP) {
		ref = GET_SLOT(obj, Rf_install("ref"));
	} else
		ref = obj;

	retv = (mongoParam *) R_ExternalPtrAddr(ref);
	if (!retv) {
		PROBLEM "Stale mongoParam handle being passed to " ERROR;
	}

	if (R_ExternalPtrTag(ref) != Rf_install("MongoHandle")) {
		PROBLEM "External pointer with wrong tag passed to gp. Was %s", CHAR(PRINTNAME(R_ExternalPtrTag(ref))) ERROR;
	}

	return (retv);
}

SEXP R_initMongo(SEXP initList)
{

	SEXP retv;

	mongoParam *gp;

	gp = Calloc(1, mongoParam);
	gp->nServer = 1;
	gp->qs = Calloc(1, queryString);
	gp->qs->skip = gp->qs->limits = 0;
	gp->Servers = NULL;
	gp->setStatus = 0L;
	gp->MongoDB = NULL;
	gp->strValue = NULL;

	ProcessPara(gp, initList);

	retv = makeGPRObject(gp, TRUE);

	return (retv);
}
