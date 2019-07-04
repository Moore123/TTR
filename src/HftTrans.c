#include "MongoQuery.h"
#include "Rmongo.h"

#define BRIEFs  9

int setBriefDimName(SEXP result)
{
	int retval = 0;
	SEXP dimNames, dnames, rnames;
	int p;

	p = 0;

	PROTECT(dimNames = allocVector(VECSXP, 2));
	p++;
	PROTECT(dnames = allocVector(VECSXP, BRIEFs));
	p++;

	SET_VECTOR_ELT(dnames, 0, mkChar("bid"));	// total bid;
	SET_VECTOR_ELT(dnames, 1, mkChar("ask"));	// total ask;
	SET_VECTOR_ELT(dnames, 2, mkChar("bin"));	// deal by buyin
	SET_VECTOR_ELT(dnames, 3, mkChar("bprc"));	// buyin price
	SET_VECTOR_ELT(dnames, 4, mkChar("sout"));	// deal by soldout
	SET_VECTOR_ELT(dnames, 5, mkChar("sprc"));	// soldout price
	SET_VECTOR_ELT(dnames, 6, mkChar("biv"));	// vary bid 
	SET_VECTOR_ELT(dnames, 7, mkChar("akv"));	// vary ask
	SET_VECTOR_ELT(dnames, 8, mkChar("masq"));	// masquade deal

	SET_VECTOR_ELT(dimNames, 1, dnames);
	rnames = R_NilValue;
	SET_VECTOR_ELT(dimNames, 0, rnames);

	setAttrib(result, R_DimNamesSymbol, dimNames);

	UNPROTECT(p);

	return (retval);

}

int setTransDimName(SEXP result)
{
	int retval = 0;
	SEXP dimNames, dnames, rnames;
	int p;

	p = 0;

	PROTECT(dimNames = allocVector(VECSXP, 2));
	p++;
	PROTECT(dnames = allocVector(VECSXP, CONVERSLEN));
	p++;

	SET_VECTOR_ELT(dnames, 0, mkChar("t"));
	SET_VECTOR_ELT(dnames, 1, mkChar("hi"));
	SET_VECTOR_ELT(dnames, 2, mkChar("lo"));
	SET_VECTOR_ELT(dnames, 3, mkChar("avp"));
	SET_VECTOR_ELT(dnames, 4, mkChar("tp"));
	SET_VECTOR_ELT(dnames, 5, mkChar("tam"));
	SET_VECTOR_ELT(dnames, 6, mkChar("bidp"));
	SET_VECTOR_ELT(dnames, 7, mkChar("askp"));
	SET_VECTOR_ELT(dnames, 8, mkChar("bid"));
	SET_VECTOR_ELT(dnames, 9, mkChar("ask"));
	SET_VECTOR_ELT(dnames, 10, mkChar("biInc"));
	SET_VECTOR_ELT(dnames, 11, mkChar("biDec"));
	SET_VECTOR_ELT(dnames, 12, mkChar("askInc"));
	SET_VECTOR_ELT(dnames, 13, mkChar("askDec"));
	SET_VECTOR_ELT(dnames, 14, mkChar("passBi"));
	SET_VECTOR_ELT(dnames, 15, mkChar("actBi"));
	SET_VECTOR_ELT(dnames, 16, mkChar("passAsk"));
	SET_VECTOR_ELT(dnames, 17, mkChar("actAsk"));
	SET_VECTOR_ELT(dnames, 18, mkChar("ff"));
	SET_VECTOR_ELT(dnames, 19, mkChar("vtype"));

	SET_VECTOR_ELT(dimNames, 0, dnames);
	rnames = R_NilValue;
	SET_VECTOR_ELT(dimNames, 1, rnames);
	setAttrib(result, R_DimNamesSymbol, dimNames);

	UNPROTECT(p);

	return (retval);

}

