#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>
#include <dirent.h>

#include "R.h"
#include "Rdefines.h"
#include "Rinternals.h"

#include "R_ext/Arith.h"
#include "R_ext/Error.h"
#include "Rversion.h"

#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>

#include "common.h"

ProfitStateM PftStM[] = {
	{BeTen, BeCont, "每10股", PftBase}
	,
	{BeCont, BeCont, "	", PftNothing}
	,
	{BeContPrice, BeCont | BeNum, "配股价", PftPeiguP}
	,
	{BeCont, BeContPrice | BeNum, "配股", PftPeigu}
	,
	{BeCont, BeNum, "送", PftHongGu}
	,
	{BeCont, BeNum, "红利", PftHongli}
	,
	{BeCont, BeNum, "派息", PftHongli}
	,
	{BeCont, BeNum, "转增", PftHongGu}
	,
	{BeCont, BeNum, "增发", PftNothing}
	,
	{BeNum, BeCont, "元", PftNothing}
	,
	{BeNum, BeCont, "股", PftNothing}
	,
	{BeCont, BeCont, " ", PftNothing}
	,
	{BeCont, BeCont, "；", PftNothing}
	,
	{BeNum, BeCont, "股追送股票", PftNothing}
};

typedef struct Comps_Profit {
	TProfit *TPHead;
	TProfit *TPTail;

} CProfit;

typedef struct GLOBALPARAM {
//List4Files *FilesListH,*FilesListT;

	char *File;
	char *ProfitFile;
	char *RefFile;

	unsigned int startdate;
	unsigned int enddate;
	int nDays;
	int CountN;

	unsigned int topN;

	unsigned int type;

} GlobalParam;

GlobalParam *GP;

typedef struct DAYCHAINS {
	dzh ohlcVal;
	struct DAYCHAINS *prev, *next;

} DayChains;

size_t Xfread(void *PTR, size_t size, size_t nmemb, FILE * fhandle)
{

	int sLong = sizeof(long);
	int retval = 0;
	dzh *ptr = (dzh *) PTR;

	if (sLong == 4)
		return fread(ptr, size, nmemb, fhandle);
	while (sLong == 8) {
		retval += fread((void *)&ptr->date, 4, 1, fhandle);

		if (retval <= 0)
			break;
		retval += fread((void *)&ptr->open, 4, 1, fhandle);
		if (retval <= 0)
			break;
		retval += fread((void *)&ptr->high, 4, 1, fhandle);
		if (retval <= 0)
			break;
		retval += fread((void *)&ptr->low, 4, 1, fhandle);
		if (retval <= 0)
			break;
		retval += fread((void *)&ptr->endp, 4, 1, fhandle);
		if (retval <= 0)
			break;
		retval += fread((void *)&ptr->volumn, 4, 1, fhandle);
		if (retval <= 0)
			break;
		retval += fread((void *)&ptr->amount, 4, 1, fhandle);
		if (retval <= 0)
			break;
		retval += fseek(fhandle, 0x4, SEEK_CUR);
		break;
	}

	return (retval);

}

int isFinishedRead(dzh * rec, GlobalParam * gp)
{
	int retval = TRUE;

	if (gp->type & OPT_SETEND) {

		if (rec->date < gp->enddate) {

			retval = FALSE;
		}

	} else if (gp->type & OPT_DATES) {
		if (gp->CountN-- > 0)
			retval = FALSE;

	}

	return (retval);

}

ConfOpt OptSet[] = {
	{"Files", 0, OPT_FILES}
	,
	{"Profit", 0, OPT_PROFIT}
	,
	{"Reference", 0, OPT_REFERENCE}
	,
	//{"Between", OPT_OPTIONS, OPT_},
	{"StartDate", 0, OPT_SETSTART}
	,
	{"EndDate", 0, OPT_SETEND}
	,
	{"DatesN", 0, OPT_DATES}
	,

	{"RateN", 0, 0}
	,
	{"AmountN", 0, 0}
	,
	{NULL, -1, -1}
};

int ReadingParamSet(SEXP listConf, GlobalParam * gp)
{

	int i = 0, n = 0;
	SEXP pname, values;
	int found = FALSE;

	int retval = 0;

	pname = getAttrib(listConf, R_NamesSymbol);

	while (1) {
		if (n >= LENGTH(listConf))
			break;
		values = VECTOR_ELT(listConf, n);
		found = FALSE;
		i = 0;

		while ((OptSet[i].SetString != NULL) && (found != TRUE)) {

			if (strncmp(OptSet[i].SetString, CHAR(STRING_ELT(pname, n)), strlen(CHAR(STRING_ELT(pname, n)))) != 0) {
				i++;
				continue;
			}

			found = TRUE;

			switch (OptSet[i].opt) {

			case OPT_FILES:
				gp->type |= OPT_FILES;
				gp->File = Calloc(strlen(CHAR(STRING_ELT(values, 0))) + 1, char);
				sprintf(gp->File, "%s", CHAR(STRING_ELT(values, 0)));
				break;

			case OPT_PROFIT:
				gp->type |= OPT_PROFIT;
				gp->ProfitFile = Calloc(strlen(CHAR(STRING_ELT(values, 0))) + 1, char);
				sprintf(gp->ProfitFile, "%s", CHAR(STRING_ELT(values, 0)));
				break;

			case OPT_REFERENCE:
				gp->RefFile = Calloc(strlen(CHAR(STRING_ELT(values, 0))) + 1, char);
				sprintf(gp->RefFile, "%s", CHAR(STRING_ELT(values, 0)));
				gp->type |= OPT_REFERENCE;
				break;

			case OPT_SETSTART:
				gp->type |= OPT_SETSTART;
				gp->startdate = (unsigned int)REAL(VECTOR_ELT(listConf, n))[0];
				break;

			case OPT_SETEND:
				gp->type |= OPT_SETEND;
				gp->enddate = (unsigned int)REAL(VECTOR_ELT(listConf, n))[0];
				break;

			case OPT_DATES:
				gp->type |= OPT_DATES;
				gp->nDays = (int)REAL(VECTOR_ELT(listConf, n))[0];
				break;

			case OPT_RATE:
			case OPT_AMOUNT:
				break;

			default:
				retval++;
				break;
			}

		}

		n++;
	}

	return (retval);

}

