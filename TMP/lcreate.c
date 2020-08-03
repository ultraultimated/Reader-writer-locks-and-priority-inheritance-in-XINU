#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

LOCAL int newloc();

extern unsigned long ctr1000;

int lcreate() {
    STATWORD ps;
    int loc;

    disable(ps);
    loc = newloc();
    restore(ps);
    if (loc == SYSERR) {
        return (SYSERR);
    }
    return (loc);
}

LOCAL int newloc() {
    int loc;
    int i;

    do {
        loc = nextloc--;
        nextloc = nextloc < 0 ? NLOCKS - 1 : nextloc;
        if (locadd[loc].lstate != LFREE) {
            continue;
        }

        if (locadd[loc].lstate == LFREE) {
            locadd[loc].lstate = LUSED;
            locadd[loc].created = ctr1000;
            locadd[loc].ltype = LNONE;
            globallock = lock;
            return (loc);
        }
        i++;
    } while (i < NLOCKS);
    return (SYSERR);
}
