/**********
 get_systime.c
 
 get the local system time as a string
 
 return value:
 character string
 
 Created by Alan Di Vittorio on 5 Sep 2013
 Copyright 2018 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
 This file is part of Moirai.
 
 Moirai is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. Moirai is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with Moirai (/moirai/docs/COPYING.txt). If not, see <https://www.gnu.org/licenses/>.
 
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