'netGet' <- function(stocklist,url) {

  return( .Call('netGet',as.character(stocklist),as.character(url)))

}


