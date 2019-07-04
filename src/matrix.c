#include "Rmongo.h"
#include "gsl/gsl_matrix.h"


#define LEVEL5 10

gsl_matrix *raw2matrix(TickRaw *src,int x, int y) {
   gsl_matrix *m;
   m = gsl_matrix_alloc (x, y);

  return(m);
}

gsl_matrix *raw2gmatrixA(const TickRaw *raw,int xcol, int ycol){
  int cnt,j,k;

   gsl_matrix *m,*tm,*gm=NULL;
   gsl_vector *gv;

   double xmin,xmax,ymin,ymax;

  
 while(raw!=NULL) {
   if ( raw->nRows < 2 ) break; 

  j = cnt = 0;
   for(j=0; j<raw->nRows; j++) {
	if (raw->v[xcol+j*CONVERSLEN] <= 0) continue;
	if (raw->v[ycol+j*CONVERSLEN] <= 0) continue;
	cnt++;
   }
  
   m = gsl_matrix_alloc(cnt,CONVERSLEN);
   
  k = 0;
   
   for(j=0; j<raw->nRows; j++) {
	if (raw->v[xcol+j*CONVERSLEN] <= 0) continue;
	if (raw->v[ycol+j*CONVERSLEN] <= 0) continue;
 	if ( k>= cnt) break;	
   	    memcpy(&m->data[k*CONVERSLEN],&raw->v[j*CONVERSLEN],sizeof(double)*CONVERSLEN);
	    k++;
   }

   tm = gsl_matrix_alloc(CONVERSLEN,cnt);
   gsl_matrix_transpose_memcpy(tm,m);

   gv = gsl_vector_alloc(cnt);

   memcpy(gv->data,&tm->data[xcol*cnt],sizeof(double)*cnt);
   gsl_vector_minmax(gv,&xmin,&xmax);
   ndebug("%d xmin %4.2lf xmax %4.2lf\n",xcol, xmin,xmax);

   memcpy(gv->data,&tm->data[ycol*cnt],sizeof(double)*cnt);
   gsl_vector_minmax(gv,&ymin,&ymax);
   ndebug("%d ymin %4.2lf ymax %4.2lf\n",ycol, ymin,ymax);
/*
   gm = gsl_matrix_alloc (square, square);
   for ( k=0 ; k<square; k++)
   	for ( j=0 ; j<square; j++)
           gsl_matrix_set (gm, k, j, 0.25 + 100 * k + j);
*/
   gsl_vector_free(gv);
   gsl_matrix_free(m);
   break;
} 

  return( tm );

}


int inrange(double s, double s1, double s2)
{
    return (((s <= s1) || (s >= s2)) ? FALSE : TRUE);
}

int withinrange(double s, double s1, double s2)
{
    return (((s > s1) && (s < s2)) ? TRUE : FALSE);
}


int sameside(int i, int j)
{
    return (((i < 5) && (j < 5)) ? TRUE : (((i >= 5) && (j >= 5)) ? TRUE : FALSE));
}

double notlist_price(double *s1, double *s2) {
       double retval = 0.0;
       int i;
        for (i = 0; i < LEVEL5; i++)
            if (withinrange(s2[B1P + 2 * i], s1[B1P], s1[O1P]) == TRUE) {
                    retval += s2[B1P + 2 * i] * s2[B1V + 2 * i];
          }

	return(retval);
}


int bidaskIncDec(double *ptr, double *s1, double *s2) {

    int i, j,x, retval;
    int found;

    for (i = 0; i < LEVEL5; i++) {

       if (s1[B1P + 2 * i] == 0) continue;
       if (withinrange(s1[B1P + 2 * i], s2[B5P], s2[O5P]) == FALSE) continue;

       found = FALSE;

       for (j = 0; j < LEVEL5; j++) {

           if (withinrange(s2[B1P + 2 * j], s1[B5P], s1[O5P]) == FALSE) continue;

           if (s1[B1P + 2 * i] == s2[B1P + 2 * j]) {

           found = TRUE; x = 0;

           if (i >= 5) x += 2;

           if (sameside(i, j) == TRUE) {

           if (s1[B1V + 2 * i] < s2[B1V + 2 * j]) x += 1;
                 ptr[EleBidInc + x] += abs(s1[B1P + 2 * i] * s1[B1V + 2 * i]
                                    - s2[B1P + 2 * j] * s2[B1V + 2 * j]); 
           } else {
                 ptr[EleBidInc + x] += abs(s1[B1P + 2 * i] * s1[B1V + 2 * i]
                                    + s2[B1P + 2 * j] * s2[B1V + 2 * j]);
                    // should not subtraction amDiff letter;
           }

           break;
           }

      }                          // end for j loop

      if (found != TRUE) {
          x = 0;
          x = (i >= 5)?x+2:x;
             ptr[EleBidInc + x] += s1[B1P + 2 * i] * s1[B1V + 2 * i];
        }

     }    // end for i loop

     return(retval);

}

