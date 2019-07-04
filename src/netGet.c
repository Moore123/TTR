#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>
#include <dirent.h>

#include <R.h>
#include <Rinternals.h>

#include <R_ext/Arith.h>
#include <R_ext/Error.h>
#include <Rdefines.h>
#include "R_ext/Rdynload.h"

#define MAX_QUERY_LEN      1024*64

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{

	int retv = 0;

	snprintf(userp, MAX_QUERY_LEN, "%s", (char *)buffer);

	return (retv);
}

SEXP netGet(SEXP stk, SEXP url)
{

	CURL *curl;
	CURLcode res;

	char *buffer, *ubuff;

	SEXP result, dim;
	int xmax, i, p = 0;

	buffer = Calloc(MAX_QUERY_LEN, char);
	ubuff = Calloc(MAX_QUERY_LEN, char);

	xmax = LENGTH(stk);

	PROTECT(result = allocVector(STRSXP, xmax * 2));
	p++;

	curl = curl_easy_init();
	if (curl) {

		for (i = 0; i < xmax; i++) {
		    memset(ubuff,0x0,sizeof(char)*MAX_QUERY_LEN);	
			snprintf(ubuff, MAX_QUERY_LEN, CHAR(STRING_ELT(url, 0)), CHAR(STRING_ELT(stk, i)));

			curl_easy_setopt(curl, CURLOPT_URL, ubuff);

			curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

			if ((res = curl_easy_perform(curl)) == 0) {

				SET_STRING_ELT(result, i, mkChar(CHAR(STRING_ELT(stk, i))));
				SET_STRING_ELT(result, xmax + i, mkChar(buffer));
			} else {
				SET_STRING_ELT(result, i, NA_STRING);
				SET_STRING_ELT(result, xmax + i, mkChar(buffer));
			}

		}

		/* always cleanup */
		curl_easy_cleanup(curl);
		free(buffer);
		free(ubuff);

	}

	PROTECT(dim = allocVector(INTSXP, 2));
	p++;
	INTEGER(dim)[0] = xmax;
	INTEGER(dim)[1] = 2;
	setAttrib(result, R_DimSymbol, dim);

	UNPROTECT(p);

	return (result);
}