int ReLocateFile(FILE * fp, GlobalParam * gp)
{
	int retval = 0;

	dzh *dbuff;

	dbuff = Calloc(1, dzh);

	if ((gp->type & OPT_SETSTART) || (gp->type & OPT_SETEND)) {

		if (gp->type & OPT_SETSTART) {
			while (Xfread(dbuff, 0x20, 1, fp) > 0) {

				if (dbuff->date >= gp->startdate) {
					fseek(fp, (-1L) * 0x20, SEEK_CUR);
					break;
				}
			}
		} else if ((gp->type & OPT_SETEND) && (gp->type & OPT_DATES)) {
			while (Xfread(dbuff, 0x20, 1, fp) > 0) {
				if (dbuff->date >= gp->enddate) {
					fseek(fp, (-1L) * (gp->nDays) * 0x20, SEEK_CUR);
					break;
				}
			}
		}

	} else if (gp->type & OPT_DATES) {

		fseek(fp, (long)(0L - ((gp->nDays) * 0x20)), SEEK_END);

		gp->CountN = gp->nDays;

	}

	free(dbuff);
	return (retval);

}

int AssignTP(TProfit * TP, int assign, float tmpvalue)
{

	int retval = 0;

	switch (assign) {
	case PftHongGu:
		TP->type |= PftHongGu;
		TP->div += tmpvalue;

		break;

	case PftPeigu:
		TP->base = PftBaseMent;
		TP->type |= PftPeigu;
		TP->rate = tmpvalue;
		break;

	case PftPeiguP:
		TP->type |= PftPeiguP;
		TP->price = tmpvalue;
		break;

	case PftHongli:
		TP->type |= PftHongli;
		TP->sub = tmpvalue;
		break;

	case PftBase:
		TP->base = PftBaseMent;
		TP->div = 0.0;
		TP->sub = 0.0;
		TP->price = 0.0;
		TP->rate = 0.0;
		TP->type = PftNothing;

		break;

	case PftNothing:
		break;

	default:
		break;

	}
	return (retval);

}

int ParseProfit(TProfit * TP, char *profitstr, Dump_Data * ddmp)
{

	unsigned int CurrentState;

	unsigned int cchar = 0;

	int i, j, retval = 0;
	float tmpvalue = 0.0;
	char *tempbuff;

	CurrentState = BeInit;

	TP->date = ddmp->date;

	TP->div = 0.0;

	cchar = 0;

	while ((CurrentState != BeEnd) && (cchar < strlen(profitstr))) {

		for (j = 0; j < DIM(PftStM); j++) {

			if (strncmp(PftStM[j].smname, profitstr + cchar, strlen(PftStM[j].smname)) == 0) {

				cchar += strlen(PftStM[j].smname);
				CurrentState = PftStM[j].nextstate;

				if (PftStM[j].nextstate & BeNum) {

					i = 0;
					while (isdigit(*(profitstr + cchar + i)) || *(profitstr + cchar + i) == '.') {
						i++;
					}

					tempbuff = Calloc(i + 1, char);
					memcpy(tempbuff, profitstr + cchar, i);
					sscanf(tempbuff, "%f", &tmpvalue);
					cchar += i;

					AssignTP(TP, PftStM[j].smassign, tmpvalue);

					tmpvalue = 0.0;

				}

				if (PftStM[j].smassign & PftBase) {
					AssignTP(TP, PftStM[j].smassign, tmpvalue);
				}
				j = 0;

			}
		}

		if (j == DIM(PftStM)) {
			cchar++;
		}

	}
	return (retval);
}