int briefDiff(double *dst, transOptions * tdst)
{
	int i, j, k, retval = 0;
	double avPrice, amDiff, vDiff, up_thres, dn_thres, *ds1, *ds2;

	//ndebug("source data %d x %d\n",tdst->dims[0],tdst->dims[1]);

	struct timeval timer;
	tic(&timer);

	for (k = 0; k < tdst->dims[1] - 1; k++) {

		ds1 = ((double *)tdst->data_ptr) + (tdst->dims[0] * k);
		ds2 = ds1 + tdst->dims[0];

		j = k;

		for (i = 0; i <= 8; i += 2) {
			*(dst + j) += *(ds1 + O1V + i) * *(ds1 + O1P + i);
			*(dst + tdst->dims[1] + j) += *(ds1 + B1V + i) * *(ds1 + B1P + i);
		}

		amDiff = *(ds1 + AM) - *(ds1 + AM + tdst->dims[0]);
		vDiff = *(ds1 + VL) - *(ds1 + VL + tdst->dims[0]);
		avPrice = amDiff / vDiff;

		// ndebug("%d v %6.2f  %6.2f p %3.4f vs b1p %3.4f  o1p %3.4f\n",k,amDiff,vDiff,avPrice,ds1[B1P],ds1[O1P]);

		*(dst + tdst->dims[1] * 3 + j) = ds1[O1P];
		*(dst + tdst->dims[1] * 5 + j) = ds1[B1P];

		up_thres = round((ds1[YC]) * 110) / 100;
		dn_thres = round((ds1[YC]) * 90) / 100;

		if ((avPrice >= ds2[B1P]) && (avPrice <= ds2[O1P])) {

			*(dst + tdst->dims[1] * 2 + j) =
				 (amDiff - vDiff * ds1[B1P]) / round((ds1[O1P] - ds1[B1P]) * 100) * ds1[O1P] * 100;

			*(dst + tdst->dims[1] * 4 + j) =
				 (amDiff - vDiff * ds1[O1P]) / round((ds1[B1P] - ds1[O1P]) * 100) * ds1[B1P] * 100;
		}

		while (1) {

			if (avPrice < ds2[B5P]) {
				*(dst + tdst->dims[1] * 2 + j) = amDiff;
				break;
			}

			if (avPrice < ds2[B4P]) {
				*(dst + tdst->dims[1] * 2 + j) =
					 ds2[B1V] * ds2[B1P] + ds2[B2V] * ds2[B2P] + ds2[B3V] * ds2[B3P] + ds2[B4V] * ds2[B4P];
				if (vDiff - ds2[B4V] - ds2[B3V] - ds2[B2V] - ds2[B1V] < 0)
					break;
			}

			if (avPrice < ds2[B3P]) {
				*(dst + tdst->dims[1] * 2 + j) = ds2[B1V] * ds2[B1P] + ds2[B2V] * ds2[B2P] + ds2[B3V] * ds2[B3P];
				if (vDiff - ds2[B3V] - ds2[B2V] - ds2[B1V] < 0)
					break;
			}

			if (avPrice < ds2[B2P]) {
				*(dst + tdst->dims[1] * 2 + j) = ds2[B1V] * ds2[B1P] + ds2[B2V] * ds2[B2P];
				if (vDiff - ds2[B2V] - ds2[B1V] < 0)
					break;
			}

			if (avPrice < ds2[B1P]) {

				if (vDiff > *(ds1 + B1V + tdst->dims[0])) {
					*(dst + tdst->dims[1] * 8 + j) = amDiff - *(ds1 + B1V + tdst->dims[0]) * *(ds1 + B1P + tdst->dims[0]);
					*(dst + tdst->dims[1] * 2 + j) = *(ds1 + B1V + tdst->dims[0]) * *(ds1 + B1P + tdst->dims[0]);
					break;
				} else
					*(dst + tdst->dims[1] * 2 + j) = amDiff;

			}

			break;
		}

		while (1) {

			if (avPrice > ds2[O5P]) {
				*(dst + tdst->dims[1] * 2 + j) = amDiff;
				break;
			}

			if (avPrice > ds2[O4P]) {
				*(dst + tdst->dims[1] * 2 + j) =
					 ds2[B1V] * ds2[B1P] + ds2[B2V] * ds2[B2P] + ds2[B3V] * ds2[B3P] + ds2[B4V] * ds2[B4P];
				if (vDiff - ds2[B3V] - ds2[B2V] - ds2[B1V] < 0)
					break;
			}

			if (avPrice > ds2[O3P]) {
				*(dst + tdst->dims[1] * 2 + j) = ds2[B1V] * ds2[B1P] + ds2[B2V] * ds2[B2P] + ds2[B3V] * ds2[B3P];
				if (vDiff - ds2[B3V] - ds2[B2V] - ds2[B1V] < 0)
					break;
			}

			if (avPrice > ds2[O2P]) {
				*(dst + tdst->dims[1] * 2 + j) = ds2[B1V] * ds2[B1P] + ds2[B2V] * ds2[B2P] + ds2[B3V] * ds2[B3P];
				if (vDiff - ds2[B3V] - ds2[B2V] - ds2[B1V] < 0)
					break;
			}

			if (avPrice > ds2[O1P]) {

				if (vDiff > *(ds1 + O1V + tdst->dims[0])) {
					*(dst + tdst->dims[1] * 8 + j) = amDiff - *(ds1 + O1V + tdst->dims[0]) * *(ds1 + O1P + tdst->dims[0]);
					*(dst + tdst->dims[1] * 4 + j) = *(ds1 + O1V + tdst->dims[0]) * *(ds1 + O1P + tdst->dims[0]);
				} else
					*(dst + tdst->dims[1] * 4 + j) = amDiff;

			}

			break;
		}

	}

	toc(&timer);

	return (retval);
}

