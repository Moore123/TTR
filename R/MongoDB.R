setClass("MongoHandle", representation(ref = "externalptr"))

'initMongo' <- function(...) {

  return( .Call("R_initMongo",list(...)))

}

'mongoQuery' <- function(Mongo,...) {

  return( .Call("R_MongoQuery",Mongo,as.character(...)));

}

'hftTrans' <- function(...) {

  return( .Call("R_hftTrans",list(...)) )

}

'hftTrans2' <- function(...) {

  return( .Call("R_hftTrans2",list(...)) )
}

'staticalMongo' <- function(Mongo,...) {

  return( .Call("R_staticalMongo",list(...)))

}

'hftTail' <- function(Mongo,stkName,ticket) {

  if(missing(ticket)) {
    ticket <- C(5)
  }
 
  return( .Call("R_hftTail",Mongo,as.character(stkName),ticket))

}


