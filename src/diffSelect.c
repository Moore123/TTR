/*
 * =====================================================================================
 *
 *       Filename:  diffSelect.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2011年06月18日 14时19分48秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Michael Moore (lyxmoo), lyxmoo@gmail.com
 *        Company:  who's who
 *
 * =====================================================================================
 */

#include "Rmongo.h"
#include "MongoQuery.h"
#include "lz4.h"
#include "lz4hc.h"
#include <string.h>

#define DNUM 12

int setDim(SEXP result, int nCol, int nRow)
{
	int retval = 0;
	SEXP dims;

	PROTECT(dims = allocVector(INTSXP, 2));

	INTEGER(dims)[0] = nCol;
	INTEGER(dims)[1] = nRow;

	setAttrib(result, R_DimSymbol, dims);
	UNPROTECT(1);

	return (retval);

}

int setDimName(SEXP result)
{
	int retval = 0;
	SEXP dimNames, dnames, rnames;
	int p;

	p = 0;

	PROTECT(dimNames = allocVector(VECSXP, 2));
	p++;
	PROTECT(dnames = allocVector(VECSXP, DNUM));
	p++;

	SET_VECTOR_ELT(dnames, 0, mkChar("seq"));	// sequence id
	SET_VECTOR_ELT(dnames, 1, mkChar("stk"));	// stock id
	SET_VECTOR_ELT(dnames, 2, mkChar("t"));	// timestamp
	SET_VECTOR_ELT(dnames, 3, mkChar("td"));	// time differ
	SET_VECTOR_ELT(dnames, 4, mkChar("amDiff"));	// am differ
	SET_VECTOR_ELT(dnames, 5, mkChar("SumBid"));	// sum bid amount
	SET_VECTOR_ELT(dnames, 6, mkChar("SumAsk"));	// sum ask amount
	SET_VECTOR_ELT(dnames, 7, mkChar("initB"));	// deal by bid
	SET_VECTOR_ELT(dnames, 8, mkChar("passO"));	// deal by ask
	SET_VECTOR_ELT(dnames, 9, mkChar("bInc"));	// bid Increase
	SET_VECTOR_ELT(dnames, 10, mkChar("oInc"));	// ask Increase
	SET_VECTOR_ELT(dnames, 11, mkChar("avP"));

	SET_VECTOR_ELT(dimNames, 0, dnames);
	rnames = R_NilValue;
	SET_VECTOR_ELT(dimNames, 1, rnames);
	setAttrib(result, R_DimNamesSymbol, dimNames);

	UNPROTECT(p);

	return (retval);

}