int depTransDiff(double *dst, transOptions * tdst)
{
	int i, j, k, retval = 0;
	double avPrice, amDiff, vDiff, up_thres, dn_thres, *ds1, *ds2;

	//ndebug("source data %d x %d\n",tdst->dims[0],tdst->dims[1]);

	struct timeval timer;
	tic(&timer);

	for (k = 0; k < tdst->dims[1] - 1; k++) {

		ds1 = ((double *)tdst->data_ptr) + (tdst->dims[0] * k);

		j = k;

		for (i = 0; i <= 8; i += 2) {
			*(dst + j) += *(ds1 + O1V + i) * *(ds1 + O1P + i);
			*(dst + tdst->dims[1] + j) += *(ds1 + B1V + i) * *(ds1 + B1P + i);
		}

		amDiff = *(ds1 + AM) - *(ds1 + AM + tdst->dims[0]);
		vDiff = *(ds1 + VL) - *(ds1 + VL + tdst->dims[0]);
		avPrice = amDiff / vDiff;

		// ndebug("%d v %6.2f  %6.2f p %3.4f vs b1p %3.4f  o1p %3.4f\n",k,amDiff,vDiff,avPrice,ds1[B1P],ds1[O1P]);

		*(dst + tdst->dims[1] * 3 + j) = ds1[O1P];
		*(dst + tdst->dims[1] * 5 + j) = ds1[B1P];

		up_thres = round((ds1[YC]) * 110) / 100;
		dn_thres = round((ds1[YC]) * 90) / 100;

		// price state
		//{ if price +10%,  frag entind is buyin , gaint is sold } 
		if (avPrice == up_thres) {

			if (vDiff >= tdst->gaint)
				*(dst + tdst->dims[1] * 4 + j) = amDiff;
			else
				*(dst + tdst->dims[1] * 2 + j) = amDiff;
			continue;

		}
		//{ if price -10%,  frag entind is sold , gaint is buyin }
		//   setdown  and return ;
		if (avPrice == dn_thres) {

			if (vDiff >= tdst->gaint)
				*(dst + tdst->dims[1] * 2 + j) = amDiff;
			else
				*(dst + tdst->dims[1] * 4 + j) = amDiff;
			continue;
		}
#if 0
		// volumn state
		{
		any other pirce, gaint over bid is masq}
		{
		any other pirce, gaint blow ask is masq}
		setdown and return;
#endif

		if ((avPrice >= ds1[B1P]) && (avPrice <= ds1[O1P])) {

			*(dst + tdst->dims[1] * 2 + j) =
				 (amDiff - vDiff * ds1[B1P]) / round((ds1[O1P] - ds1[B1P]) * 100) * ds1[O1P] * 100;

			*(dst + tdst->dims[1] * 4 + j) =
				 (amDiff - vDiff * ds1[O1P]) / round((ds1[B1P] - ds1[O1P]) * 100) * ds1[B1P] * 100;
		}

		ds2 = ds1 + tdst->dims[0];

		while (1) {

			if (avPrice < ds2[B5P]) {
				*(dst + tdst->dims[1] * 2 + j) = amDiff;
				break;
			}

			if (avPrice < ds2[B4P]) {
				*(dst + tdst->dims[1] * 2 + j) =
					 ds2[B1V] * ds2[B1P] + ds2[B2V] * ds2[B2P] + ds2[B3V] * ds2[B3P] + ds2[B4V] * ds2[B4P];
				if (vDiff - ds2[B4V] - ds2[B3V] - ds2[B2V] - ds2[B1V] < 0)
					break;
			}

			if (avPrice < ds2[B3P]) {
				*(dst + tdst->dims[1] * 2 + j) = ds2[B1V] * ds2[B1P] + ds2[B2V] * ds2[B2P] + ds2[B3V] * ds2[B3P];
				if (vDiff - ds2[B3V] - ds2[B2V] - ds2[B1V] < 0)
					break;
			}

			if (avPrice < ds2[B2P]) {
				*(dst + tdst->dims[1] * 2 + j) = ds2[B1V] * ds2[B1P] + ds2[B2V] * ds2[B2P];
				if (vDiff - ds2[B2V] - ds2[B1V] < 0)
					break;
			}

			if (avPrice < ds1[B1P]) {

				if (vDiff > *(ds1 + B1V + tdst->dims[0])) {
					*(dst + tdst->dims[1] * 8 + j) = amDiff - *(ds1 + B1V + tdst->dims[0]) * *(ds1 + B1P + tdst->dims[0]);
					*(dst + tdst->dims[1] * 2 + j) = *(ds1 + B1V + tdst->dims[0]) * *(ds1 + B1P + tdst->dims[0]);
					break;
				} else
					*(dst + tdst->dims[1] * 2 + j) = amDiff;

			}

			break;
		}

		while (1) {

			if (avPrice > ds2[O5P]) {
				*(dst + tdst->dims[1] * 2 + j) = amDiff;
				break;
			}

			if (avPrice > ds2[O4P]) {
				*(dst + tdst->dims[1] * 2 + j) =
					 ds2[B1V] * ds2[B1P] + ds2[B2V] * ds2[B2P] + ds2[B3V] * ds2[B3P] + ds2[B4V] * ds2[B4P];
				if (vDiff - ds2[B3V] - ds2[B2V] - ds2[B1V] < 0)
					break;
			}

			if (avPrice > ds2[O3P]) {
				*(dst + tdst->dims[1] * 2 + j) = ds2[B1V] * ds2[B1P] + ds2[B2V] * ds2[B2P] + ds2[B3V] * ds2[B3P];
				if (vDiff - ds2[B3V] - ds2[B2V] - ds2[B1V] < 0)
					break;
			}

			if (avPrice > ds2[O2P]) {
				*(dst + tdst->dims[1] * 2 + j) = ds2[B1V] * ds2[B1P] + ds2[B2V] * ds2[B2P] + ds2[B3V] * ds2[B3P];
				if (vDiff - ds2[B3V] - ds2[B2V] - ds2[B1V] < 0)
					break;
			}

			if (avPrice > ds1[O1P]) {

				if (vDiff > *(ds1 + O1V + tdst->dims[0])) {
					*(dst + tdst->dims[1] * 8 + j) = amDiff - *(ds1 + O1V + tdst->dims[0]) * *(ds1 + O1P + tdst->dims[0]);
					*(dst + tdst->dims[1] * 4 + j) = *(ds1 + O1V + tdst->dims[0]) * *(ds1 + O1P + tdst->dims[0]);
				} else
					*(dst + tdst->dims[1] * 4 + j) = amDiff;

			}

			break;
		}

	}

	toc(&timer);

	return (retval);
}

