#include "Rmongo.h"
#include "MongoQuery.h"
#include <string.h>

#define isspace(q) ((q==' ')||(q=='\t'))

#define DROPDATABASE  "database"
#define DROPCOLLECTION "collection"

MgPx_command CMD[] = {

	{"select", "select command", NULL, select_cmd, 0}
	,									  //select from RAW mongoDB

	{"zelect", "select transform to zip gridfs", NULL, zelect_cmd, 0}
	,									  // from RAW mongDB transform lz4zipped gridfs

	{"xelect", "select from ziped gridfs", NULL, xelect_cmd, 0}
	,									  // select from lz4zipped gridfs

	{"delete", "delete command", NULL, delete_cmd, 0}
	,
	{"insert", "insert command", NULL, insert_cmd, 0}
	,
	{"drop", "drop command", NULL, drop_cmd, 0}
	,
	{NULL, NULL, NULL, NULL, 0}
};

int freeVdList(qVdms * list)
{

	int retval;
	qVdms *qVCur, *qVPtr;

	retval = 0;
	qVCur = list;

	while (qVCur != NULL) {
		qVPtr = qVCur;
		qVCur = qVCur->next;
		if (qVPtr->dm != NULL)
			free(qVPtr->dm);
		free(qVPtr);
	}

	return (retval);

}

int freeQvList(qV * list)
{

	int retval;
	qV *qVCur, *qVPtr;

	retval = 0;
	qVCur = list;

	while (qVCur != NULL) {
		qVPtr = qVCur;
		qVCur = qVCur->next;

		if (qVPtr->v != NULL)
			free(qVPtr->v);
		free(qVPtr);

	}

	return (retval);

}

int mybson_dump_raw(mongoParam * gp,int depth)
{

	bson_iterator i;
	
	bson_type t;

	const char *key;
	char *strBuf;

	int cnt, order;

	qVdms *vDhead, *vDcurr;
	double *dfBuff;

	cnt = order = 0;
	vDhead = vDcurr = NULL;

	dfBuff = Calloc(256, double);
	memset(dfBuff, 0x0, sizeof(double) * 256);
	
	bson_iterator_init(&i, mongo_cursor_bson( gp->cursor ));

	while (bson_iterator_next(&i)) {

		t = bson_iterator_type(&i);

		if (t == 0) break;

		key = bson_iterator_key(&i);

		if ((gp->setStatus & setCols) == 0) {

			if (vDhead == NULL) {
				vDhead = vDcurr = Calloc(1, qVdms);

				vDcurr->next = NULL;
			} else {
				vDcurr->next = Calloc(1, qVdms);
				vDcurr = vDcurr->next;
				vDcurr->next = NULL;
			}
			vDcurr->dm = strdup(key);
			cnt++;
		}

		switch (order) {
		case 0:
			dfBuff[order] = (double)gp->nRows;
			break;

		case 1:
			strBuf = strdup((char *)bson_iterator_string(&i));
			dfBuff[order] = atoll(strBuf + 2);
			break;

		default:
			dfBuff[order] = bson_iterator_double(&i);
			break;
		}

		order++;
	}

   if( order != 0 ) {

	if ((gp->setStatus & setCols) == 0) {
		gp->setStatus |= setCols;
		gp->qVlist = gp->qVcurr = Calloc(1, qV);
		gp->qVcurr->next = NULL;
		gp->qVcurr->v = NULL;

		PROTECT(gp->ColNames = allocVector(VECSXP, cnt));
		gp->nCols = cnt;
		vDcurr = vDhead;

        cnt = 0;
		while (vDcurr != NULL) {
			SET_VECTOR_ELT(gp->ColNames, cnt, mkChar(vDcurr->dm));
			vDcurr = vDcurr->next;
            cnt ++;
		}

		freeVdList(vDhead);
		UNPROTECT(1);

	} else {
		gp->qVcurr->next = Calloc(1, qV);
		gp->qVcurr = gp->qVcurr->next;
		gp->qVcurr->next = NULL;
		gp->qVcurr->v = NULL;
	}

	if (gp->nCols != 0) {
		gp->qVcurr->v = Calloc(gp->nCols, double);
		memcpy(gp->qVcurr->v, dfBuff, sizeof(double) * gp->nCols);
	}

   }

	free(dfBuff);
	return (cnt);
}