int setRTDimName(SEXP result)
{

	int retval = 0;
	SEXP dimNames, dnames, rnames;
	int p, n;

	p = n = 0;

	PROTECT(dimNames = allocVector(VECSXP, 2));
	p++;
	PROTECT(dnames = allocVector(VECSXP, EMPTYROWS));
	p++;

	SET_VECTOR_ELT(dnames, n++, mkChar("_id"));
	SET_VECTOR_ELT(dnames, n++, mkChar("stk"));
	SET_VECTOR_ELT(dnames, n++, mkChar("to"));
	SET_VECTOR_ELT(dnames, n++, mkChar("yc"));
	SET_VECTOR_ELT(dnames, n++, mkChar("nw"));
	SET_VECTOR_ELT(dnames, n++, mkChar("hi"));
	SET_VECTOR_ELT(dnames, n++, mkChar("lo"));
	SET_VECTOR_ELT(dnames, n++, mkChar("nwb"));
	SET_VECTOR_ELT(dnames, n++, mkChar("nwo"));
	SET_VECTOR_ELT(dnames, n++, mkChar("vl"));
	SET_VECTOR_ELT(dnames, n++, mkChar("am"));
	SET_VECTOR_ELT(dnames, n++, mkChar("b1v"));
	SET_VECTOR_ELT(dnames, n++, mkChar("b1p"));
	SET_VECTOR_ELT(dnames, n++, mkChar("b2v"));
	SET_VECTOR_ELT(dnames, n++, mkChar("b2p"));
	SET_VECTOR_ELT(dnames, n++, mkChar("b3v"));
	SET_VECTOR_ELT(dnames, n++, mkChar("b3p"));
	SET_VECTOR_ELT(dnames, n++, mkChar("b4v"));
	SET_VECTOR_ELT(dnames, n++, mkChar("b4p"));
	SET_VECTOR_ELT(dnames, n++, mkChar("b5v"));
	SET_VECTOR_ELT(dnames, n++, mkChar("b5p"));
	SET_VECTOR_ELT(dnames, n++, mkChar("o1v"));
	SET_VECTOR_ELT(dnames, n++, mkChar("o1p"));
	SET_VECTOR_ELT(dnames, n++, mkChar("o2v"));
	SET_VECTOR_ELT(dnames, n++, mkChar("o2p"));
	SET_VECTOR_ELT(dnames, n++, mkChar("o3v"));
	SET_VECTOR_ELT(dnames, n++, mkChar("o3p"));
	SET_VECTOR_ELT(dnames, n++, mkChar("o4v"));
	SET_VECTOR_ELT(dnames, n++, mkChar("o4p"));
	SET_VECTOR_ELT(dnames, n++, mkChar("o5v"));
	SET_VECTOR_ELT(dnames, n++, mkChar("o5p"));
	SET_VECTOR_ELT(dnames, n++, mkChar("d"));
	SET_VECTOR_ELT(dnames, n++, mkChar("t"));

	SET_VECTOR_ELT(dimNames, 0, dnames);
	rnames = R_NilValue;
	SET_VECTOR_ELT(dimNames, 1, rnames);
	setAttrib(result, R_DimNamesSymbol, dimNames);

	UNPROTECT(p);

	return (retval);
}

double diffTime(double curr, double next)
{
	double H, M, S;

	H = trunc(curr / 10000) - trunc(next / 10000);
	M = trunc((curr - (trunc(curr / 10000) * 10000)) / 100)
		 - trunc((next - (trunc(next / 10000) * 10000)) / 100);
	S = (curr - trunc(curr / 100) * 100)
		 - (next - trunc(next / 100) * 100);

	while (S < 0) {
		S += 60;
		M -= 1;
	}
	while (M < 0) {
		M += 60;
		H -= 1;
	}

	return (H * 3600 + M * 60 + S);

}

int diffValue(double *dst, double *ds1, double *ds2)
{

	int i, j, retval = 0;
	double avPrice, amDiff, vDiff;

	*(dst + 4) = amDiff = ds1[10] - ds2[10];
	vDiff = ds1[9] - ds2[9];

	if (vDiff != 0) {
		avPrice = amDiff / vDiff;

		for (i = 0; i <= 8; i += 2) {
			*(dst + 6) += ds1[21 + i] * ds1[22 + i];
			*(dst + 5) += ds1[11 + i] * ds1[12 + i];
		}

		for (i = 0; i <= 8; i += 2) {
			for (j = 0; j <= 8; j += 2) {
				if (ds1[22 + i] == ds2[22 + j]) {
					*(dst + 10) += (ds1[21 + i] - ds2[21 + j]) * ds2[22 + j];
					break;
				}
			}
		}

		for (i = 0; i <= 8; i += 2) {
			for (j = 0; j <= 8; j += 2) {
				if (ds1[12 + i] == ds2[12 + j]) {
					*(dst + 9) += (ds1[11 + i] - ds2[11 + j]) * ds2[12 + j];
					break;
				}
			}
		}
		*(dst + 11) = avPrice;

		if (avPrice >= *(ds2 + 22)) {;
			// deal up over ask scrab  
		} else if (avPrice <= *(ds2 + 12)) {;
			// deal down under bid list

		} else if ((avPrice > ds2[12]) && (avPrice < ds2[22])) {

			*(dst + 7) = (amDiff - vDiff * ds2[12])
				 / round((ds2[22] - ds2[12]) * 100) * ds2[22] * 100;

			*(dst + 8) = (amDiff - vDiff * ds2[22])
				 / round((ds2[12] - ds2[22]) * 100) * ds2[12] * 100;

		}

	}

	return (retval);

}