SEXP R_hftTrans(SEXP initList)
{

	SEXP result, elmt;
	int i, cnt, p = 0, *ddims;

	double *xptr, *xmids;

	char *sptr, *vptr;
	transOptions *tropt;

	qV *sdata=NULL;
	TickRaw *tickraw;

	tropt = Calloc(1, transOptions);
	tropt->dims = Calloc(2, int);
	tropt->dims[0] = 0;
	tropt->dims[1] = 0;
	tropt->methods = 0;
	tropt->tims = Calloc(2, int);
	tropt->tims[0] = 1;
	tropt->tims[1] = 1;

	result = getAttrib(initList, R_NamesSymbol);
	vptr = NULL;

	for (i = 0; i < LENGTH(result); i++) {

		sptr = strdup(CHAR(STRING_ELT(result, i)));
		elmt = VECTOR_ELT(initList, i);

		if (strcmp(sptr, "data") == 0) {
			ddims = INTEGER(GET_DIM(elmt));
			xptr = tropt->data_ptr = NUMERIC_POINTER(elmt);

			tropt->dims[0] = ddims[0];
			tropt->dims[1] = ddims[1];

			sdata = Calloc(1, qV);
			sdata->nRows = ddims[0];
			sdata->nCols = ddims[1];
			sdata->v = REAL(elmt);

		}

		if (strcmp(sptr, "method") == 0) {
			vptr = strdup(CHAR(STRING_ELT(elmt, 0)));

			if (strcmp(vptr, "brief") == 0)
				tropt->methods = METHOD_BRIEF;
			if (strcmp(vptr, "detail") == 0)
				tropt->methods = METHOD_DEPTRANS;
			if (strcmp(vptr, "tick") == 0)
				tropt->methods = METHOD_TICKTR;

			free(vptr);
		}

		if (strcmp(sptr, "tims") == 0) {

			tropt->tims[0] = REAL(elmt)[0];
			tropt->tims[1] = REAL(elmt)[1];

			//ndebug("tims for output %d %d \n", (int)tropt->tims[0], (int)tropt->tims[1]);
		}

		if (strcmp(sptr, "big") == 0) {
			tropt->gaint = REAL(elmt)[0];
			ndebug("gaint for huge bid-ask %ld \n", (long)tropt->gaint);

		}

		free(sptr);

	}

	if ( sdata != NULL)
	switch (tropt->methods) {

	case METHOD_BRIEF:

		PROTECT(result = allocVector(REALSXP, tropt->dims[1] * BRIEFs));

		setDim(result, tropt->dims[1], BRIEFs);
		setBriefDimName(result);
		xmids = NUMERIC_POINTER(result);
		for (i = 0; i < tropt->dims[1] * BRIEFs; i++)
			xmids[i] = 0.0f;
		briefDiff(xmids, tropt);
		UNPROTECT(1);

		break;

	case METHOD_DEPTRANS:

		p = 0;
		tickraw = tick2detail(sdata, 0);

		if (tickraw != NULL) {
			PROTECT(result = allocVector(REALSXP, tickraw->nRows * tickraw->nCols));
			p++;
			for (cnt = 0; cnt < tickraw->nRows * tickraw->nCols; cnt++)
				REAL(result)[cnt] = tickraw->v[cnt];
			setDim(result, tickraw->nCols, tickraw->nRows);
			setTransDimName(result);

			free(tickraw->v);
			free(tickraw);

			UNPROTECT(p);

	}	

		break;

	case METHOD_TICKTR:
		p = 0;
		tickraw = tick2raw(sdata, 0);

		if (tickraw != NULL) {
			PROTECT(result = allocVector(REALSXP, tickraw->nRows * tickraw->nCols));
			p++;
			for (cnt = 0; cnt < tickraw->nRows * tickraw->nCols; cnt++)
				REAL(result)[cnt] = tickraw->v[cnt];
			setDim(result, tickraw->nCols, tickraw->nRows);
			setTransDimName(result);

			free(tickraw->v);
			free(tickraw);

			UNPROTECT(p);

		}
		break;
	default:
		ndebug("methods is unknown!\n");
		break;
	}

	if (tropt->dims != NULL)
		free(tropt->dims);
	free(tropt);
	return (result);
}


