#
#   TTR: Technical Trading Rules
#
#   Copyright (C) 2007-2008  Joshua M. Ulrich
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

"runSum" <-
function(x, n=10, cumulative=FALSE) {

  x <- try.xts(x, error=as.matrix)

  if( n < 1 || n > NROW(x) ) stop("Invalid 'n'")

  # Count NAs, ensure they're only at beginning of data.
  NAs <- sum(is.na(x))
  if( NAs > 0 ) {
    if( any( is.na(x[-(1:NAs)]) ) ) stop("Series contains non-leading NAs")
  }
  beg <- 1 + NAs
  len <- NROW(x) - NAs

  # Initialize result vector 
  result <- double(NROW(x))

  if(cumulative) {
    result[beg:NROW(x)] <- cumsum(x[beg:NROW(x)])
  } else {
    result[(n+beg-1)] <- sum(x[beg:(n+beg-1)])

    # Call Fortran routine
    result <- .Fortran( "runsum",
                     ia = as.double(x[beg:NROW(x)]),
                     lia = as.integer(len),
                     n = as.integer(n),
                     oa = as.double(result[beg:NROW(x)]),
                     loa = as.integer(len),
                     PACKAGE = "TTR",
                     DUP = FALSE )$oa
    
    # Prepend NAs from original data
    result <- c( rep( NA, NAs ), result )
  }
  
  # Replace 1:(n-1) with NAs
  is.na(result) <- c(1:(n-1+NAs))

  # Convert back to original class
  reclass(result, x)
}

#-------------------------------------------------------------------------#

"wilderSum" <-
function(x, n=10) {

  x <- try.xts(x, error=as.matrix)

  if( n < 1 || n > NROW(x) ) stop("Invalid 'n'")

  # Count NAs, ensure they're only at beginning of data, then remove.
  NAs <- sum( is.na(x) )
  if( NAs > 0 ) {
    if( any( is.na(x[-(1:NAs)]) ) ) stop("Series contains non-leading NAs")
  }
  beg <- 1 + NAs
  len <- NROW(x) - NAs

  result <- .Fortran( "wilder",
                   ia  = as.double(x[beg:NROW(x)]),
                   lia = as.integer(len),
                   n   = as.integer(n),
                   oa  = as.double(x[beg:NROW(x)]),
                   loa = as.integer(len),
                   PACKAGE = "TTR",
                   DUP = FALSE )$oa

  # Replace 1:(n-1) with NAs and prepend NAs from original data
  is.na(result) <- c(1:(n-1))
  result <- c( rep( NA, NAs ), result )

  # Convert back to original class
  reclass(result, x)
}

#-------------------------------------------------------------------------#

"runMin" <-
function(x, n=10, cumulative=FALSE) {

  x <- try.xts(x, error=as.matrix)

  if( n < 1 || n > NROW(x) ) stop("Invalid 'n'")

  # Count NAs, ensure they're only at beginning of data, then remove.
  NAs <- sum( is.na(x) )
  if( NAs > 0 ) {
    if( any( is.na(x[-(1:NAs)]) ) ) stop("Series contains non-leading NAs")
  }
  beg <- 1 + NAs
  len <- NROW(x) - NAs

  # Initialize result vector 
  result <- double(NROW(x))
  
  if(cumulative) {
    result[beg:NROW(x)] <- cummin(x[beg:NROW(x)])
  } else {
    result[(n+beg-1)] <- min(x[beg:(n+beg-1)])

    result <- .Fortran( "runmin",
                     ia = as.double(x[beg:NROW(x)]),
                     lia = as.integer(len),
                     n = as.integer(n),
                     oa = as.double(result[beg:NROW(x)]),
                     loa = as.integer(len),
                     PACKAGE = "TTR",
                     DUP = FALSE )$oa

    # Prepend NAs from original data
    result <- c( rep( NA, NAs ), result )
  }
  
  # Replace 1:(n-1) with NAs
  is.na(result) <- c(1:(n-1+NAs))

  # Convert back to original class
  reclass(result, x)
}

#-------------------------------------------------------------------------#

"runMax" <-
function(x, n=10, cumulative=FALSE) {

  x <- try.xts(x, error=as.matrix)
  
  if( n < 1 || n > NROW(x) ) stop("Invalid 'n'")

  # Count NAs, ensure they're only at beginning of data, then remove.
  NAs <- sum( is.na(x) )
  if( NAs > 0 ) {
    if( any( is.na(x[-(1:NAs)]) ) ) stop("Series contains non-leading NAs")
  }
  beg <- 1 + NAs
  len <- NROW(x) - NAs

  # Initialize result vector 
  result <- double(NROW(x))

  if(cumulative) {
    result[beg:NROW(x)] <- cummax(x[beg:NROW(x)])
  } else {
    result[(n+beg-1)] <- max(x[beg:(n+beg-1)])

    result <- .Fortran( "runmax",
                     ia = as.double(x[beg:NROW(x)]),
                     lia = as.integer(len),
                     n = as.integer(n),
                     oa = as.double(result[beg:NROW(x)]),
                     loa = as.integer(len),
                     PACKAGE = "TTR",
                     DUP = FALSE )$oa
  }

  # Replace 1:(n-1) with NAs and prepend NAs from original data
  is.na(result) <- c(1:(n-1))
  result <- c( rep( NA, NAs ), result )

  # Convert back to original class
  reclass(result, x)
}

