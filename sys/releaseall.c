#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

int max_lockss(int ldes) {
    int i, max = -9999;
    for (i = q[locadd[ldes].ltail].qprev; i != locadd[ldes].lhead; i = q[i].qprev) {
        if (proctab[i].pprio >= max)
            max = proctab[i].pprio;
    }
    return max;
}

/*int max_allss(int pid	){
	int g, all = 0;
	int al[100], count = 0;
	for(g = 0; g<NLOCKS; g++){
		if(proctab[pid].ltype[g] != -1){
			al[count] = g;
			count++;
		}
	}
	int full = 0, max = -9999;
		for(full = 0; full < count; full++){
			if(max < max_lockss(al[full]))	max = max_lockss(al[full]);	
		}
	return max;		
}*/

void changepriority(int ldes) {
    int prev, max = -9999;
    struct lentry *lptr = &locadd[ldes];
    int i, j;
    for (i = q[locadd[ldes].ltail].qprev; i != locadd[ldes].lhead; i = q[i].qprev) {
        if (max < proctab[prev].pprio)
            max = proctab[prev].pprio;
    }
    while (j < NPROC) {
        if (lptr->allprocess[j] == READ || lptr->allprocess[j] == WRITE) {
            if (max > proctab[j].pinh)
                chprio(j, proctab[j].pinh);
            else
                chprio(j, max);
        }
        j++;
    }
}

int changeotherlocks() {
    int a;
    int i, all = 0;
    int al[100], count = 0;
    i = 0;
    do {
        if (proctab[currpid].ltype[i] != -1) {
            al[count] = i;
            count++;
        }
        i++;
    } while (i < NLOCKS);
    int full = 0, max = -9999;
    while (full < count) {
        if (max < max_lockss(al[full]))
            max = max_lockss(al[full]);
        full++;
    }
    a = max;
}

int nextprocess(int ldes) {
    int back, all[50], allcount = 0, init = 0, id, idd;
    if (q[locadd[ldes].ltail].qprev != locadd[ldes].lhead) {
        if(!init) {
            id = q[locadd[ldes].ltail].qprev;
            idd = q[id].qprev;
        }
        if (proctab[id].ltype[ldes] != WRITE) {
            all[allcount] = id;
            allcount++;
            for(;q[id].qkey >= q[idd].qkey && idd != locadd[ldes].lhead;idd = q[idd].qprev){
                if (proctab[idd].ltype[ldes] != READ) {
                    int iddd = q[idd].qprev;
                    int tim = proctab[iddd].lreqtim - proctab[idd].lreqtim;

                    int write_lock_prio = q[idd].qkey;
                    for (;q[idd].qkey < q[iddd].qkey && tim <= 400 && iddd != locadd[ldes].lhead;iddd = q[iddd].qprev) {
                        if (proctab[iddd].ltype[ldes] == READ) {
                            all[allcount] = iddd;
                            allcount++;
                        }
                        if (iddd != locadd[ldes].lhead)
                            tim = proctab[iddd].lreqtim - proctab[idd].lreqtim;
                    }
                    break;

                } else {
                    all[allcount] = idd;
                    allcount++;
                }

            }
            int i = 0;
            while (i < allcount){
                proctab[all[i]].waitlock = -1;
                dequeue(all[i]);
                ready(all[i], RESCHNO);
                changepriority(ldes);
                locadd[ldes].ltype = READ;
                locadd[ldes].allprocess[all[i]] = READ;
                locadd[ldes].noofreaders++;


                i++;
            }
            return OK;
        } else {
            if (q[idd].qkey <= q[id].qkey ) {
                int count = 0;
                for (;q[id].qkey == q[idd].qkey && idd != locadd[ldes].lhead;idd = q[idd].qprev) {
                    if (proctab[idd].ltype[ldes] == READ) {
                        int tim = proctab[idd].lreqtim - proctab[id].lreqtim;
                        if (tim <= 400) {
                            all[allcount] = idd;
                            allcount++;
                            count++;
                            int iddd = q[idd].qprev;
                            int timm = proctab[iddd].lreqtim - proctab[id].lreqtim;
                            do {
                                all[allcount] = iddd;
                                allcount++;
                                iddd = q[iddd].qprev;
                                if (iddd == locadd[ldes].lhead)
                                    continue;
                                else
                                    timm = proctab[iddd].lreqtim - proctab[id].lreqtim;
                            }while(timm <= 400 && iddd != locadd[ldes].lhead);
                            break;
                        }
                    }

                }
                if (count != 0) {
                    int i = 0;
                    while(i < allcount) {
                        proctab[all[i]].waitlock = -1;
                        dequeue(all[i]);
                        changepriority(ldes);
                        ready(all[i], RESCHNO);
                        locadd[ldes].ltype = READ;
                        locadd[ldes].allprocess[all[i]] = READ;
                        locadd[ldes].noofreaders++;
                        i++;
                    }

                } else {
                    locadd[ldes].allprocess[id] = WRITE;
                    locadd[ldes].ltype = WRITE;
                    proctab[id].waitlock = -1;
                    dequeue(id);
                    changepriority(ldes);
                    ready(id, RESCHNO);
                }
                return OK;

            } else {
                proctab[id].waitlock = -1;
                dequeue(id);
                changepriority(ldes);
                ready(id, RESCHNO);
                locadd[ldes].ltype = WRITE;
                locadd[ldes].allprocess[id] = WRITE;
                return OK;

            }
        }
    }else {
        locadd[ldes].lstate = LNONE;
        return OK;
    }
}

int releaseall(int numlocks, int ldes1, ...) {
    STATWORD ps;
    struct lentry *lptr;

    disable(ps);

    int i, ldes, back = OK, condition = 0, nextid;
    i = 0;
    while(i < numlocks){
        ldes = *((int *) &ldes1 + i);
        lptr = &locadd[ldes];
       // lptr->lprio = max_lockss(ldes);
        int check = isbadlock(ldes) || lptr->lstate == LFREE || lptr->allprocess[currpid] == LNONE;
        if (!check) {
            lptr->allprocess[currpid] = LNONE;
            if (proctab[currpid].pinh != -1) {
                proctab[currpid].pinh += proctab[currpid].pprio;
                proctab[currpid].pprio = proctab[currpid].pinh - proctab[currpid].pprio;
                proctab[currpid].pinh -= proctab[currpid].pprio;
            }
            int a = changeotherlocks();
            if (a != -9999)
                chprio(currpid, a);
            if (lptr->ltype == WRITE)
                condition = 1;
            else if (lptr->ltype == READ) {
                lptr->noofreaders--;
                condition = lptr->noofreaders > 0 ? 0 : 1;
            }
            if (condition == 1) {
                int backs = nextprocess(ldes);
                if (back != -1)
                    back = backs;
            }

        } else {
            restore(ps);
            back = SYSERR;
        }
        i++;
    }
    resched();
    restore(ps);
    return (back);
}