int dninrange(double s, double s1, double s2)
{
    return (((s <= s1) || (s > s2)) ? FALSE : TRUE);
}

double notlist_upprice(double *s1, double *s2) {
       double retval = 0.0;
       int i;
        for (i = 0; i < LEVEL5; i++)
            if (dninrange(s2[B1P + 2 * i], s1[B1P], s1[O1P]) == TRUE) {
                    retval += s2[B1P + 2 * i] * s2[B1V + 2 * i];
          }

	return(retval);
}

int upinrange(double s, double s1, double s2)
{
    return (((s < s1) || (s >= s2)) ? FALSE : TRUE);
}

double notlist_dnprice(double *s1, double *s2) {
       double retval = 0.0;
       int i;
        for (i = 0; i < LEVEL5; i++)
            if (upinrange(s2[B1P + 2 * i], s1[B1P], s1[O1P]) == TRUE) {
                    retval += s2[B1P + 2 * i] * s2[B1V + 2 * i];
          }

	return(retval);
}

TickRaw *tick2raw(qV *sd, int tDiff)
{

    int i, j, k, t, x;
    TickRaw *raw_head, *ptr;
    int found, nRows, *up_t, *dn_t;
    TRADE_STATE t_b;

    double avPrice, amDiff, vDiff,tmDiff;
    double *ds1, *ds2, *ptr_v;
    double by_bid, by_ask;
    double tSubs = (double)tDiff;

    raw_head = ptr = Calloc(1, TickRaw);

    while (1) {

        if (sd == NULL) break;
        if (sd->nCols < 2) break;

        for (k = 0; k < sd->nCols - 1; k++) {
            if (tDiff == 0) break;

            ds1 = ((double *)sd->v) + (EMPTYCOLS * k);
            ds2 = ds1 + EMPTYCOLS;

            if ((tDiff > 0) && (tSubs -= diffTime(ds1[T], ds2[T])) < 0) break;
        }

        if (tDiff == 0)  nRows = sd->nCols; else nRows = k+1;

        ptr->nRows = nRows - 1;
        ptr->nCols = CONVERSLEN;

        ptr->v = Calloc(CONVERSLEN * ptr->nRows, double);

        for (k = 0; k < nRows - 1; k++) {
            ptr_v = ptr->v + (CONVERSLEN * k);

	        ptr_v[EleTickAm] = ptr_v[EleTickPrice] 
	    	= ptr_v[EleHigh] = ptr_v[EleLow] 
	    	= ptr_v[EleAsk] = ptr_v[EleBid] 
	    	= ptr_v[ElePassAsk] = ptr_v[ElePassBid] 
	    	= ptr_v[EleActAsk] = ptr_v[EleActBid] = 0.0;

            ds1 = ((double *)sd->v) + (EMPTYCOLS * k);
            ds2 = ds1 + EMPTYCOLS;

            if ( ( tmDiff = diffTime(ds1[T], ds2[T]) ) > MINITICK ) continue;

            found = FALSE;
            t_b = T_unset;
            t = 0;

            if ( (Tistrade(ds1[T]) == FALSE) && 
             (Tistrade(ds2[T]) == FALSE) ) continue;

            amDiff = *(ds1 + AM) - *(ds2 + AM);
            vDiff = *(ds1 + VL) - *(ds2 + VL);

            ptr_v[EleTime] = ds1[T];
	    			// FIX time+date+stk generate a mark; [STK] [D] [T]
            ptr_v[EleHigh] = ds1[HI];
            ptr_v[EleLow] = ds1[LO];
            ptr_v[EleBidPrice] = ds2[B1P];
            ptr_v[EleAskPrice] = ds2[O1P];
            ptr_v[EleAveragePrice] = ds1[AM] / ds1[VL];
            ptr_v[EleBid] =
                 ds2[B1P] * ds2[B1V] + ds2[B2P] * ds2[B2V] + ds2[B3P] * ds2[B3V] + ds2[B4P] * ds2[B4V] +
                 ds2[B5P] * ds2[B5V];
            ptr_v[EleAsk] =
                 ds2[O1P] * ds2[O1V] + ds2[O2P] * ds2[O2V] + ds2[O3P] * ds2[O3V] + ds2[O4P] * ds2[O4V] +
                 ds2[O5P] * ds2[O5V];

	    if ( vDiff < MINIVOLVAR ) continue;

            ptr_v[EleTickAm] = amDiff;
            avPrice = amDiff / vDiff;
            ptr_v[EleTickPrice] = avPrice;

            // setting_raw

            t_b = T_unset;
        if ( tmDiff  > MINITICK/2 ) {

		 if ( avPrice <= ds2[B1P] ) { ptr_v[EleActAsk] = amDiff; }
		 if ( avPrice >= ds2[O1P] ) { ptr_v[EleActBid] = amDiff; }

         if ( (vDiff<ds2[B1V]) && (vDiff<ds2[O1V]) ) 
		     bidaskIncDec(ptr_v,ds1,ds2);

	     continue;

	    } 

        //  ptr_v[EleTickAm] = ptr_v[EleTickAm]/((int)(tmDiff/5));
	   	//FIXME maynot set here ?

    	   bidaskIncDec(ptr_v,ds1,ds2);

            if (withinrange(avPrice, ds2[B1P], ds2[O1P]) == TRUE) {
		 t_b = T_balance;

		 if ( ( amDiff > ptr_v[EleBid] ) || (amDiff > ptr_v[EleAsk] ) )
		 {
                    ptr_v[EleFlatFinger] = amDiff;
		    continue;
		 }

		 if ( avPrice == ds2[B1P] ) { ptr_v[ElePassAsk] = amDiff; continue; }
		 if ( avPrice == ds2[O1P] ) { ptr_v[ElePassBid] = amDiff; continue; }

		ptr_v[ElePassBid] =
			 (amDiff - vDiff * ds1[B1P]) / round((ds1[O1P] - ds1[B1P]) * 100) * ds1[O1P] / 100;

		ptr_v[ElePassAsk] =
			 (amDiff - vDiff * ds1[O1P]) / round((ds1[B1P] - ds1[O1P]) * 100) * ds1[B1P] / 100;

	        if( ptr_v[ElePassAsk] < 0) { ptr_v[ElePassBid] += ptr_v[ElePassAsk]; ptr_v[ElePassAsk] =0.0; continue; }
	        if( ptr_v[ElePassBid] < 0) { ptr_v[ElePassAsk] += ptr_v[ElePassBid]; ptr_v[ElePassBid] =0.0;continue; }

                continue;
	    }
            if ((ds1[B1P] + ds1[O1P]) == (ds2[B1P] + ds2[O1P])){
		 t_b = T_balance;

		 if ( ( amDiff > ptr_v[EleBid] ) || (amDiff > ptr_v[EleAsk] ) )
		 {
                    ptr_v[EleFlatFinger] = amDiff;
		    continue;
		 }
		
		 if ( avPrice == ds2[B1P] ) { ptr_v[ElePassAsk] = amDiff; continue; }
		 if ( avPrice == ds2[O1P] ) { ptr_v[ElePassBid] = amDiff; continue; }
    		 
		ptr_v[ElePassAsk] =
			 (amDiff - vDiff * ds1[B1P]) / round((ds1[O1P] - ds1[B1P]) * 100) * ds1[O1P] / 100;

		ptr_v[ElePassBid] =
			 (amDiff - vDiff * ds1[O1P]) / round((ds1[B1P] - ds1[O1P]) * 100) * ds1[B1P] / 100;

	        if( ptr_v[ElePassAsk] < 0) { ptr_v[ElePassAsk] += ptr_v[ElePassBid]; ptr_v[ElePassBid] =0.0; continue; }
	        if( ptr_v[ElePassBid] < 0) { ptr_v[ElePassBid] += ptr_v[ElePassAsk]; ptr_v[ElePassAsk] =0.0;continue; }
		
                 continue;
             }

             if (ds2[B1P] == round(ds1[YC] * 1.1 * 100) / 100) {
		        t_b = T_rise_limit;
                (amDiff < (double)HUGEAM) ? (ptr_v[EleActBid] = amDiff) : (ptr_v[EleActAsk] = amDiff);
                continue;
             }

             if (ds2[O1P] == round(ds1[YC] * 0.9 * 100) / 100) {
                t_b = T_down_limit;
                (amDiff < (double)HUGEAM) ? (ptr_v[EleActAsk] = amDiff) : (ptr_v[EleActBid] = amDiff);
                continue;
             }

             if ((ds1[B1P] + ds1[O1P]) > (ds2[B1P] + ds2[O1P])) {
                 t_b = T_rise;
		    
		 if ( amDiff > ptr_v[EleBid] ) 
                    ptr_v[EleFlatFinger] = amDiff;
		 
		    ptr_v[EleActBid] = notlist_price(ds1,ds2);
		    if (amDiff > ptr_v[EleActBid]) {
			ptr_v[ElePassBid]=amDiff-ptr_v[EleActBid];
			ptr_v[EleActBid] = amDiff - ptr_v[ElePassBid];
		        } else  ptr_v[EleActBid] = amDiff;
                    continue;
             }
             if ((ds1[B1P] + ds1[O1P]) < (ds2[B1P] + ds2[O1P])) {
                 t_b = T_down;
		 if ( amDiff > ptr_v[EleAsk] ) 
                    ptr_v[EleFlatFinger] = amDiff;
	
		    ptr_v[EleActAsk] = notlist_price(ds1,ds2);
		    if (amDiff > ptr_v[EleActAsk]) {
			ptr_v[ElePassAsk]=amDiff-ptr_v[EleActAsk];
			ptr_v[EleActAsk] = amDiff - ptr_v[ElePassAsk];
		    } else ptr_v[EleActAsk] = amDiff;
                    continue;
             }

        }  //end for k loop

        break;

    }


    return (raw_head);

}

