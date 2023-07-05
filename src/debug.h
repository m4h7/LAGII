/*
	debug.h
*/

#ifndef __DEBUG_H__
#define __DEBUG_H__

//
// DEFINES
//

#define LAGII_DEBUG		// Comment out to clear stuff

//===================
// DEBUGGING MESSAGE
//===================

#ifdef __GNUC__
#define DEBUG_LEAD		\
		fprintf (stderr, "[%s]: ", __PRETTY_FUNCTION__);
#else
#define DEBUG_LEAD		\
		fprintf (stderr, "[<unknown>]: ");
#endif	// __GNUC__



#ifdef LAGII_DEBUG

#define DEBUG_MSG1(arg1)		\
	if (1) {			\
		DEBUG_LEAD		\
		fprintf (stderr, arg1);	\
	}
#define DEBUG_MSG2(arg1,arg2)			\
	if (1) {				\
		DEBUG_LEAD			\
		fprintf (stderr, arg1, arg2);	\
	}
#define DEBUG_MSG3(arg1,arg2,arg3)			\
	if (1) {					\
		DEBUG_LEAD				\
		fprintf (stderr, arg1, arg2, arg3);	\
	}
#define DEBUG_MSG4(arg1,arg2,arg3,arg4)				\
	if (1) {						\
		DEBUG_LEAD					\
		fprintf (stderr, arg1, arg2, arg3, arg4);	\
	}
#define DEBUG_MSG5(arg1,arg2,arg3,arg4,arg5)			\
	if (1) {						\
		DEBUG_LEAD					\
		fprintf (stderr, arg1, arg2, arg3, arg4, arg5);	\
	}

#else	// !LAGII_DEBUG

#define DEBUG_MSG1(arg1)
#define DEBUG_MSG2(arg1,arg2)
#define DEBUG_MSG3(arg1,arg2,arg3)
#define DEBUG_MSG4(arg1,arg2,arg3,arg4)
#define DEBUG_MSG5(arg1,arg2,arg3,arg4,arg5)

#endif	// LAGII_DEBUG

#endif	// __DEBUG_H__
