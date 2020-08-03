/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
int pidd, newprioo, changed = 0;

int max_lock(int ldes) {
    int prev, max = -9999;
    for (prev = q[locadd[ldes].ltail].qprev; prev != locadd[ldes].lhead; prev = q[prev].qprev) {
        if (max < proctab[prev].pprio)
            max = proctab[prev].pprio;
    }
    return max;
}

int max_count(int max_there, int ldes) {
    int prev, count = 0;
    for (prev = q[locadd[ldes].ltail].qprev; prev != locadd[ldes].lhead; prev = q[prev].qprev) {
        if (proctab[prev].pprio == max_there)
            count++;
    }
    return count;
}

int max_second(int max_there, int ldes) {
    int prev, count = 9999, id = -1;
    prev = q[locadd[ldes].ltail].qprev;
    for (prev = q[locadd[ldes].ltail].qprev; prev != locadd[ldes].lhead; prev = q[prev].qprev) {
        if (max_there != proctab[prev].pprio && count > max_there - proctab[prev].pprio) {
            count = max_there - proctab[prev].pprio;
            id = proctab[prev].pprio;
        }
    }
    return id;
}

int max_all(int pid, int wait_ldes) {
    int g, all = 0;
    int al[100], count = 0;
    g = 0;
    while (g < NLOCKS) {
        if (g != wait_ldes && proctab[pid].ltype[g] != -1) {
            al[count] = g;
            count++;
        }
        g++;
    }
    int full = 0, max = -9999;
    while (full < count) {
        if (max_lock(al[full]) > max)
            max = max_lock(al[full]);
        full++;
    }
    return max;
}

int all_all(struct pentry *ptr, int oldprio, int newprio) {
    struct lentry *lptr = &locadd[ptr->waitlock];
    int init = 0;
    int maxall,unchanged,max_there;
    if(!init) {
        unchanged = newprio;
        maxall = max_all(pidd, ptr->waitlock);
        max_there = max_lock(ptr->waitlock);
    }
    if (maxall > newprio)
        newprio = maxall;
    if (max_there >= oldprio) {
        int max_counts = max_count(max_there, ptr->waitlock);
        if (max_counts > 1) {
            int check = max_second(max_there, ptr->waitlock);
            int togive = 0;
            if (check > newprio)
                togive = newprio;
            else if (check > newprio)
                togive = check;
            else
                togive = newprio;
            int in_lock = 0;
            ptr->pprio = newprio;
            ptr->pinh = unchanged;
            while (in_lock < NPROC) {
                int check = lptr->allprocess[in_lock] == WRITE || lptr->allprocess[in_lock] == READ;
                if (check) {
                    if (proctab[in_lock].pinh > togive)
                        chprio(in_lock, proctab[in_lock].pinh);
                    else
                        chprio(in_lock, togive);
                }
                in_lock++;
            }
        }
    }
    return newprio;
}

void newprocChange(int pid, int newprio) {
    int g, all = 0;
    int al[100], count = 0;
    g = 0;
    while (g < NLOCKS) {
        if (proctab[pid].ltype[g] != -1) {
            al[count] = g;
            all = g;
            count++;
        }
        g++;
    }
    if (count > 1) {
        int full = 0, max = -9999;
        while (full < count) {
            if (max_lock(al[full]) > max)
                max = max_lock(al[full]);
            full++;
        }
        if (max > newprio) {
            changed = 1;
            proctab[pid].pinh = newprio;

        } else {
            proctab[pid].pinh = newprio;
            proctab[pid].pprio = newprio;
        }
    } else {
        int a = max_lock(all);
        if (a > newprio) {
            changed = 1;
            proctab[pid].pinh = newprio;

        } else {
            proctab[pid].pinh = newprio;
            proctab[pid].pprio = newprio;
        }
    }
}

void procChange(int pid, int newprio) {
    struct pentry *ptr;
    ptr = &proctab[pidd];
    if (ptr->pstate == PRWAIT) {
        int max = -9999;
        int prev;
        for (prev = q[locadd[ptr->waitlock].ltail].qprev; prev != locadd[ptr->waitlock].lhead; prev = q[prev].qprev) {
            if (proctab[prev].pprio >= max)
                max = proctab[prev].pprio;
        }
        if (max < newprio) {
            int oldprio = ptr->pprio;
            if (max >= oldprio) {
                struct lentry *lptr = &locadd[ptr->waitlock];
                ptr->pinh = newprio;
                ptr->pprio = newprio;
                int in_locks = 0;
                while (in_locks < NPROC) {
                    if (lptr->allprocess[in_locks] == READ || lptr->allprocess[in_locks] == WRITE) {
                        chprio(in_locks, newprio);
                    }
                    in_locks++;
                }
            } else {
                int newp = all_all(ptr, oldprio, newprio);
            }
        } else if (max == newprio) {
            int oldprio = ptr->pprio;
            if (max > oldprio) {
                ptr->pprio = newprio;
            } else if (max <= oldprio) {
                int newp = all_all(ptr, oldprio, newprio);
            }
        } else if (max > newprio) {
            int oldprio = ptr->pprio;
            if (max <= oldprio) {
                int newp = all_all(ptr, oldprio, newprio);

            } else if (max <= oldprio) {
                ptr->pprio = newprio;
                ptr->pinh = newprio;
            }
        }
    } else {
        newprocChange(pid, newprio);
    }
}

SYSCALL chprio(int pid, int newprio) {
    pidd = pid;
    newprioo = newprio;
    STATWORD ps;
    struct pentry *pptr;

    disable(ps);
    if (isbadpid(pid) || newprio <= 0 ||
        (pptr = &proctab[pid])->pstate == PRFREE) {
        restore(ps);
        return (SYSERR);
    }
    procChange(pid, newprio);
    if (changed != 1) pptr->pprio = newprio;
    restore(ps);
    return (newprio);
}