SEXP R_hftTrans2(SEXP initList)
{

	SEXP result, elmt;
	int i, n, found, p = 0, *ddims;
	double *xrst;
	float x, y;;

	char *sptr;
	qV *sdata;
	TickRaw *tickraw;

	result = getAttrib(initList, R_NamesSymbol);

	for (i = 0; i < LENGTH(result); i++) {

		sptr = strdup(CHAR(STRING_ELT(result, i)));

		elmt = VECTOR_ELT(initList, i);
		if (strcmp(sptr, "data") == 0) {
			ddims = INTEGER(GET_DIM(elmt));
			
			sdata = Calloc(1, qV);
			sdata->nRows = ddims[0];
			sdata->nCols = ddims[1];
			sdata->v = REAL(elmt);

		}
		if (strcmp(sptr, "method") == 0) {
			ndebug("methods is %s\n", CHAR(asChar(elmt)));
		}
		if (strcmp(sptr, "dims") == 0) {

			x = REAL(elmt)[0];
			y = REAL(elmt)[1];

			ndebug("dims for output %d %d \n", (int)x, (int)y);
		}

	}

	p = 0;
	tickraw = tick2raw(sdata, 0);

	if (tickraw != NULL) {
		PROTECT(result = allocVector(REALSXP, tickraw->nRows * tickraw->nCols));
		p++;
		for (n = 0; n < tickraw->nRows * tickraw->nCols; n++)
			REAL(result)[n] = tickraw->v[n];
		setDim(result, tickraw->nCols, tickraw->nRows);
		setTransDimName(result);

		free(tickraw->v);
		free(tickraw);

		UNPROTECT(p);

	}

	return (result);

}