#if 0

int diffValue(double *dst,double *ds1,double *ds2) {

  int i,j,retval = 0;
  double avPrice,amDiff,vDiff;

  *(dst + 4) = amDiff = ds1[10] - ds2[10];
  vDiff = ds1[9]-ds2[9];
   

  if( vDiff != 0) {
   avPrice = amDiff / vDiff;

        for(i=0;i<=8;i+=2) {
          *(dst+6) += ds1[21+i] * ds1[22+i];
          *(dst+5) += ds1[11+i] * ds1[12+i];
    }

        for(i=0;i<=8;i+=2) {
        for(j=0;j<=8;j+=2) {  
      if(ds1[22+i] == ds2[22+j])  {
        *(dst +10) += (ds1[21+i] - ds2[21+j])*ds2[22+j];
        break; }
         } 
    }
    
        for(i=0;i<=8;i+=2) {
        for(j=0;j<=8;j+=2) {  
      if(ds1[12+i] == ds2[12+j])  {
        *(dst +9) += (ds1[11+i] - ds2[11+j])*ds2[12+j];
        break; }
         } 
    }
    *(dst +11) = avPrice;

   if( avPrice >= *(ds2+22)) {  ;
     // deal up over ask scrab  
   } else if( avPrice <= *(ds2+12)) { ;
     // deal down under bid list

   } else if( (avPrice > ds2[12]) && (avPrice < ds2[22]) ) {
  
#endif


int balance_ba_passive(double *ptr, double *s1, double *s2,double amD,double vD) {
    while(1) {     
		ptr[ElePassBid] =
			 (amD - vD * s1[B1P]) / ( round((s1[O1P] - s1[B1P]) * 100) / 100 ) * s1[O1P];

		ptr[ElePassAsk] =
			 (amD - vD * s1[O1P]) / ( round((s1[B1P] - s1[O1P]) * 100) / 100 ) * s1[B1P];

	        if( ptr[ElePassAsk] < 0) 
                { ptr[ElePassBid] += ptr[ElePassAsk]; ptr[ElePassAsk] =0.0; break; }
	        if( ptr[ElePassBid] < 0) 
                { ptr[ElePassAsk] += ptr[ElePassBid]; ptr[ElePassBid] =0.0; break; }

            break;
	 } 
    return(0);
}

int balance_bidaskIncDec(double *ptr, double *s1, double *s2) {

    int i, j,x, retval=0;
    int found;

    for (i = 0; i < LEVEL5; i++) {

       if (s1[B1P + 2 * i] == 0) continue;
       if (withinrange(s1[B1P + 2 * i], s2[B5P], s2[O5P]) == FALSE) continue;

       found = FALSE;

       for (j = 0; j < LEVEL5; j++) {

           if (withinrange(s2[B1P + 2 * j], s1[B5P], s1[O5P]) == FALSE) continue;

           if (s1[B1P + 2 * i] == s2[B1P + 2 * j]) {

           x = (i >= 5)? x = 2: 0;  //change from bidInc to askInc

           if (sameside(i, j) == TRUE) {

           if (s1[B1V + 2 * i] < s2[B1V + 2 * j]) x += 1;
               ptr[EleBidInc + x] += abs(s1[B1P + 2 * i] * s1[B1V + 2 * i]
                                  - s2[B1P + 2 * j] * s2[B1V + 2 * j]); 
           } else {
               ptr[EleBidInc + x] += abs(s1[B1P + 2 * i] * s1[B1V + 2 * i]
                                  + s2[B1P + 2 * j] * s2[B1V + 2 * j]);
                    // should not subtraction amDiff letter;
           }
           found = TRUE;
           break;
           }

      }         // end for j loop

      if (found != TRUE) {
          x = 0;
          x = (i >= 5)?x+2:x;
             ptr[EleBidInc + x] += s1[B1P + 2 * i] * s1[B1V + 2 * i];
        }

     }    // end for i loop

     return(retval);

}

TickRaw *tick2detail(qV *sd, int tDiff)
{

    int i, j, k, t, x;
    TickRaw *raw_head, *ptr;
    int found, nRows, *up_t, *dn_t;
    TRADE_STATE t_b;

    double avPrice, amDiff, vDiff,tmDiff;
    double *ds1, *ds2, *ptr_v;
    double by_bid, by_ask;
    double tSubs = (double)tDiff;

    raw_head = ptr = Calloc(1, TickRaw);

    while (1) {

        if (sd == NULL) break;
        if (sd->nCols < 2) break;

        for (k = 0; k < sd->nCols - 1; k++) {
            if (tDiff == 0) break;

            ds1 = ((double *)sd->v) + (EMPTYCOLS * k);
            ds2 = ds1 + EMPTYCOLS;

            if ((tDiff > 0) && (tSubs -= diffTime(ds1[T], ds2[T])) < 0) break;
        }

        if (tDiff == 0)  nRows = sd->nCols; else nRows = k+1;

        ptr->nRows = nRows - 1;
        ptr->nCols = CONVERSLEN;

        ptr->v = Calloc(CONVERSLEN * ptr->nRows, double);

        for (k = 0; k < nRows - 1; k++) {
            ptr_v = ptr->v + (CONVERSLEN * k);

	        ptr_v[EleTickAm] = ptr_v[EleTickPrice] = ptr_v[EleHigh] = ptr_v[EleLow] 
            = ptr_v[EleAsk] = ptr_v[EleBid] = ptr_v[ElePassAsk] = ptr_v[ElePassBid] 
	    	= ptr_v[EleActAsk] = ptr_v[EleActBid] = 0.0;

            ds1 = ((double *)sd->v) + (EMPTYCOLS * k);
            ds2 = ds1 + EMPTYCOLS;

            found = FALSE;
            t_b = T_unset;

            if ( (Tistrade(ds1[T]) == FALSE) && 
             (Tistrade(ds2[T]) == FALSE) ) continue;

            amDiff = *(ds1 + AM) - *(ds2 + AM);
            vDiff = *(ds1 + VL) - *(ds2 + VL);

    x=0;
            if (ds1[B1P]==ds2[B1P])  x++;
            if (ds1[O1P]==ds2[O1P])  x++;
            if (ds1[B2P]==ds2[B2P])  x++;
            if (ds1[O2P]==ds2[O2P])  x++;
            if (ds1[B3P]==ds2[B3P])  x++;
            if (ds1[O3P]==ds2[O3P])  x++;
            if (ds1[B4P]==ds2[B4P])  x++;
            if (ds1[O4P]==ds2[O4P])  x++;
            if (ds1[B5P]==ds2[B5P])  x++;
            if (ds1[O5P]==ds2[O5P])  x++;
              
    if( x >=8 ) goto skip_diff;

         if( ( vDiff / ( *(ds1+VL) ) > 0.5 )  || ( vDiff / ( *(ds1+VL) ) < -0.5 ) 
         || ( amDiff / ( *(ds1+AM) ) > 0.5 )  || ( amDiff / ( *(ds1+AM) ) < -0.5 ) ) continue;
        
            if ( (ds1[B1P]/ds2[B1P])>1.2 ||  (ds1[O1P]/ds2[O1P])>1.2 
              || (ds1[B2P]/ds2[B2P])>1.2 ||  (ds1[O2P]/ds2[O2P])>1.2 
              || (ds1[B3P]/ds2[B3P])>1.2 ||  (ds1[O3P]/ds2[O3P])>1.2 
              || (ds1[B4P]/ds2[B4P])>1.2 ||  (ds1[O4P]/ds2[O4P])>1.2 
              || (ds1[B5P]/ds2[B5P])>1.2 ||  (ds1[O5P]/ds2[O5P])>1.2 
              || (ds1[B1P]/ds2[B1P])<0.8 ||  (ds1[O1P]/ds2[O1P])<0.8 
              || (ds1[B2P]/ds2[B2P])<0.8 ||  (ds1[O2P]/ds2[O2P])<0.8 
              || (ds1[B3P]/ds2[B3P])<0.8 ||  (ds1[O3P]/ds2[O3P])<0.8 
              || (ds1[B4P]/ds2[B4P])<0.8 ||  (ds1[O4P]/ds2[O4P])<0.8 
              || (ds1[B5P]/ds2[B5P])<0.8 ||  (ds1[O5P]/ds2[O5P])<0.8 ) continue;

   skip_diff:  
            ptr_v[EleTime] = ds1[T];
	    			// FIX time+date+stk generate a mark; [STK] [D] [T]
            ptr_v[EleHigh] = ds1[HI];
            ptr_v[EleLow] = ds1[LO];
            ptr_v[EleBidPrice] = ds2[B1P];
            ptr_v[EleAskPrice] = ds2[O1P];
            ptr_v[EleAveragePrice] = ds1[AM] / ds1[VL];
            ptr_v[EleBid] =
                 ds2[B1P] * ds2[B1V] + ds2[B2P] * ds2[B2V] + ds2[B3P] * ds2[B3V] + ds2[B4P] * ds2[B4V] +
                 ds2[B5P] * ds2[B5V];
            ptr_v[EleAsk] =
                 ds2[O1P] * ds2[O1V] + ds2[O2P] * ds2[O2V] + ds2[O3P] * ds2[O3V] + ds2[O4P] * ds2[O4V] +
                 ds2[O5P] * ds2[O5V];

	    if ( vDiff < MINIVOLVAR ) continue;

            ptr_v[EleTickAm] = amDiff;
            avPrice = amDiff / vDiff;
            ptr_v[EleTickPrice] = avPrice;

       x=0;
            if (ds1[B1P]==ds2[B1P])  x++;
            if (ds1[O1P]==ds2[O1P])  x++;
            if (ds1[B2P]==ds2[B2P])  x++;
            if (ds1[O2P]==ds2[O2P])  x++;
            if (ds1[B3P]==ds2[B3P])  x++;
            if (ds1[O3P]==ds2[O3P])  x++;
            if (ds1[B4P]==ds2[B4P])  x++;
            if (ds1[O4P]==ds2[O4P])  x++;
            if (ds1[B5P]==ds2[B5P])  x++;
            if (ds1[O5P]==ds2[O5P])  x++;
              
    if( x >=9 ) goto skip_time;

        if ( ( tmDiff = diffTime(ds1[T], ds2[T]) ) > MINITICK/4 ) continue;

        // setting_raw
        if ( tmDiff == MINITICK/4 ) {

		if ( avPrice <= ds2[B1P] ) { ptr_v[EleActAsk] = amDiff; }
		if ( avPrice >= ds2[O1P] ) { ptr_v[EleActBid] = amDiff; }

        if ( (vDiff<ds2[B1V]) && (vDiff<ds2[O1V]) ) 
		     bidaskIncDec(ptr_v,ds1,ds2);

	     continue;

	    } 

        //  ptr_v[EleTickAm] = ptr_v[EleTickAm]/((int)(tmDiff/5));
	   	//FIXME maynot set here ?
    skip_time:
    
    while(1) {
        if ( (ds1[B1P] == ds2[B1P]) & (ds2[B1P] == round(ds1[YC] * 1.1 * 100) / 100) ) {
             t_b = T_rise_limit; break; }
        if ( (ds1[O1P]==ds2[O1P]) & (ds2[O1P] == round(ds1[YC] * 0.9 * 100) / 100) ) {
                t_b = T_down_limit; break; }
        if ( (ds1[B1P] == ds2[B1P]) &&  (ds1[O1P] == ds2[O1P]) ) {
             t_b = T_balance; break; }
        if ( (ds1[B1P] == ds2[B1P]) &&  (ds1[O1P] > ds2[O1P]) ) {
             t_b = T_rise1; break; }
        if ( (ds1[B1P] == ds2[B1P]) &&  (ds1[O1P] < ds2[O1P]) ) {
             t_b = T_down1; break; }

        if ( (ds1[B1P] < ds2[B1P]) &&  (ds1[O1P] < ds2[O1P]) ) {
             t_b = T_down; break; }
        if ( (ds1[B1P] < ds2[B1P]) &&  (ds1[O1P] == ds2[O1P]) ) {
             t_b = T_down2; break; }
        if ( (ds1[B1P] < ds2[B1P]) &&  (ds1[O1P] > ds2[O1P]) ) {
             t_b = T_flat; break; }

        if ( (ds1[B1P] > ds2[B1P]) &&  (ds1[O1P] > ds2[O1P]) ) {
             t_b = T_rise; break; }
        if ( (ds1[B1P] > ds2[B1P]) &&  (ds1[O1P] == ds2[O1P]) ) {
             t_b = T_rise2; break; }
        if ( (ds1[B1P] > ds2[B1P]) &&  (ds1[O1P] < ds2[O1P]) ) {
             t_b = T_flat1; break; }
        
        break;
        }

    ptr_v[EleType] = t_b; 
    while(1) {

        balance_bidaskIncDec(ptr_v,ds1,ds2);

     switch( t_b ) {
         case T_unset: break; //error
         case T_balance:
               
    	       if (withinrange(avPrice, ds2[B1P], ds2[O1P]) == TRUE)  {
                   if ( amDiff > (ptr_v[EleBid]+ptr_v[EleAsk]) ) {
                       ptr_v[EleFlatFinger] = amDiff; break; }
                   else balance_ba_passive(ptr_v,ds1,ds2,amDiff,vDiff); 
               } else if ( avPrice == ds2[B1P] ) {
                   ptr_v[EleActAsk] = amDiff; 
               } else if ( avPrice == ds2[O1P] ) {
                   ptr_v[EleActBid] = amDiff; 
               }
            break;

         case T_rise:
         case T_rise1:
         case T_rise2:
             while(1) {
               
    	       if (withinrange(avPrice, ds2[B1P], ds2[O1P]) == TRUE)  {
                      balance_ba_passive(ptr_v,ds1,ds2,amDiff,vDiff); 
                      ptr_v[EleActBid] += ptr_v[ElePassBid]; 
                      ptr_v[ElePassBid] = 0;
                      break;
               } 
               
               if ( ( ( avPrice >= ds2[O5P] )  && ds2[O5P]!=0 ) || 
                    ( ( avPrice > ds2[O4P] ) && ds2[O5P]==0 ) )  {
                      ptr_v[EleActBid] = ds2[O1P] * ds2[O1V] + ds2[O2P] * ds2[O2V] 
                                        + ds2[O3P] * ds2[O3V] + ds2[O4P] * ds2[O4V];
                      if ( ptr_v[EleActBid] < amDiff ) 
                            ptr_v[EleFlatFinger] = amDiff-ptr_v[EleActBid];
                      break;
               } 

               if ( ( ( avPrice >= ds2[O4P] )  && ds2[O4P]!=0 ) ||
                    ( ( avPrice > ds2[O3P] ) && ds2[O4P]==0 ) )  {
                      ptr_v[EleActBid] = ds2[O1P] * ds2[O1V] + ds2[O2P] * ds2[O2V] 
                                        + ds2[O3P] * ds2[O3V];
                      if ( ptr_v[EleActBid] < amDiff ) 
                            ptr_v[ElePassBid] = amDiff-ptr_v[EleActBid];
                      break;
               } 
               
               if ( ( ( avPrice >= ds2[O3P] )  && ds2[O3P]!=0 ) ||
                    ( ( avPrice > ds2[O2P] ) && ds2[O3P]==0 ) )  {
                      ptr_v[EleActBid] = ds2[O1P] * ds2[O1V] + ds2[O2P] * ds2[O2V] 
                                        + ds2[O3P] * ds2[O3V];
                      if ( ptr_v[EleActBid] < amDiff ) 
                            ptr_v[ElePassBid] = amDiff-ptr_v[EleActBid];
                      break;
               } 
               
               if ( ( ( avPrice >= ds2[O2P] )  && ds2[O2P]!=0 ) ||
                    ( ( avPrice > ds2[O1P] ) && ds2[O2P]==0 ) )  {
                      ptr_v[EleActBid] = ds2[O1P] * ds2[O1V] ;
                      if ( ptr_v[EleActBid] < amDiff ) 
                            ptr_v[ElePassBid] = amDiff-ptr_v[EleActBid];
                      break;
               } 
               
               if ( ( avPrice > ds2[O1P] )  && ds2[O1P]!=0 ) {
                      ptr_v[EleActBid] = ds2[O1P] * ds2[O1V];
                      if ( ptr_v[EleActBid] < amDiff ) 
                            ptr_v[ElePassBid] = amDiff-ptr_v[EleActBid];
                      break;
               }

               if ( avPrice == ds2[O1P] ) {
                      ptr_v[ElePassBid] = amDiff;
                   }

                   break;
             }
            break;

         case T_down:
         case T_down1:
         case T_down2:
             while(1) {
    	       if (withinrange(avPrice, ds2[B1P], ds2[O1P]) == TRUE)  {
                      balance_ba_passive(ptr_v,ds1,ds2,amDiff,vDiff); 
                      ptr_v[EleActAsk] += ptr_v[ElePassAsk]; 
                      ptr_v[ElePassAsk] = 0;
                      break;
               } 
               
               if ( ( ( avPrice <= ds2[B5P] )  && ds2[B5P]!=0 ) || 
                    ( ( avPrice < ds2[B4P] ) && ds2[B5P]==0 ) )  {
                      ptr_v[EleActAsk] = ds2[B1P] * ds2[B1V] + ds2[B2P] * ds2[B2V] 
                                        + ds2[B3P] * ds2[B3V] + ds2[B4P] * ds2[B4V];
                      if ( ptr_v[EleActAsk] < amDiff ) 
                            ptr_v[EleFlatFinger] = amDiff-ptr_v[EleActAsk];
                      break;
               } 

               if ( ( ( avPrice <= ds2[B4P] )  && ds2[B4P]!=0 ) ||
                    ( ( avPrice < ds2[B3P] ) && ds2[B4P]==0 ) )  {
                      ptr_v[EleActAsk] = ds2[B1P] * ds2[B1V] + ds2[B2P] * ds2[B2V] 
                                        + ds2[B3P] * ds2[B3V];
                      if ( ptr_v[EleActAsk] < amDiff ) 
                            ptr_v[ElePassAsk] = amDiff-ptr_v[EleActAsk];
                      break;
               } 
               
               if ( ( ( avPrice <= ds2[B3P] )  && ds2[B3P]!=0 ) ||
                    ( ( avPrice < ds2[B2P] ) && ds2[B3P]==0 ) )  {
                      ptr_v[EleActAsk] = ds2[B1P] * ds2[B1V] + ds2[B2P] * ds2[B2V] 
                                        + ds2[B3P] * ds2[B3V];
                      if ( ptr_v[EleActAsk] < amDiff ) 
                            ptr_v[ElePassAsk] = amDiff-ptr_v[EleActAsk];
                      break;
               } 
               
               if ( ( ( avPrice <= ds2[B2P] )  && ds2[B2P]!=0 ) ||
                    ( ( avPrice < ds2[B1P] ) && ds2[B2P]==0 ) )  {
                      ptr_v[EleActAsk] = ds2[B1P] * ds2[B1V] ;
                      if ( ptr_v[EleActAsk] < amDiff ) 
                            ptr_v[ElePassAsk] = amDiff-ptr_v[EleActAsk];
                      break;
               } 
               
               if ( ( avPrice < ds2[B1P] )  && ds2[B1P]!=0 ) {
                      ptr_v[EleActAsk] = ds2[B1P] * ds2[B1V];
                      if ( ptr_v[EleActAsk] < amDiff ) 
                            ptr_v[ElePassAsk] = amDiff-ptr_v[EleActAsk];
                      break;
               }

               if ( avPrice == ds2[B1P] ) {
                      ptr_v[ElePassAsk] = amDiff;
                   }

                   break;
             }
            
         break;

         case T_flat:
         case T_flat1:
               
    	       if (withinrange(avPrice, ds2[B1P], ds2[O1P]) == TRUE)  {
                      balance_ba_passive(ptr_v,ds1,ds2,amDiff,vDiff); 
               } 
              break;

         case  T_rise_limit:
              if ( (amDiff>500000) & ( amDiff*0.02 > ds2[O1P]*ds2[O1V] ) )
                    ptr_v[ElePassAsk] = amDiff;
              else 
                    ptr_v[ElePassBid] = amDiff;
          break;

         case  T_down_limit:
              if ( (amDiff>500000) & ( amDiff*0.02 > ds2[B1P]*ds2[B1V] ) )
                    ptr_v[ElePassBid] = amDiff;
              else 
                    ptr_v[ElePassAsk] = amDiff;
          break; 

         default:
            break;

       }  // end switch

      break;

    }  // end while(1) before switch

    } //end for
    break;
    }

    return (raw_head);
}
