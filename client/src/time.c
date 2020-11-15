#include "time.h"

struct timeval new_timeval(time_t sec, time_t usec) {
    struct timeval t = {sec, usec};
    return t;
}

struct timeval new_cooldown(int sec, int usec) {
    struct timeval t;
    gettimeofday(&t, NULL);
    t.tv_sec += sec;
    t.tv_usec += usec;
    return t;
}

bool timeout(struct timeval t, int time) {
    struct timeval cur_t;
    PCHK(gettimeofday(&cur_t, NULL));
    cur_t.tv_sec -= time;
    return timercmp(&cur_t, &t, >=);
}

float get_timevalue(struct timeval t) {
    return t.tv_sec + (t.tv_usec * pow(10, -6));
}

struct timeval op_timeval(struct timeval t1, struct timeval t2, char op) {
    if (op == '+') {
        return new_timeval(t1.tv_sec + t2.tv_sec, t1.tv_usec + t2.tv_usec);
    } else if (op == '-') {
        return new_timeval(t1.tv_sec - t2.tv_sec, t1.tv_usec - t2.tv_usec);
    }
    return new_timeval(-1, -1);
}

struct timeval op_ntimeval(struct timeval t, int n, char op) {
    if (op == '*') {
        return new_timeval(t.tv_sec * n, t.tv_usec * n);
    } else if (op == '/') {
        return new_timeval(t.tv_sec / n, t.tv_usec / n);
    }
    return new_timeval(-1, -1);
}
