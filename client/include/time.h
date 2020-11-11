#ifndef __TIMEVAL_H__
#define __TIMEVAH_H__

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct timeval new_timeval(time_t sec, time_t usec);

float get_timevalue(struct timeval t);

struct timeval op_timeval(struct timeval t1, char op, struct timeval t2);

struct timeval op_ntimeval(struct timeval t, char op, int n);

#endif