int dumpMongoFound(mongoParam * gp) {

	int retval = 0;

	int temp = 1;

	while (mongo_cursor_next(gp->cursor) == MONGO_OK ) {

        gp->nRows = temp;
		
		if ( retval = mybson_dump_raw(gp,0) < 0) break;
        
		if ((gp->qs->limits != 0) && (temp >= gp->qs->limits))
			break;
		else
			temp++;
	}

	mongo_cursor_destroy(gp->cursor);

	return (retval);
}

int queryRunning(MgPx_arglist * args, mongoParam * gp)
{

	int i, nChar;

/*  select all from tk2011_06_08 where stk=sh600000 limit 10 */
/*  zelect all from tk2011_06_08 where stk=sh600000 limit 10 */

/*  dselete all from tk2011_06_08 where stk=sh600000 */

	bson bbuf;

	char *tokPtr;
	char *nsSrc, *nsDest, *stmp;

	gp->strValue = Calloc(STR_BUFF_LEN, char);

	gp->qs->limits = 0;

	i = 1;
	while (i < args->argc) {

		if ((strncmp(args->argv[i], "all", 3) == 0)
			 || strncmp(args->argv[i], "*", 1) == 0) {

			bson_init(&bbuf);
			gp->setStatus &= (setConn | setDB | setGridfs | setNoBQ | setREMOVE);
			gp->setStatus |= setBsonInited;
			gp->nCols = 0;
			gp->qVlist = gp->qVcurr = NULL;

		} else if (strncmp(args->argv[i], "from", 4) == 0) {
			i++;
			gp->ns = Calloc(STR_BUFF_LEN, char);
			sprintf(gp->ns, "%s.%s", gp->MongoDB, args->argv[i]);

			if ((gp->setStatus & setGridfs) != 0)
				sprintf(gp->gridFileName, "%s/%s", gp->MongoDB, args->argv[i]);

			gp->setStatus |= setCollection;

		} else if (strncmp(args->argv[i], "where", 5) == 0) {
			i++;
			nChar = 0;
			stmp = args->argv[i];
			if ((tokPtr = strstr(stmp, ">=")) != NULL) {
				nChar = 2;
			} else if ((tokPtr = strstr(stmp, "<=")) != NULL) {
				nChar = 2;
			} else if ((tokPtr = strstr(stmp, "!=")) != NULL) {
				nChar = 2;
			} else if ((tokPtr = strstr(stmp, "=")) != NULL) {
				nChar = 1;

				nsSrc = strndup(args->argv[i], tokPtr - stmp);
				nsDest = strdup(tokPtr + nChar);

				if ((gp->setStatus & setGridfs) != 0)
					sprintf(gp->gridFileName + strlen(gp->gridFileName), "_%s.lz4", nsDest);

				if (((gp->setStatus & setBsonInited) ^ setBsonInited) == 0) {
					bson_append_string(&bbuf, nsSrc, nsDest);
				}

				free(nsSrc);
				free(nsDest);
			}

		} else if (strncmp(args->argv[i], "limit", 5) == 0) {
			i++;
			gp->qs->limits = atoi(args->argv[i]);

		} else if (strncmp(args->argv[i], "skip", 4) == 0) {
			i++;
			gp->qs->skip = atoi(args->argv[i]);

		}
		i++;
	}

	while (((gp->setStatus & MASKSET) ^ (setConn | setDB | setCollection | setBsonInited)) == 0) {

		if ( (gp->setStatus & setNoBQ) != 0 )
			break;

		bson_finish( &bbuf );

		if ((gp->setStatus & setREMOVE) != 0) {
			mongo_remove(gp->conn, gp->ns, &bbuf,NULL);
		} else {
		     if ( (gp->cursor = mongo_find(gp->conn, gp->ns, &bbuf, NULL, 
			 			  gp->qs->limits, gp->qs->skip, 0) ) != NULL )
			dumpMongoFound(gp);
		}

		break;
	}

	free(gp->strValue);
	gp->strValue = NULL;

	if ((gp->setStatus & setCollection) != 0)
		free(gp->ns);

	return (i);
}

