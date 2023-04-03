# Marcus Perry PNNL 3/22/23 
# inputStructure.R 
# Creates R vector from identical text input file used for C Moirai for use in R Moirai


# Read the values from the input file and assign them to the values in the input list
# This suppresses a warning caused by not having a new line at the end of the input file, or because all empty lines are 
# removed from the input file.
values <- suppressWarnings(readLines(file.choose())) # Choose an input file and read it in 
values <- gsub("#.*", "", values)   # Remove all characters after a '#' symbol
values <- gsub("\\t", "", values)  # Remove all tabs
values <- gsub("\\s+$", "", values) # Remove all extra spaces 
input <- values[values != ""]       # Filter out empty lines
# Each line from the input file is assigned to "input" as a character string. The number of character strings should correspond to
# the number of input lines as in C Moirai. 
input
# Check to ensure number of inputs is correct as intended