#-------------------------------------------------------------------------#

"FrunMin" <-
function(x, n=10, cumulative=FALSE) {

  x <- try.xts(x, error=as.matrix)

  if( n < 1 || n > NROW(x) ) stop("Invalid 'n'")

  # Count NAs, ensure they're only at beginning of data, then remove.
  NAs <- sum( is.na(x) )
  if( NAs > 0 ) {
    if( any( is.na(x[-(1:NAs)]) ) ) stop("Series contains non-leading NAs")
  }
  beg <- 1 + NAs
  len <- NROW(x) - NAs

  # Initialize result vector 
  result <- double(NROW(x))
  
  if(cumulative) {
    result[beg:NROW(x)] <- cummin(x[beg:NROW(x)])
  } else {
    result[(beg)] <- min(x[beg:(beg+n-1)])

    result <- .Fortran( "Frunmin",
                     ia = as.double(x[beg:NROW(x)]),
                     lia = as.integer(len),
                     n = as.integer(n),
                     oa = as.double(result[beg:NROW(x)]),
                     loa = as.integer(len),
                     PACKAGE = "TTR",
                     DUP = FALSE )$oa

    # Prepend NAs from original data
    result <- c( rep( NA, NAs ), result )
  }
  
  # Replace 1:(n-1) with NAs
  # is.na(result) <- c(1:(n-1+NAs))

  # Convert back to original class
  reclass(result, x)
}

#-------------------------------------------------------------------------#

"FrunMax" <-
function(x, n=10, cumulative=FALSE) {

  x <- try.xts(x, error=as.matrix)
  
  if( n < 1 || n > NROW(x) ) stop("Invalid 'n'")

  # Count NAs, ensure they're only at beginning of data, then remove.
  NAs <- sum( is.na(x) )
  if( NAs > 0 ) {
    if( any( is.na(x[-(1:NAs)]) ) ) stop("Series contains non-leading NAs")
  }
  beg <- 1 + NAs
  len <- NROW(x) - NAs

  # Initialize result vector 
  result <- double(NROW(x))

  if(cumulative) {
    result[beg:NROW(x)] <- cummax(x[beg:NROW(x)])
  } else {
    result[(beg)] <- max(x[beg:(beg+n-1)])

    result <- .Fortran( "Frunmax",
                     ia = as.double(x[beg:NROW(x)]),
                     lia = as.integer(len),
                     n = as.integer(n),
                     oa = as.double(result[beg:NROW(x)]),
                     loa = as.integer(len),
                     PACKAGE = "TTR",
                     DUP = FALSE )$oa
  }

  # Replace 1:(n-1) with NAs and prepend NAs from original data
  # is.na(result) <- c(1:(n-1))
  # result <- c( rep( NA, NAs ), result )

  # Convert back to original class
  reclass(result, x)
}

#-------------------------------------------------------------------------#

"runMean" <-
function(x, n=10, cumulative=FALSE) {

  if(cumulative) {
    result <- runSum(x, n, cumulative) / 1:NROW(x)
  } else {
    result <- runSum(x, n) / n
  }

  return(result)
}

#-------------------------------------------------------------------------#

"runMedian" <-
function(x, n=10, non.unique="mean", cumulative=FALSE) {

  x <- try.xts(x, error=as.matrix)

  if( n < 1 || n > NROW(x) ) stop("Invalid 'n'")

  # Count NAs, ensure they're only at beginning of data, then remove.
  NAs <- sum( is.na(x) )
  if( NAs > 0 ) {
    if( any( is.na(x[-(1:NAs)]) ) ) stop("Series contains non-leading NAs")
  }
  beg <- 1 + NAs
  len <- NROW(x) - NAs

  # Non-unique median
  non.unique <- match.arg(non.unique, c('mean','max','min'))
  non.unique <- switch( non.unique, mean=0, max=1, min=-1 )
  
  # Call Fortran routine
  result <- .Fortran( "runmedian",
                   ia = as.double(x[beg:NROW(x)]),
                   n = as.integer(n),
                   oa = double(len),
                   la = as.integer(len),
                   ver = as.integer(non.unique),
                   cu = as.integer(cumulative),
                   PACKAGE = "TTR",
                   DUP = FALSE )$oa

  # Replace 1:(n-1) with NAs and prepend NAs from original data
  is.na(result) <- c(1:(n-1))
  result <- c( rep( NA, NAs ), result )

  # Convert back to original class
  reclass(result, x)
}

