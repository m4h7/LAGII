/*
	utility.c
*/

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "mem.h"
#include "types.h"
#include "utility.h"


char *l_strdup (char *src)
{
	char *ret;
	int i;

	if (!src)
		return NULL;

	i = strlen (src);
	if (i > 200)
		i = 200;
	ret = (char *) MemAlloc (i + 1);
	strncpy (ret, src, i);
	return ret;
}

boolean l_strcmp (const char *one, const char *two)
{
	if (strcmp (one, two) == 0)
		return true;
	return false;
}

long int l_FileSize (const char *fname)
{
	struct stat s;

	if (fname == NULL)
		return 0;
	stat (fname, &s);
	return (long int) s.st_size;
}

boolean l_fileexists (const char *fname)
{
	struct stat s;
	int i;

	if (fname == NULL)
		return false;
	i = stat (fname, &s);
	if (i == 0)
		return true;
	return false;
}
