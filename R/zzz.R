#".onLoad" <- function(lib,pkg) {
#  message("Technical Trading Rules (version 0.20-0)\n")
#}
.onUnload <- function(libpath)
    library.dynam.unload("TTR", libpath)