int dselect_cmd(MgPx_arglist * args, mongoParam * gp)
{
	int retval = 0;
	double *xptr;
	int i, j;
	int p = 0;

#if 0
	retval = queryRunning(args, gp);

	if (gp->nCols == 0) {
		PROTECT(gp->result = allocVector(REALSXP, DNUM));
		p++;
		xptr = NUMERIC_POINTER(gp->result);
		memset(xptr, NA_REAL, sizeof(double) * DNUM);
		UNPROTECT(p);

	} else if ((gp->setStatus & setCols) != 0) {
		PROTECT(gp->result = allocVector(REALSXP, DNUM * (gp->nRows - 1)));
		p++;
		xptr = NUMERIC_POINTER(gp->result);
		memset(xptr, 0x0, sizeof(double) * DNUM * (gp->nRows - 1));
		gp->qVcurr = gp->qVlist;
		i = 0;
		while (1) {
			if (gp->qVcurr == NULL)
				break;
			while (gp->qVcurr->next != NULL) {
				*(xptr) = ++i;
				*(xptr + 1) = *(gp->qVcurr->v + 1);
				*(xptr + 2) = *(gp->qVcurr->v + 32);
				*(xptr + 3) = diffTime(*(gp->qVcurr->v + 32), *(gp->qVcurr->next->v + 32));
				if (*(xptr + 3) > 30)
					*(xptr + 3) = NA_REAL;
				else {
					diffValue(xptr, gp->qVcurr->v, gp->qVcurr->next->v);
				}
				xptr = xptr + DNUM;
				gp->qVcurr = gp->qVcurr->next;
			}
			break;
		}
		UNPROTECT(p);
	}

	freeQvList(gp->qVlist);

	setDim(gp->result, DNUM, (gp->nRows < 2) ? 1 : (gp->nRows - 1));
	setDimName(gp->result);
#endif

	return (retval);
}

int zelect_cmd(MgPx_arglist * args, mongoParam * gp)
{

	int retval;
	double *xptr;
	SEXP dims, dimNames, rnames;

	int outSize, inSize;
	char *out_buff;

	int i;

	gridfs gfs[1];
	gridfile gfile[1];

	gp->setStatus |= setGridfs;
	gp->gridFileName = Calloc(STR_BUFF_LEN, char);

	retval = queryRunning(args, gp);
	gp->nCols = EMPTYROWS;

	if (gp->nCols == 0) {
		PROTECT(gp->result = allocVector(REALSXP, EMPTYROWS));
		xptr = NUMERIC_POINTER(gp->result);
		memset(xptr, NA_REAL, sizeof(double) * EMPTYROWS);
		gp->nRows = 1;
		inSize = sizeof(double) * EMPTYROWS;

	} else {
		PROTECT(gp->result = allocVector(REALSXP, gp->nCols * gp->nRows));
		xptr = NUMERIC_POINTER(gp->result);
		gp->qVcurr = gp->qVlist;

		while (gp->qVcurr != NULL) {
			memcpy(xptr, gp->qVcurr->v, sizeof(double) * gp->nCols);

			xptr = xptr + gp->nCols;
			gp->qVcurr = gp->qVcurr->next;
		}

		inSize = sizeof(double) * gp->nCols * gp->nRows;
	}

	out_buff = Calloc(CHUNKSIZE, char);

	xptr = NUMERIC_POINTER(gp->result);
	outSize = LZ4_compress((char *)xptr, out_buff + MiniHeadSize, inSize);

	memcpy(out_buff, &outSize, sizeof(int));	// zipped size
	memcpy(out_buff + sizeof(int), &inSize, sizeof(int));	// rawSize
	memcpy(out_buff + sizeof(int) * 2, &gp->nRows, sizeof(int));	// nRows

	gridfs_init(gp->conn, "hft", "fs", gfs);
	gridfs_store_buffer(gfs, out_buff, outSize + MiniHeadSize, gp->gridFileName, "lz4binary",GRIDFILE_DEFAULT);
	gridfs_destroy(gfs);

	free(out_buff);

	UNPROTECT(1);
	freeQvList(gp->qVlist);

	setDim(gp->result, gp->nCols, (gp->nRows < 2) ? 1 : (gp->nRows));
	setRTDimName(gp->result);

	if ((gp->setStatus & setGridfs) != 0) {
		free(gp->gridFileName);
	}

	return (retval);

}