int GetProfitShare(const char *indexname, CProfit * CP)
{

	char *descbuffer;

	Dump_Data PP4para;

	FILE *fp;
	int i, ii, retval = 0;
	int year, month, date;

	TProfit *TProfitHead, *TProfitPtr, *TProfitTemp;

	if ((fp = fopen(indexname, "r")) == NULL) {
		retval = -1;
		return retval;
	}

	if ((descbuffer = Calloc(MAX_STR_LEN, char)) == NULL) {

		retval = -1;
		return (retval);
	}

	TProfitPtr = TProfitHead = CP->TPHead;

	TProfitTemp = NULL;

	do {
		memset(descbuffer, 0x0, MAX_STR_LEN);

		if (fgets(descbuffer, MAX_STR_LEN, fp) != NULL) {

			for (i = 0; i < MAX_STR_LEN; i++) {
				ii = i;
				while (ISDIGIT(*(descbuffer + ii)) || *(descbuffer + ii) == '-') {
					ii++;
				}

				if ((ii - i) == 10) {
					sscanf(descbuffer + i, "%4d-%2d-%2d", &year, &month, &date);
					PP4para.date = year * 10000 + month * 100 + date;

					retval = TRUE;
					break;
				} else {
					retval = FAIL;
					break;
				}

			}

		} else {
			retval = FAIL;
			break;
		}

		memset(descbuffer, 0x0, MAX_STR_LEN);

		if (fgets(descbuffer, MAX_STR_LEN, fp) == NULL) {
			retval = FAIL;
			break;
		}

		if (TProfitTemp == NULL) {
			ParseProfit(TProfitPtr, descbuffer, &PP4para);
			TProfitTemp = TProfitPtr;
		} else {
			TProfitTemp = Calloc(1, TProfit);
			ParseProfit(TProfitTemp, descbuffer, &PP4para);

			if ((TProfitTemp->type & (0xf)) != 0) {
				TProfitTemp->prev = TProfitPtr;
				TProfitTemp->next = TProfitPtr->next;
				TProfitPtr->next->prev = TProfitTemp;
				TProfitPtr->next = TProfitTemp;
				TProfitPtr = TProfitPtr->next;

			}
		}

	} while (retval == TRUE);

	fclose(fp);
	free(descbuffer);

	return retval;

}

int freeProfit(CProfit * thead)
{
	TProfit *tptr;

	tptr = thead->TPHead->next;

	while (tptr->next != thead->TPHead) {
		tptr = tptr->next;
		free(tptr->prev);
	}

	free(thead->TPHead);
	free(thead);

	return (0);

}

int ReCalcPrice4(CProfit * CP, double *xrst, int xmax)
{

	int i, jmax;
	int retval = 0;

	jmax = xmax;

	TProfit *TProfitHead, *TProfitTail, *TProfitPtr;

	TProfitHead = CP->TPHead;
	TProfitPtr = TProfitTail = CP->TPHead;

	do {

		if ((unsigned int)xrst[jmax - 1] < (unsigned int)TProfitPtr->date) {

			i = jmax - 1;

			do {

				if ((TProfitPtr->type & PftHongli) != 0) {

					xrst[xmax * 2 + i] = round(xrst[xmax * 2 + i] - ((TProfitPtr->sub) * 10));
					xrst[xmax * 3 + i] = round(xrst[xmax * 3 + i] - ((TProfitPtr->sub) * 10));
					xrst[xmax * 4 + i] = round(xrst[xmax * 4 + i] - ((TProfitPtr->sub) * 10));
					xrst[xmax + i] = round(xrst[xmax + i] - ((TProfitPtr->sub) * 10));
				}

				if ((TProfitPtr->type & PftHongGu) != 0) {

					xrst[xmax * 2 + i] = round(xrst[xmax * 2 + i] / ((TProfitPtr->div + PftBaseMent) / PftBaseMent));
					xrst[xmax * 3 + i] = round(xrst[xmax * 3 + i] / ((TProfitPtr->div + PftBaseMent) / PftBaseMent));
					xrst[xmax * 4 + i] = round(xrst[xmax * 4 + i] / ((TProfitPtr->div + PftBaseMent) / PftBaseMent));
					xrst[xmax + i] = round(xrst[xmax + i] / ((TProfitPtr->div + PftBaseMent) / PftBaseMent));
				}

				if ((TProfitPtr->type & PftPeigu) != 0) {

					xrst[xmax * 2 + i] =
						 round((xrst[xmax * 2 + i] + (TProfitPtr->price) * (TProfitPtr->rate) * 10) / (TProfitPtr->rate +
																																 PftBaseMent));
					xrst[xmax * 3 + i] =
						 round((xrst[xmax * 3 + i] + (TProfitPtr->price) * (TProfitPtr->rate)) * 10 / (TProfitPtr->rate +
																																 PftBaseMent));
					xrst[xmax * 4 + i] =
						 round((xrst[xmax * 4 + i] + (TProfitPtr->price) * (TProfitPtr->rate)) * 10 / (TProfitPtr->rate +
																																 PftBaseMent));
					xrst[xmax + i] =
						 round((xrst[xmax + i] + (TProfitPtr->price) * (TProfitPtr->rate) * 10) / (TProfitPtr->rate +
																															PftBaseMent));
				}

				i--;

			} while (i >= 0);

			TProfitPtr = TProfitPtr->next;
		}

		jmax--;

	} while (jmax > 0);

	return (retval);

}

int freeDchains(DayChains * thead)
{
	DayChains *tptr;

	tptr = thead;

	while (tptr->next != NULL) {
		tptr = tptr->next;
		free(tptr->prev);
	}

	free(tptr);

	return (0);

}

int freeGP(GlobalParam * gp)
{
	if (gp->type & OPT_FILES)
		free(gp->File);

	if (gp->type & OPT_PROFIT)
		free(gp->ProfitFile);

	if (gp->type & OPT_REFERENCE)
		free(gp->RefFile);

	free(gp);
	return (0);
}

