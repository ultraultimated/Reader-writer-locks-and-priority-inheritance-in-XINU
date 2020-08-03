#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

extern unsigned long ctr1000;

int prio_writer(int priority, int time, int id, int tail, int ldes) {
    int i;
    for (i = q[tail].qprev; i != locadd[ldes].lhead && q[i].qkey > priority; i = q[i].qprev) {
        if (proctab[i].lstatus != WRITE)
            continue;
        else if (proctab[i].lstatus == WRITE)
            return TRUE;
    }
    return FALSE;
}


void priorityIn(int ldes) {
    struct pentry *currptr;
    struct pentry *newptr;
    struct lentry *lptr;
    currptr = &proctab[currpid];
    lptr = &locadd[ldes];
    currptr->pstate = PRWAIT;
    int i = 0;

    do {
        int question = lptr->allprocess[i] == WRITE || lptr->allprocess[i] == READ;
        if (question) {
            if (proctab[i].pprio <= currptr->pprio) {
                proctab[i].pinh = proctab[i].pinh == -1 ? proctab[i].pprio : proctab[i].pinh;
                chprio(i, currptr->pprio);
            }
        }
        i++;
    } while (i < NPROC);

}

void prioritt(int ldes, int pid) {
    struct lentry *lptr;
    lptr = &locadd[ldes];
    int i;
    int max = -99999;
    for (i = q[locadd[ldes].ltail].qprev; i != locadd[ldes].lhead; i = q[i].qprev) {
        if (proctab[i].pprio >= max)
            max = proctab[i].pprio;
    }
    if (max <= proctab[pid].pprio)
        chprio(pid, max);
}


int lock(int ldes1, int type, int priority) {
    STATWORD ps;
    struct pentry *pptr;
    disable(ps);
    int check =
            proctab[currpid].lacquire[ldes1] != -1 && (proctab[currpid].lacquire[ldes1] - locadd[ldes1].created) < 0;
    if (check) {
        restore(ps);
        return (SYSERR);
    } else {
        if (locadd[ldes1].allprocess[currpid] == DELETED) {
            locadd[ldes1].allprocess[currpid] = LNONE;
            return (DELETED);
        } else if (isbadlock(ldes1) || locadd[ldes1].lstate == LFREE) {
            restore(ps);
            return (SYSERR);
        } else {
            if (locadd[ldes1].ltype == READ) {
                if (type == READ) {
                    if (prio_writer(priority, ctr1000, currpid, locadd[ldes1].ltail, ldes1) != TRUE) {
                        proctab[currpid].ltype[ldes1] = type;
                        proctab[currpid].lreqtim = ctr1000;
                        proctab[currpid].lstatus = type;
                        locadd[ldes1].noofreaders++;
                        locadd[ldes1].allprocess[currpid] = type;
                        prioritt(ldes1, currpid);
                        locadd[ldes1].ltype = type;

                    } else {
                        proctab[currpid].lstatus = type;
                        proctab[currpid].waitlock = ldes1;
                        proctab[currpid].pstate = PRWAIT;
                        proctab[currpid].lreqtim = ctr1000;
                        proctab[currpid].ltype[ldes1] = type;
                        insert(currpid, locadd[ldes1].lhead, priority);
                        priorityIn(ldes1);
                        resched();
                    }
                } else if (type == WRITE) {
                    proctab[currpid].lstatus = type;
                    proctab[currpid].ltype[ldes1] = type;
                    proctab[currpid].waitlock = ldes1;
                    proctab[currpid].lreqtim = ctr1000;
                    proctab[currpid].pstate = PRWAIT;
                    insert(currpid, locadd[ldes1].lhead, priority);
                    priorityIn(ldes1);
                    resched();
                }
            } else if (locadd[ldes1].ltype == WRITE) {
                proctab[currpid].pstate = PRWAIT;
                proctab[currpid].lreqtim = ctr1000;
                proctab[currpid].lstatus = type;
                proctab[currpid].waitlock = ldes1;
                proctab[currpid].ltype[ldes1] = type;
                insert(currpid, locadd[ldes1].lhead, priority);
                priorityIn(ldes1);
                resched();
            } else if (locadd[ldes1].ltype == LNONE) {
                proctab[currpid].lreqtim = ctr1000;
                proctab[currpid].lstatus = type;
                proctab[currpid].ltype[ldes1] = type;
                locadd[ldes1].allprocess[currpid] = type;
                locadd[ldes1].ltype = type;
                prioritt(ldes1, currpid);
                locadd[ldes1].noofreaders = type == READ ? locadd[ldes1].noofreaders + 1 : locadd[ldes1].noofreaders;
            }
        }
    }
    restore(ps);
    return (OK);
}