int xelect_cmd(MgPx_arglist * args, mongoParam * gp)
{
	int retval;
	double *xptr;
	SEXP dims, dimNames, rnames;

	int outSize=0, inSize=0;
	char *out_buff;

	gridfs gfs[1];
	gridfile gfile[1];

	gp->setStatus |= setGridfs;
	gp->setStatus |= setNoBQ;

	gp->gridFileName = Calloc(STR_BUFF_LEN, char);

	retval = queryRunning(args, gp);

	out_buff = Calloc(CHUNKSIZE, char);
	gp->setStatus &= ~setgfile;
	gp->setStatus &= ~setgfs;

	retval = -1;

	gridfs_init(gp->conn, "hft", "fs", gfs);
	gp->setStatus |= setgfs;

	while (1) {

		if ((retval = gridfs_find_filename(gfs, gp->gridFileName, gfile)) != MONGO_OK ) {
			gp->setStatus &= ~setgfile;
			break;
		}
		gp->setStatus |= setgfile;

		if (!gridfile_exists(gfile)) {
			inSize = MiniHeadSize * DNUM;
			gp->setStatus &= ~setgfile;
			break;
		}

		retval = gridfile_read_buffer(gfile, out_buff, CHUNKSIZE);

		if (retval < sizeof(int) * 4) {
			inSize = MiniHeadSize * DNUM;
			break;
		}

		memcpy(&inSize, out_buff, sizeof(int));	// zippedSize
		if (inSize > CHUNKSIZE)
			break;

		memcpy(&outSize, out_buff + sizeof(int), sizeof(int));	// rawSize 
		memcpy(&gp->nRows, out_buff + sizeof(int) * 2, sizeof(int));	// nCols 

		gp->nCols = EMPTYROWS;

		break;

	}

	if ( (gp->setStatus & setgfile ) != 0 ) 
		gridfile_destroy(gfile);
	
	if ((gp->setStatus & setgfs) != 0 ) 
		gridfs_destroy(gfs);

	if (retval != (inSize + MiniHeadSize)) {

		PROTECT(gp->result = allocVector(REALSXP, EMPTYROWS));
		xptr = NUMERIC_POINTER(gp->result);
		memset(xptr, NA_REAL, sizeof(double) * EMPTYROWS);
		gp->nRows = 1;
		gp->nCols = EMPTYROWS;

	} else {

		PROTECT(gp->result = allocVector(REALSXP, gp->nCols * gp->nRows));
		xptr = NUMERIC_POINTER(gp->result);
		gp->qVcurr = gp->qVlist;

		retval = LZ4_uncompress_unknownOutputSize((char *)out_buff + MiniHeadSize, (char *)xptr, inSize, outSize);
		//retval = LZ4_uncompress( (char *)out_buff+MiniHeadSize,(char *)xptr,outSize);
		if (retval < 0)
			ndebug("uncompress error %d from zipped %d target %d gfile %s \n", retval, inSize, outSize, gp->gridFileName);

	}

	free(out_buff);

	setDim(gp->result, gp->nCols, (gp->nRows < 2) ? 1 : (gp->nRows));
	setRTDimName(gp->result);

	UNPROTECT(1);
	freeQvList(gp->qVlist);

	if ((gp->setStatus & setGridfs) != 0) {
		free(gp->gridFileName);
		gp->setStatus &= ~setGridfs;
	}

	gp->setStatus &= ~setgfile;
	gp->setStatus &= ~setgfs;

	return (retval);

}
