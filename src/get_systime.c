/**********
 get_systime.c
 
 get the local system time as a string
 
 return value:
 character string
 
 Created by Alan Di Vittorio on 5 Sep 2013
 Copyright 2013 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
 **********/

#include "moirai.h"

char *get_systime() {
	
	time_t lt;
	struct tm *tm_ptr;
	
	lt = time(NULL);
	tm_ptr = localtime(&lt);
	strcpy(systime, asctime(tm_ptr));
	
	return systime;
}