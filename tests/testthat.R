Sys.setenv("R_TESTS" = "")

library(testthat)
library(moirai)

test_check("moirai")