SEXP getnday(FILE * fhandle, double *FloatPtr, GlobalParam * gp)
{

	int i = 0;

	dzh *recorder = NULL;

	DayChains *DHead, *DPtr;
	SEXP result;

	DHead = DPtr = Calloc(1, DayChains);

	DHead->next = NULL;
	DHead->prev = NULL;
	recorder = Calloc(1, dzh);

	while (Xfread(recorder, 0x20, 1, fhandle) > 0) {

		memcpy(&DPtr->ohlcVal, recorder, sizeof(dzh));

		i++;

		if (isFinishedRead(recorder, gp) == TRUE) {
			break;
		}

		DPtr->next = Calloc(1, DayChains);
		DPtr->next->prev = DPtr;
		DPtr = DPtr->next;
		DPtr->next = NULL;

	}

	gp->CountN = gp->nDays = i;

	PROTECT(result = NEW_NUMERIC(gp->CountN * CNUM));
	FloatPtr = NUMERIC_POINTER(result);

	for (i = 0; i < gp->CountN; i++)
		FloatPtr[i] = 0.0;

	DPtr = DHead;

	i = 0;

	while ((DPtr != NULL) && (i < gp->nDays)) {

		//date
		FloatPtr[i] = DPtr->ohlcVal.date;
		//openor
		FloatPtr[gp->nDays + i] = DPtr->ohlcVal.open;
		//highor
		FloatPtr[gp->nDays * 2 + i] = DPtr->ohlcVal.high;
		//lowor
		FloatPtr[gp->nDays * 3 + i] = DPtr->ohlcVal.low;
		//endor
		FloatPtr[gp->nDays * 4 + i] = DPtr->ohlcVal.endp;
		//volumn
		FloatPtr[gp->nDays * 5 + i] = (float)((DPtr->ohlcVal.volumn));
		//amount
		FloatPtr[gp->nDays * 6 + i] = (DPtr->ohlcVal.amount);
		i++;
		if (isFinishedRead(&DPtr->ohlcVal, gp) == TRUE)
			break;
		DPtr = DPtr->next;
	}

	free(recorder);
	freeDchains(DHead);

	return (result);

}

/*

RawPath<-c("/home/stock/rawdata")
ProfitPath <- c("/home/stock/finance")
Files 
Profit
StartDate,
EndDate,
nDays
select_ttrc(Files="/home/stock/rawdata/sh600000.day",Profit="/home/stock/finance/sh600000",StartDate=20100101,EndDate=20100202)

*/

SEXP select_ttrc(SEXP ListConf)
{

	FILE *fhandle;

	CProfit *CP;

	struct stat dzh_fstat, tmp_fstat;

	int i, p = 0;

	SEXP result, dim;

	double *xrst;

	GlobalParam *gp;

	gp = Calloc(1, GlobalParam);
	gp->nDays = gp->startdate = gp->enddate = gp->type = gp->topN = gp->CountN = 0;

	gp->ProfitFile = gp->RefFile = gp->File = NULL;

	ReadingParamSet(ListConf, gp);

	while (1) {
		if (stat(gp->File, &dzh_fstat) != 0) {
			PROTECT(result = NEW_NUMERIC(1));
			p++;
			xrst = NUMERIC_POINTER(result);
			xrst[0] = NA_REAL;
			break;
		}

		if ((dzh_fstat.st_size % 0x20) != 0) {
			PROTECT(result = NEW_NUMERIC(1));
			p++;
			xrst = NUMERIC_POINTER(result);
			xrst[0] = NA_REAL;
			break;
		}

		if ((fhandle = fopen(gp->File, "rb")) != NULL) {
			ReLocateFile(fhandle, gp);
			result = getnday(fhandle, xrst, gp);
			p++;
			xrst = NUMERIC_POINTER(result);

			i = 0;

			if ((gp->type & OPT_PROFIT) && (stat(gp->ProfitFile, &tmp_fstat) == 0)) {

				CP = Calloc(1, CProfit);
				CP->TPHead = Calloc(1, TProfit);
				CP->TPTail = CP->TPHead;
				CP->TPHead->prev = CP->TPHead;
				CP->TPHead->next = CP->TPHead;

				GetProfitShare(gp->ProfitFile, CP);

				ReCalcPrice4(CP, xrst, gp->nDays);

				freeProfit(CP);
			}

			PROTECT(dim = allocVector(INTSXP, 2));
			p++;
			INTEGER(dim)[0] = gp->nDays;
			INTEGER(dim)[1] = CNUM;
			setAttrib(result, R_DimSymbol, dim);

			fclose(fhandle);
			break;

		}

	}

	SEXP dimNames, dnames, rnames;

	PROTECT(dimNames = allocVector(VECSXP, 2));
	p++;
	PROTECT(dnames = allocVector(VECSXP, CNUM));
	p++;

	SET_VECTOR_ELT(dnames, 0, mkChar("Date"));
	SET_VECTOR_ELT(dnames, 1, mkChar("Open"));
	SET_VECTOR_ELT(dnames, 2, mkChar("High"));
	SET_VECTOR_ELT(dnames, 3, mkChar("Low"));
	SET_VECTOR_ELT(dnames, 4, mkChar("Close"));
	SET_VECTOR_ELT(dnames, 5, mkChar("Volume"));
	SET_VECTOR_ELT(dnames, 6, mkChar("Amount"));

	SET_VECTOR_ELT(dimNames, 1, dnames);
	rnames = R_NilValue;
	SET_VECTOR_ELT(dimNames, 0, rnames);
	setAttrib(result, R_DimNamesSymbol, dimNames);

	UNPROTECT(p);
	freeGP(gp);
	return (result);

}