int select_cmd(MgPx_arglist * args, mongoParam * gp)
{
	int retval;
	double *xptr;
	SEXP dims, dimNames, rnames;

	retval = queryRunning(args, gp);

	if (gp->nCols == 0) {
		PROTECT(gp->result = allocVector(REALSXP, EMPTYROWS));
		xptr = NUMERIC_POINTER(gp->result);
		memset(xptr, NA_REAL, sizeof(double) * EMPTYROWS);
		gp->nCols = EMPTYROWS;
		gp->nRows = 1;
		setDim(gp->result,gp->nCols,gp->nRows);
		UNPROTECT(1);

	} else if ((gp->setStatus & setCols) != 0) {

		PROTECT(gp->result = allocVector(REALSXP, gp->nCols * gp->nRows));
		xptr = NUMERIC_POINTER(gp->result);
		gp->qVcurr = gp->qVlist;

		while (gp->qVcurr != NULL) {
			memcpy(xptr, gp->qVcurr->v, sizeof(double) * gp->nCols);
			xptr = xptr + gp->nCols;
			gp->qVcurr = gp->qVcurr->next;
		}
		UNPROTECT(1);
	}

	freeQvList(gp->qVlist);

	if ((gp->setStatus & setCols) != 0) {

		PROTECT(dims = allocVector(INTSXP, 2));
		PROTECT(dimNames = allocVector(VECSXP, 2));
		INTEGER(dims)[0] = gp->nCols;
		INTEGER(dims)[1] = gp->nRows;

		setAttrib(gp->result, R_DimSymbol, dims);
     
		SET_VECTOR_ELT(dimNames, 0, gp->ColNames);
		rnames = R_NilValue;
		SET_VECTOR_ELT(dimNames, 1, rnames);

		setAttrib(gp->result, R_DimNamesSymbol, dimNames);

		UNPROTECT(2);

	}

	return (retval);

}

int insert_cmd(MgPx_arglist * args, mongoParam * gp)
{

	int i;
	ndebug("insert running  %d\n", gp->nServer);
	i = 0;
	while (i++ < args->argc)
		ndebug("%s \n", args->argv[i - 1]);

	return (i);
}

int drop_cmd(MgPx_arglist * args, mongoParam * gp)
{

	int i;
	bson b;
	char *ns;

/* bson_bool_t mongo_cmd_drop_db(mongo_connection * conn, const char * db)
*  bson_bool_t mongo_cmd_drop_collection(mongo_connection * conn, const char * db, const char * collection, bson * out)
*/

	i = 0;

	gp->strValue = Calloc(STR_BUFF_LEN, char);
	if (strncmp(args->argv[1], DROPDATABASE, 8) == 0) {
		if ((i = mongo_cmd_drop_db(gp->conn, args->argv[2])) == 0)
			sprintf(gp->strValue, "drop database %s success return %d\n", args->argv[2], i);
		else
			sprintf(gp->strValue, "drop database %s failed return %d\n", args->argv[2], i);
	}

	if (strncmp(args->argv[1], DROPCOLLECTION, 10) == 0) {
		ns = Calloc(STR_BUFF_LEN, char);
		sprintf(ns, "%s.%s", gp->MongoDB, args->argv[2]);
		if (((i = mongo_cmd_drop_collection(gp->conn, gp->MongoDB, args->argv[2], NULL)) == 0)
			 && mongo_find_one(gp->conn, ns, bson_shared_empty(), bson_shared_empty(), NULL)) {
			sprintf(gp->strValue, "drop collection %s.%s failed return %d\n", gp->MongoDB, args->argv[2], i);

		} else
			sprintf(gp->strValue, "drop collection %s.%s success return %d\n", gp->MongoDB, args->argv[2], i);
		free(ns);
	}

	PROTECT(gp->result = allocVector(STRSXP, 1));
	SET_STRING_ELT(gp->result, 0, mkChar(gp->strValue));
	UNPROTECT(1);
	free(gp->strValue);
	gp->strValue = NULL;

	return (i);
}

int delete_cmd(MgPx_arglist * args, mongoParam * gp)
{

	int retval = 0;

	gp->setStatus |= setREMOVE;

	retval = queryRunning(args, gp);

	PROTECT(gp->result = allocVector(STRSXP, 1));
	gp->strValue = Calloc(STR_BUFF_LEN, char);
	sprintf(gp->strValue, "done");
	SET_STRING_ELT(gp->result, 0, mkChar(gp->strValue));
	UNPROTECT(1);

	free(gp->strValue);
	gp->strValue = NULL;

	return (retval);
}

