'WofYear' <- function(current_date)
{
    t<-as.POSIXlt(current_date)
    t1<-ISOdate(t$year+1900, 1, 1)
    weeks<-difftime(current_date, t1, units="w")
    return(weeks)
}


'getAll' <-
function(pathname, sString, Restore) {

  if( !missing(pathname) &&
       missing(sString) )
    stop('\'Restore\' must be specified to adjust sString')
       
  resv <- .Call('getAll',as.character(pathname),sString,Restore)

  return(resv)

}

'getLastAll' <-
function(pathname, daylists=3) {

  resv <- .Call('getLastAll',as.character(pathname),daylists)

  return(resv)

}

'getLastAllV' <-
function(pathname, daylists=3) {

  resv <- .Call('getLastAllV',as.character(pathname),daylists)

  return(resv)

}