int getNday(FILE * fhandle, double *FloatPtr, int xmax)
{

	int i, retval = 0;

	dzh *recorder = NULL;

	recorder = Calloc(1, dzh);

	i = 0;

	while ((Xfread(recorder, 0x20, 1, fhandle) > 0) && (i < xmax)) {

		//date
		FloatPtr[i] = recorder->date;
		//openor
		FloatPtr[xmax + i] = recorder->open;
		//highor
		FloatPtr[xmax * 2 + i] = recorder->high;
		//lowor
		FloatPtr[xmax * 3 + i] = recorder->low;
		//endor
		FloatPtr[xmax * 4 + i] = recorder->endp;
		//volumn
		FloatPtr[xmax * 5 + i] = (float)((recorder->volumn));
		//amount
		FloatPtr[xmax * 6 + i] = (recorder->amount);
		i++;

	}
	free(recorder);

	return (retval);

}

SEXP get_ttrc(SEXP c_files, SEXP days, SEXP close)
{

	FILE *fhandle;

	dzh *recorder = NULL;

	CProfit *CP;

	char *prf_name, *stk_name;

	struct stat dzh_fstat, tmp_fstat;

	int i, xmax, p = 0;

	PROTECT(days = AS_NUMERIC(days));
	p++;
	PROTECT(close = AS_NUMERIC(close));
	p++;

	SEXP result, dim;

	double *xdays, *xrst;

	while (1) {
		if (stat(CHAR(STRING_ELT(c_files, 0)), &dzh_fstat) != 0) {
			PROTECT(result = NEW_NUMERIC(1));
			p++;
			xrst = NUMERIC_POINTER(result);
			xrst[0] = NA_REAL;
			break;
		}

		if ((dzh_fstat.st_size % 0x20) != 0) {
			PROTECT(result = NEW_NUMERIC(1));
			p++;
			xrst = NUMERIC_POINTER(result);
			xrst[0] = NA_REAL;
			break;
		}

		xdays = NUMERIC_POINTER(days);
		xmax =
			 (xdays[0] ==
			  0) ? (dzh_fstat.st_size / 0x20) : ((xdays[0] <
															  dzh_fstat.st_size / 0x20) ? (xdays[0]) : (dzh_fstat.st_size / 0x20));

		PROTECT(result = NEW_NUMERIC(xmax * CNUM));
		p++;
		xrst = NUMERIC_POINTER(result);

		for (i = 0; i < xmax; i++)
			xrst[i] = 0.0;

		recorder = Calloc(1, dzh);

		if (LENGTH(c_files) > 1) {
			prf_name = Calloc(STRING_LEN, char);
			stk_name = Calloc(MIN_STR_LEN, char);
			strncpy(stk_name, CHAR(STRING_ELT(c_files, 0)) + strlen(CHAR(STRING_ELT(c_files, 0))) - 12, 8);

			snprintf(prf_name, STRING_LEN, "%s%s", CHAR(STRING_ELT(c_files, 1)), stk_name);

			free(stk_name);

			if (stat(prf_name, &tmp_fstat) == 0) {
				CP = Calloc(1, CProfit);
				CP->TPHead = Calloc(1, TProfit);
				CP->TPTail = CP->TPHead;
				CP->TPHead->prev = CP->TPHead;
				CP->TPHead->next = CP->TPHead;
				GetProfitShare(prf_name, CP);

			}
		}

		if ((fhandle = fopen(CHAR(STRING_ELT(c_files, 0)), "rb")) != NULL) {
			fseek(fhandle, 0L - (xmax * 0x20), SEEK_END);
			i = 0;

			getNday(fhandle, xrst, xmax);

			if (stat(prf_name, &tmp_fstat) == 0) {

				ReCalcPrice4(CP, xrst, xmax);
				freeProfit(CP);
			}

			PROTECT(dim = allocVector(INTSXP, 2));
			p++;
			INTEGER(dim)[0] = xmax;
			INTEGER(dim)[1] = CNUM;
			setAttrib(result, R_DimSymbol, dim);

			//"Date","Open","High","Low","Close","Volume","Amount"

			SEXP dimNames, dnames, rnames;

			PROTECT(dimNames = allocVector(VECSXP, 2));
			PROTECT(dnames = allocVector(VECSXP, CNUM));

			SET_VECTOR_ELT(dnames, 0, mkChar("Date"));
			SET_VECTOR_ELT(dnames, 1, mkChar("Open"));
			SET_VECTOR_ELT(dnames, 2, mkChar("High"));
			SET_VECTOR_ELT(dnames, 3, mkChar("Low"));
			SET_VECTOR_ELT(dnames, 4, mkChar("Close"));
			SET_VECTOR_ELT(dnames, 5, mkChar("Volume"));
			SET_VECTOR_ELT(dnames, 6, mkChar("Amount"));

			SET_VECTOR_ELT(dimNames, 1, dnames);
			rnames = R_NilValue;
			SET_VECTOR_ELT(dimNames, 0, rnames);
			setAttrib(result, R_DimNamesSymbol, dimNames);
			UNPROTECT(2);

			Free(recorder);
			if (LENGTH(c_files) > 1) {
				free(prf_name);
			}
			fclose(fhandle);
			break;

		}
	}

	UNPROTECT(p);
	return (result);

}

