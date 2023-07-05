/*
	utility.h
*/

#ifndef __UTILITY_H__
#define __UTILITY_H__

#include "types.h"

char *l_strdup (char *src);
boolean l_strcmp (const char *one, const char *two);
long int l_FileSize (const char *fname);
boolean l_fileexists (const char *fname);

#endif	// __UTILITY_H__