int update_cmd(MgPx_arglist * args, mongoParam * gp)
{

	int i;
	ndebug("drop running  %d\n", gp->nServer);
	i = 0;
	while (i++ < args->argc)
		ndebug("%s \n", args->argv[i - 1]);
	return (i);
}

inline char *findspace(char *src)
{

	register char *t1;
	register char *t2;

	t1 = strchr(src, ' ');
	t2 = strchr(src, '\t');
	if (t1 && t2) {
		if (t1 < t2)
			t2 = NULL;
		else
			t1 = NULL;
	}

	if (t1)
		return t1;
	return t2;
}

int argcount(const char *string)
{

	int ln;
	int cnt;
	int i;

	cnt = 1;
	ln = strlen(string);
	i = 0;

	while (i < ln) {
		if (isspace(string[i])) {
			while (isspace(string[i]))
				++i;
			if (string[i])
				++cnt;
		}
		++i;
	}

	return cnt;
}

MgPx_arglist *new_args(void)
{

	MgPx_arglist *res;

	res = Calloc(1, MgPx_arglist );
	res->argc = 0;
	res->argv = NULL;
	return (res);
}

void add_args(MgPx_arglist * arg, const char *elm)
{

	if (arg->argc) {
		arg->argv = (char **)realloc(arg->argv, (arg->argc + 1) * sizeof(char *));
		arg->argv[arg->argc++] = strdup(elm);
	} else {
		arg->argc = 1;
		arg->argv = (char **)malloc(sizeof(char *));
		arg->argv[0] = strdup(elm);
	}
}

MgPx_arglist *make_args(const char *string)
{

	MgPx_arglist *result;
	char *rightbound;
	char *word;
	char *crsr;
	int count;
	int pos;

	result = Calloc(1,MgPx_arglist);
	count = argcount(string);
	result->argc = count;

	result->argv = (char **)malloc(count * sizeof(char *));

	crsr = (char *)string;
	pos = 0;

	while ((rightbound = findspace(crsr))) {
		word = Calloc((rightbound - crsr + 3), char);
		memcpy(word, crsr, rightbound - crsr);
		word[rightbound - crsr] = 0;
		result->argv[pos++] = word;
		crsr = rightbound;
		while (isspace(*crsr))
			++crsr;
	}
	if (*crsr) {
		word = strdup(crsr);
		result->argv[pos++] = word;
	}

	return result;
}

void destroy_args(MgPx_arglist * lst)
{

	int i;
	for (i = 0; i < lst->argc; ++i)
		free(lst->argv[i]);

	free(lst->argv);
	free(lst);
}

SEXP R_MongoQuery(SEXP sexpObj, SEXP initList)
{

	int i, found;

	mongoParam *gp;
	MgPx_arglist *args;
	MgPx_command *xcmd;
	char *sptr;
    double *xptr;

	gp = getGPRObject(sexpObj);
	if (gp->qs != NULL) {
		if (gp->qs->key != NULL)
			free(gp->qs->key);
		free(gp->qs);
	}

	gp->qs = Calloc(1, queryString);

	/* ndebug("query string is %s\n",CHAR(STRING_ELT(initList,0))); 
	 *  if (xcmd->func) xcmd->func (realcmd); */

	sptr = strdup(CHAR(STRING_ELT(initList, 0)));

	args = make_args(sptr);

	found = -1;
	gp->setStatus &= (setConn | setDB);

	for (i = 0; CMD[i].name != NULL; i++) {

		if (strncmp(args->argv[0], CMD[i].name, strlen(CMD[i].name)) == 0) {
			found = i;
			xcmd = &CMD[i];
			break;
		}
		if (CMD[i].name == NULL)
			break;
	}

    free(sptr);

 while(1) {

	if (found != -1) {

		if (xcmd->func) {
			(found = (*xcmd->func) (args, gp));
            }

    } else {

        PROTECT( gp->result = allocVector(REALSXP, EMPTYROWS ) ) ;
         xptr = NUMERIC_POINTER(gp->result);
         memset(xptr, NA_REAL, sizeof(double)*EMPTYROWS);
        UNPROTECT(1);

    }

   break;

 }

    destroy_args(args);

	return (gp->result);

}
