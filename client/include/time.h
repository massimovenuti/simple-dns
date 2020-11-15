#ifndef __TIME_H__
#define __TIME_H__

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#include "macro.h"

struct timeval new_timeval(time_t sec, time_t usec);

struct timeval new_cooldown(int sec, int usec);

bool timeout(struct timeval t, int time);

float get_timevalue(struct timeval t);

struct timeval op_timeval(struct timeval t1, struct timeval t2, char op);

struct timeval op_ntimeval(struct timeval t, int n, char op);

#endif