int GetContentsFromFile0(char *path, const char *mstr, char *retstr)
{

	int i, retv = 0;

	FILE *fp;

	char *buff, *p;

	buff = Calloc(MAX_STR_LEN, char);

	if ((fp = fopen(path, "rb")) == NULL) {
		retv = -1;
	} else {

		while (fgets(buff, (sizeof(char) * MAX_STR_LEN), fp) != NULL) {

			if ((p = strstr(buff, mstr)) != NULL) {
				i = 0;

				while (p > 0 && *p != '>') {
					if (isdigit(*p) || (*p == '-') || (*p == '.'))
						++i;
					if ((*p == '-') && (*(p + 1) == '-')) {
						retv = -1;
					}

					--p;
				}

				if (i > 0) {
					memcpy(retstr, p + 1, i - 1);

				} else
					retv = -1;

				break;

			}
		}

	}

	fclose(fp);
	free(buff);

	return (retv);
}

int GetContentsFromFile(char *path, const char *mstr, char *retstr)
{

	int i, retv = 0;

	FILE *fp;

	char *buff, *p;

	buff = Calloc(MAX_STR_LEN, char);

	if ((fp = fopen(path, "rb")) == NULL) {
		retv = -1;
	} else {

		while (fgets(buff, (sizeof(char) * MAX_STR_LEN), fp) != NULL) {

			if ((p = strstr(buff, mstr)) != NULL) {
				i = 0;

		        if ( fgets(buff, (sizeof(char) * MAX_STR_LEN), fp) == NULL ) {
                    retv = -1;
                    break;
                }

                i = strlen(buff);

				if (i > 0) {
					memcpy(retstr, buff, i);

				} else
					retv = -1;

				break;

			}
		}

	}

	fclose(fp);
	free(buff);

	return (retv);
}


SEXP getFile(SEXP fName, SEXP rstring)
{

	SEXP result;
	int mslen, i, p = 0;
	char *buff, *vbuf, *fullpath;

	mslen = LENGTH(rstring);

	PROTECT(result = allocVector(VECSXP, mslen + 1));
	p++;

	buff = Calloc(STRING_LEN, char);
	vbuf = Calloc(STRING_LEN, char);
	fullpath = Calloc(MAX_STR_LEN, char);

	snprintf(fullpath, MAX_STR_LEN, "%s", CHAR(STRING_ELT(fName, 0)));
	i = strlen(fullpath);
	while (i > 0) {
		if (fullpath[i] == '/')
			break;
		i--;
	}
	snprintf(vbuf, 8, "%s", fullpath + i);

	SET_VECTOR_ELT(result, 0, ScalarString(mkChar(vbuf)));

	for (i = 0; i < mslen; i++) {

		memset(vbuf, 0x0, sizeof(char) * STRING_LEN);

		if (GetContentsFromFile(fullpath, CHAR(STRING_ELT(rstring, i)), vbuf) == 0) {

			SET_VECTOR_ELT(result, i + 1, ScalarReal(strtod(vbuf, NULL)));

		} else {

			SET_VECTOR_ELT(result, i + 1, ScalarReal(NA_REAL));

		}

	}

	free(fullpath);
	free(vbuf);
	free(buff);

	UNPROTECT(p);

	return (result);

}

SEXP getAll(SEXP pname, SEXP mstring, SEXP listdays)
{

	struct dirent **xdir;
	struct stat xstat;

	int mslen, xmax, i, n, p = 0;

	SEXP result, dim;

	char *buf, *fullpath;

	char *vbuf;

	if (stat(CHAR(STRING_ELT(pname, 0)), &xstat) == 0) {

		if ((xstat.st_mode & S_IFMT) == S_IFREG) {

			return (getFile(pname, mstring));
		}

	}

	n = scandir(CHAR(STRING_ELT(pname, 0)), &xdir, NULL, alphasort);
	xmax = n - 2;

	mslen = LENGTH(mstring);
	PROTECT(result = allocVector(VECSXP, xmax * (mslen + 1)));
	p++;
	buf = Calloc(16, char);
	vbuf = Calloc(STRING_LEN, char);
	fullpath = Calloc(MAX_STR_LEN, char);

	while (n--) {

		memset(buf, 0x0, sizeof(char) * 11);
		memset(vbuf, 0x0, sizeof(char) * STRING_LEN);
		memset(fullpath, 0x0, sizeof(char) * MAX_STR_LEN);

		if ((strcmp(xdir[n]->d_name, ".") == 0) || (strcmp(xdir[n]->d_name, "..") == 0))
			continue;

		(void)snprintf(buf, 10, "%s", xdir[n]->d_name);
		snprintf(fullpath, MAX_STR_LEN, "%s/%s", CHAR(STRING_ELT(pname, 0)), xdir[n]->d_name);

		SET_VECTOR_ELT(result, n - 2, ScalarString(mkChar(buf)));

		for (i = 0; i < mslen; i++) {

			memset(vbuf, 0x0, sizeof(char) * STRING_LEN);

			if (GetContentsFromFile(fullpath, CHAR(STRING_ELT(mstring, i)), vbuf) == 0) {

				SET_VECTOR_ELT(result, xmax * (i + 1) + n - 2, ScalarReal(strtod(vbuf, NULL)));

			} else {

				SET_VECTOR_ELT(result, xmax * (i + 1) + n - 2, ScalarReal(NA_REAL));

			}

		}

		free(xdir[n]);

	}

	free(fullpath);
	free(buf);
	free(vbuf);
	free(xdir);

	PROTECT(dim = allocVector(INTSXP, 2));
	p++;
	INTEGER(dim)[0] = xmax;
	INTEGER(dim)[1] = mslen + 1;
	setAttrib(result, R_DimSymbol, dim);

	UNPROTECT(p);
	return (result);

}

