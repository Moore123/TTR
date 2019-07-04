#include "utils.h"

int Lfread(long *ptr, int cur, int cmp, FILE * gp)
{
	long tmpbuf;
	int retval = 0;

	switch (cur) {
	case 4:
		if (cmp == 4)
			retval = fread(ptr, 4, 1, gp);
		if (cmp == 8) {
			retval = fread(ptr, 4, 1, gp);
			retval = fread(&tmpbuf, 4, 1, gp);
		}
		break;
	case 8:
		if (cmp == 4)
			retval = fread(ptr, 4, 1, gp);
		if (cmp == 8) {
			retval = fread(ptr, 8, 1, gp);
		}
		break;
	default:
		retval = -1;
		break;
	}

	return (retval);

}

int ndebug(char *fmt, ...)
{
	int retval = TRUE;
	FILE *fp;

	va_list args;

	if ((fp = fopen("/var/log/TTRtempFile.log", "a+")) != NULL) {

		va_start(args, fmt);
		vfprintf(fp, fmt, args);
		va_end(args);

		fclose(fp);
	} else
		retval = FALSE;

	return (retval);
}

int name2num(char *name)
{
	int retval = STKSPACE - 1;

	if (strncmp(name, "sz00", 4) == 0) {
		retval = (atoi(name + 2));
	} else if (strncmp(name, "sz30", 4) == 0) {
		retval = (atoi(name + 3) + SZRSV);

	} else if (strncmp(name, "sh60", 4) == 0) {
		retval = (atoi(name + 3) + SZRSV + INNRSV);

	}

	return (retval);

}

int time2num(char *name)
{

	int H, M, S;
	int retval = 0;

	sscanf(name, "%02d:%02d:%02d", &H, &M, &S);

	while (1) {

		if ((H < 9) || (H >= 16)) {
			retval = 0;
			break;
		}
		if (H == 9) {
			M -= 29;
			if (M < 0) {
				retval = 0;
				break;
			}
			retval += M * 10 + (int)S / 10;
			break;
		}
		if (H == 10) {
			retval = 30 * 10 + 10;
			retval += M * 10 + (int)S / 10;
			break;
		}
		if (H == 11) {
			retval = 90 * 10 + 10;
			retval += M * 10 + (int)S / 10;
			break;
		}

		if (H == 13) {
			retval = 120 * 10 + 20;
			retval += M * 10 + (int)S / 10;
			break;
		}

		if (H == 14) {
			retval = 180 * 10 + 20;
			retval += M * 10 + (int)S / 10;
			break;
		}

		if (H == 15) {
			retval = 240 * 10 + 30;
			retval += M * 10 + (int)S / 10;
			break;
		}

		break;

	}

	return ((retval < (240 * 10 + 40)) ? retval : (240 * 10 + 40 - 1));

}

int cntComma1(char *str)
{
	int retval = 0;
	int slen;

	char *ptr = str;
	slen = strlen(ptr);

//time2num(ptr+slen - 11);

	while (*ptr) {
		if (*ptr == ',')
			retval++;
		ptr++;
	}

	return (retval);

}

int cntCommaX(char *str)
{
	int retval = 0;
	double *v = Calloc(29, double);

	char *ptr = str;

	while (*ptr) {
		if (*ptr == ',') {
			retval++;
			break;
		}
		ptr++;
	}

	if (retval == 0)
		return (retval);

	sscanf(++ptr,
			 "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
			 &v[0], &v[1], &v[2], &v[3], &v[4], &v[5], &v[6], &v[7], &v[8], &v[9], &v[10], &v[11], &v[12], &v[13], &v[14],
			 &v[15], &v[16], &v[17], &v[18], &v[19], &v[20], &v[21], &v[22], &v[23], &v[24], &v[25], &v[26], &v[27],
			 &v[28]);

	while (*ptr) {
		if (*ptr == ',')
			retval++;
		ptr++;
	}

	free(v);
	return (retval);

}

int cntComma2(char *str)
{
	int retval = 0;
	int i;
	float *v;

	char *ptr = str;

	while (*ptr) {
		if (*ptr == ',') {
			retval++;
			break;
		}
		ptr++;
	}

	if (retval == 0)
		return (retval);

	v = Calloc(31, float);

	i = 0;
	while (i < 29) {
		sscanf(++ptr, "%f", &v[i]);

		while (*ptr) {
			if (*ptr == ',') {
				retval++;
				break;
			}
			ptr++;
		}
		i++;
	}

	while (*ptr) {
		if (*ptr == ',')
			retval++;
		ptr++;
	}

	free(v);

	return (retval);

}

float summComma(char *str)
{
	int retval = 0;
	int i;
	char *ptr;
	float *v;

	ptr = str;

	v = Calloc(31, float);

	while (*ptr != ',')
		ptr++;
	i = 0;
	while (i < 29) {
		sscanf(++ptr, "%f", &v[i]);
		i++;
		while (*ptr != ',')
			ptr++;
	}

	free(v);

	return (retval);

}



/* Subtracts time values to determine run time */
int timeval_subtract (struct timeval *result, struct timeval *t2, struct timeval *t1)
{
      long int diff = (t2->tv_usec + 100000 * t2->tv_sec) -
          (t1->tv_usec + 100000 * t1->tv_sec);
            result->tv_sec = diff / 100000;
              result->tv_usec = diff % 100000;

                return (diff < 0);
}


/* Starts timer */
void tic (struct timeval *timer) { 
            gettimeofday (timer, NULL); 
}


/* Stops timer and prints difference to the screen */
void toc (struct timeval *timer) {
      struct timeval tv_end, tv_diff;

        gettimeofday (&tv_end, NULL);
          timeval_subtract (&tv_diff, &tv_end, timer);
            ndebug("running time is %ld.%06ld\n", tv_diff.tv_sec, tv_diff.tv_usec);

}

int Tistrade(double v)
{
	int retval = TRUE;

	int H = trunc(v / 10000);
	int M = trunc((v - (H * 10000)) / 100);
	int S = trunc(v - trunc(v / 100) * 100);

	while (1) {
		if ((H >= 13) && (H < 15))
			break;
		if ((H >= 10) && (H < 11))
			break;
		if ((H == 9) && (M >= 30))
			break;
		if ((H == 11) && (M < 30))
			break;
		retval = FALSE;
		break;
	}

	return (retval);
}