#-------------------------------------------------------------------------#

"runCov" <-
function(x, y, n=10, use="all.obs", sample=TRUE, cumulative=FALSE) {

  x <- try.xts(x, error=as.matrix)
  y <- try.xts(y, error=as.matrix)
  if(is.xts(x) && is.xts(y)) {
    xy <- cbind(x,y)
  } else {
    xy <- cbind( as.vector(x), as.vector(y) )
  }

  if( n < 1 || n > NROW(x) ) stop("Invalid 'n'")

  # "all.obs", "complete.obs", "pairwise.complete.obs"

  # Count NAs, ensure they're only at beginning of data, then remove.
  xNAs <- sum( is.na(x) )
  yNAs <- sum( is.na(y) )
  NAs <- max( xNAs, yNAs )
  if( NAs > 0 ) {
    if( any( is.na(xy[-(1:NAs),]) ) ) stop("Series contain non-leading NAs")
  }
  beg <- 1 + NAs
  len <- NROW(xy) - NAs
  
  xCenter <- runMean(x, n, cumulative)
  xCenter[1:(NAs+n-1)] <- 0
  yCenter <- runMean(y, n, cumulative)
  yCenter[1:(NAs+n-1)] <- 0

  # Call Fortran routine
  result <- .Fortran( "runCov",
                   rs1 = as.double(x[beg:NROW(xy)]),
                   avg1 = as.double(xCenter[beg:NROW(xy)]),
                   rs2 = as.double(y[beg:NROW(xy)]),
                   avg2 = as.double(yCenter[beg:NROW(xy)]),
                   la = as.integer(len),
                   n = as.integer(n),
                   samp = as.integer(sample),
                   oa = double(len),
                   cu = as.integer(cumulative),
                   PACKAGE = "TTR",
                   DUP = FALSE )$oa

  # Replace 1:(n-1) with NAs and prepend NAs from original data
  is.na(result) <- c(1:(n-1))
  result <- c( rep( NA, NAs ), result )

  # Convert back to original class
  # Should the attributes of *both* x and y be retained?
  reclass(result, x)
}

#-------------------------------------------------------------------------#

"runCor" <-
function(x, y, n=10, use="all.obs", sample=TRUE, cumulative=FALSE) {

  result <- runCov(x, y, n, use=use, sample=sample, cumulative) /
            ( runSD(x, n, sample=sample, cumulative) *
              runSD(y, n, sample=sample, cumulative) )

  return( result )
}

#-------------------------------------------------------------------------#

"runVar" <-
function(x, y=NULL, n=10, sample=TRUE, cumulative=FALSE) {

  if(is.null(y)) y <- x
  result <- runCov(x, y, n, use="all.obs", sample=sample, cumulative)

  return( result )
}

#-------------------------------------------------------------------------#

"runSD" <-
function(x, n=10, sample=TRUE, cumulative=FALSE) {

  result <- sqrt( runCov(x, x, n, use="all.obs",
                  sample=sample, cumulative) )

  return( result )
}

#-------------------------------------------------------------------------#

"runMAD" <-
function(x, n=10, center=NULL, stat="median",
         constant=1.4826, non.unique="mean", cumulative=FALSE) {

  x <- try.xts(x, error=as.matrix)

  if( n < 1 || n > NROW(x) ) stop("Invalid 'n'")

  # Count NAs, ensure they're only at beginning of data, then remove.
  NAs <- sum( is.na(x) )
  if( NAs > 0 ) {
    if( any( is.na(x[-(1:NAs)]) ) ) stop("Series contains non-leading NAs")
  }
  beg <- 1 + NAs
  len <- NROW(x) - NAs
  
  if(is.null(center)) {
    center <- runMedian(x, n, cumulative=cumulative)
  }
  center[1:(NAs+n-1)] <- 0

  # Mean or Median absolute deviation?
  median <- match.arg(stat, c("mean","median"))
  median <- switch( stat, median=TRUE, mean=FALSE )

  # Non-unique median
  non.unique <- match.arg(non.unique, c('mean','max','min'))
  non.unique <- switch( non.unique, mean=0, max=1, min=-1 )
  
  # Call Fortran routine
  result <- .Fortran( "runMAD",
                   rs = as.double(x[beg:NROW(x)]),      # raw series
                   cs = as.double(center[beg:NROW(x)]), # center series
                   la = as.integer(len),                # length of input arrays
                   n = as.integer(n),                   # size of rolling window
                   oa = double(len),                    # output array
                   stat = as.integer(median),           # center statistic
                   ver = as.integer(non.unique),        # median type
                   cu = as.integer(cumulative),         # from inception
                   PACKAGE = "TTR",
                   DUP = FALSE )$oa

  if( median ) result <- result * constant

  # Replace 1:(n-1) with NAs and prepend NAs from original data
  is.na(result) <- c(1:(n-1))
  result <- c( rep( NA, NAs ), result )

  # Convert back to original class
  reclass(result, x)
}