SEXP getLastFile(SEXP pathname, SEXP listdays)
{

	FILE *fhandle;

	double *xlist, *xrst;

	dzh *recorder;

	int i, j, listlen, xmax, p = 0;
	struct stat dzh_fstat;

	SEXP result;
	char *vbuf, *fullpath;

	listlen = LENGTH(listdays);

	xlist = NUMERIC_POINTER(listdays);

	xmax = xlist[listlen - 1];

	PROTECT(result = allocVector(VECSXP, listlen + 1));
	p++;

	fullpath = Calloc(MAX_STR_LEN, char);
	vbuf = Calloc(MAX_STR_LEN, char);
	recorder = Calloc(1, dzh);

	snprintf(fullpath, MAX_STR_LEN, "%s", CHAR(STRING_ELT(pathname, 0)));

	i = strlen(fullpath);
	while (i > 0) {
		if (fullpath[i] == '/')
			break;
		i--;
	};
	snprintf(vbuf, 8, "%s", fullpath + i);

	SET_VECTOR_ELT(result, 0, ScalarString(mkChar(vbuf)));

	xrst = Calloc(listlen, double);

	if (stat(CHAR(STRING_ELT(pathname, 0)), &dzh_fstat) == 0) {

		for (i = 0; i < listlen; i++) {
			if ((int)(dzh_fstat.st_size / 0x20) < xlist[i])
				xlist[i] = NA_REAL;
		}

		if ((fhandle = fopen(CHAR(STRING_ELT(pathname, 0)), "rb")) != NULL) {
			fseek(fhandle, 0L - ((xmax + 1) * 0x20), SEEK_END);
			i = 0;

			while ((Xfread(recorder, 0x20, 1, fhandle) > 0) && (i < xmax)) {

				for (j = 0; j < listlen - 1; j++) {
					if (j > (xmax - i + 1)) {
						xrst[j] += recorder->amount;
					}
				}
				i++;
			}

			for (i = 0; i < listlen; i++) {
				if (xlist[i] > 0) {
					xrst[i] /= xlist[i];
					SET_VECTOR_ELT(result, i + 1, ScalarReal(xrst[i]));
				} else
					SET_VECTOR_ELT(result, i + 1, ScalarReal(NA_REAL));
			}

			fclose(fhandle);

		}

	}

	free(recorder);
	free(fullpath);
	free(vbuf);
	free(xrst);

	UNPROTECT(p);

	return (result);
}

SEXP getLastAllV(SEXP pname, SEXP listdays)
{

	struct dirent **xdir;
	struct stat xstat;

	int mslen, xmax, dmax, i, j, n, p = 0;

	SEXP result, dim;

	char *gstr, *fullpath;

	double *duplist, *xlist, *xrst;

	FILE *fhandle;
	dzh *recorder = NULL;
	recorder = Calloc(1, dzh);

	if (stat(CHAR(STRING_ELT(pname, 0)), &xstat) == 0) {

		if ((xstat.st_mode & S_IFMT) == S_IFREG) {

			return (getLastFile(pname, listdays));
		}

	}

	n = scandir(CHAR(STRING_ELT(pname, 0)), &xdir, NULL, alphasort);
	xmax = n - 2;

	mslen = LENGTH(listdays);

	xlist = NUMERIC_POINTER(listdays);
	dmax = xlist[mslen - 1];

	duplist = Calloc(mslen, double);

	PROTECT(result = allocVector(VECSXP, xmax * (mslen + 1)));
	p++;

	gstr = Calloc(16, char);

	fullpath = Calloc(MAX_STR_LEN, char);
	xrst = Calloc(mslen, double);

	while (n--) {

		memset(gstr, 0x0, sizeof(char) * 16);
		memset(fullpath, 0x0, sizeof(char) * MAX_STR_LEN);
		memset(xrst, 0x0, sizeof(double) * mslen);
		for (i = 0; i < mslen; i++)
			duplist[i] = xlist[i];

		if ((strcmp(xdir[n]->d_name, ".") == 0) || (strcmp(xdir[n]->d_name, "..") == 0))
			continue;

		(void)snprintf(gstr, 9, "%s", xdir[n]->d_name);
		snprintf(fullpath, MAX_STR_LEN, "%s/%s", CHAR(STRING_ELT(pname, 0)), xdir[n]->d_name);

		SET_VECTOR_ELT(result, n - 2, ScalarString(mkChar(gstr)));

		if (stat(fullpath, &xstat) == 0) {

			for (i = 0; i < mslen; i++) {
				if ((int)(xstat.st_size / 0x20) < duplist[i])
					duplist[i] = 0;
			}

			if ((fhandle = fopen(fullpath, "rb")) != NULL) {
				fseek(fhandle, 0L - ((dmax + 1) * 0x20), SEEK_END);
				i = 0;

				while ((Xfread(recorder, 0x20, 1, fhandle) > 0) && (i <= dmax)) {

					for (j = 0; j < mslen; j++) {
						if (xlist[j] > (dmax - i)) {
							xrst[j] += (float)recorder->amount;
						}
					}
					i++;

				}

				for (i = 0; i < mslen; i++) {
					if (duplist[i] > 0) {
						xrst[i] /= duplist[i];
						SET_VECTOR_ELT(result, xmax * (i + 1) + n - 2, ScalarReal(xrst[i]));
					} else
						SET_VECTOR_ELT(result, xmax * (i + 1) + n - 2, ScalarReal(NA_REAL));
				}
				fclose(fhandle);
			}

		}

		free(xdir[n]);

	}

	free(fullpath);
	free(recorder);
	free(gstr);
	free(xrst);
	free(duplist);
	free(xdir);

	PROTECT(dim = allocVector(INTSXP, 2));
	p++;
	INTEGER(dim)[0] = xmax;
	INTEGER(dim)[1] = mslen + 1;
	setAttrib(result, R_DimSymbol, dim);

	UNPROTECT(p);
	return (result);

}

