'get_ttrc' <-
function(sFileName, lastDays, Restore) {

  if( !missing(lastDays) &&
       missing(Restore) )
    stop('\'Restore\' must be specified to adjust lastDays')
       
  if(missing(Restore) || all(is.na(Restore)) || NROW(Restore)==0) {
    Restore <- NA
  }

  if(missing(sFileName) || all(is.na(sFileName)) || NROW(sFileName)==0) {
    sFileName <- NA
  }

  if(missing(lastDays) || all(is.na(lastDays)) || NROW(lastDays)==0) {
    lastDays <- NA
  }

  ttrc <- .Call('get_ttrc',as.character(sFileName),lastDays,Restore)
  
  return(ttrc)

}

'select_ttrc' <-
function(...) {
       
  ttrc <- .Call('select_ttrc',list(...))
  
  return(ttrc)

}

'Select_Value'<-function(...) {

	df<-paste(RawPath,x,sep="/")
	pf<-paste(ProfitPath,x,sep="/")

	gttrc<-select_ttrc(Files=df,Profit=pf,nDays=1,...)

	return(gttrc)
 
}

'select_value'<-function(...) {

data_files<-list.files(RawPath)
data_files<-data_files[data_files!="sh000001.day"]
data_files<-data_files[data_files!="sz399001.day"]
retval<- applay(data_files,2,Select_Value,...)

}



'test_ab' <- function(...) {

.Call("convolve2", list(...))

}