SEXP getLastAll(SEXP pname, SEXP listdays)
{

	struct dirent **xdir;
	struct stat xstat;

	int mslen, xmax, dmax, i, j, n, p = 0;

	SEXP result, dim;

	char *gstr, *fullpath;

	double *duplist, *xlist, *xrst;

	FILE *fhandle;
	dzh *recorder = NULL;
	recorder = Calloc(1, dzh);

	if (stat(CHAR(STRING_ELT(pname, 0)), &xstat) == 0) {

		if ((xstat.st_mode & S_IFMT) == S_IFREG) {

			return (getLastFile(pname, listdays));
		}

	}

	n = scandir(CHAR(STRING_ELT(pname, 0)), &xdir, NULL, alphasort);
	xmax = n - 2;

	mslen = LENGTH(listdays);

	xlist = NUMERIC_POINTER(listdays);
	dmax = xlist[mslen - 1];

	duplist = Calloc(mslen, double);

	PROTECT(result = allocVector(VECSXP, xmax * (mslen + 1)));
	p++;

	gstr = Calloc(16, char);

	fullpath = Calloc(MAX_STR_LEN, char);
	xrst = Calloc(mslen, double);

	while (n--) {

		memset(gstr, 0x0, sizeof(char) * 16);
		memset(fullpath, 0x0, sizeof(char) * MAX_STR_LEN);
		memset(xrst, 0x0, sizeof(double) * mslen);
		for (i = 0; i < mslen; i++)
			duplist[i] = xlist[i];

		if ((strcmp(xdir[n]->d_name, ".") == 0) || (strcmp(xdir[n]->d_name, "..") == 0))
			continue;

		(void)snprintf(gstr, 9, "%s", xdir[n]->d_name);
		snprintf(fullpath, MAX_STR_LEN, "%s/%s", CHAR(STRING_ELT(pname, 0)), xdir[n]->d_name);

		SET_VECTOR_ELT(result, n - 2, ScalarString(mkChar(gstr)));

		if (stat(fullpath, &xstat) == 0) {

			for (i = 0; i < mslen; i++) {
				if ((int)(xstat.st_size / 0x20) < duplist[i])
					duplist[i] = 0;
			}

			if ((fhandle = fopen(fullpath, "rb")) != NULL) {
				fseek(fhandle, 0L - ((dmax + 1) * 0x20), SEEK_END);
				i = 0;

				while ((Xfread(recorder, 0x20, 1, fhandle) > 0) && (i <= dmax)) {

					for (j = 0; j < mslen; j++) {
						if (xlist[j] > (dmax - i)) {
							xrst[j] += (float)recorder->volumn;
						}
					}
					i++;

				}

				for (i = 0; i < mslen; i++) {
					if (duplist[i] > 0) {
						xrst[i] /= duplist[i];
						SET_VECTOR_ELT(result, xmax * (i + 1) + n - 2, ScalarReal(xrst[i]));
					} else
						SET_VECTOR_ELT(result, xmax * (i + 1) + n - 2, ScalarReal(NA_REAL));
				}
				fclose(fhandle);
			}

		}

		free(xdir[n]);

	}

	free(fullpath);
	free(recorder);
	free(gstr);
	free(xrst);
	free(duplist);
	free(xdir);

	PROTECT(dim = allocVector(INTSXP, 2));
	p++;
	INTEGER(dim)[0] = xmax;
	INTEGER(dim)[1] = mslen + 1;
	setAttrib(result, R_DimSymbol, dim);

	UNPROTECT(p);
	return (result);

}

SEXP convolve2(SEXP ab)
{

	GlobalParam *gp;
	SEXP abc;

	gp = Calloc(1, GlobalParam);
	ReadingParamSet(ab, gp);
	return (abc);
}

SEXP convolve22(SEXP a, SEXP b)
{
	int i, j, na, nb, nab, p = 0;
	double *xa, *xb, *xab;
	SEXP ab;
	PROTECT(a = AS_NUMERIC(a));
	p++;
	PROTECT(b = AS_NUMERIC(b));
	p++;
	na = LENGTH(a);
	nb = LENGTH(b);
	nab = na + nb - 1;
	PROTECT(ab = NEW_NUMERIC(nab));
	p++;

	xa = NUMERIC_POINTER(a);
	xb = NUMERIC_POINTER(b);
	xab = NUMERIC_POINTER(ab);

	for (i = 0; i < nab; i++)
		xab[i] = 0.0;
	for (i = 0; i < na; i++)
		for (j = 0; j < nb; j++)
			xab[i + j] += xa[i] * xb[j];
	warning("programming by lyxmoo@msn.com");

	UNPROTECT(p);
	return (ab);
	//test_ab(StartDate=20100101,EndDate=20101121,DatesN=12)
}
