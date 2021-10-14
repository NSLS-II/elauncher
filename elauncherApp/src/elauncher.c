
#include <stdlib.h>
#include <string.h>

#include <epicsString.h>
#include <epicsAssert.h>
#include <cantProceed.h>
#include <epicsThread.h>
#include <epicsMutex.h>
#include <epicsEvent.h>
#include <callback.h>
#include <epicsExit.h>

#include <errlog.h>

#include <dbStaticLib.h>
#include <alarm.h>
#include <dbEvent.h>
#include <dbAccess.h>
#include <recSup.h>
#include <devSup.h>
#include <alarm.h>
#include <boRecord.h>
#include <recGbl.h>

int launcherVerbose = 0;

typedef struct {
    epicsMutexId lock;

    epicsThreadId thread;

    epicsEventId start;

    boRecord *prec;

    unsigned int running:1;
    unsigned int done:1;

    char *cmd;

    int ret;
} work;

static void worker(void*);
static void cleanup(void*);

static long init_record(boRecord* prec)
{
    return 2;
}

static long add_record(dbCommon* pcom)
{
    DBENTRY entry;
    boRecord *prec=(boRecord*)pcom;
    work *priv=prec->dpvt;

    if(!priv) {

        priv=mallocMustSucceed(sizeof(*priv), "elauncher malloc");
        prec->dpvt = priv;

        priv->lock = epicsMutexMustCreate();
        priv->start = epicsEventMustCreate(epicsEventEmpty);
        priv->prec = prec;
        priv->cmd = NULL;
        priv->ret = -1;
        priv->running = 0;
        priv->done = 0;

        priv->thread = epicsThreadMustCreate("launcher",
                                             epicsThreadPriorityMedium,
                                             epicsThreadGetStackSize(epicsThreadStackSmall),
                                             &worker, priv);

        epicsAtExit(&cleanup, priv);
    }

    assert(prec->out.type == INST_IO);

    dbInitEntry(pdbbase, &entry);

    dbFindRecord(&entry, prec->name); /* can't fail */

    if(dbFindInfo(&entry, "cmd")==0) {
        priv->cmd = epicsStrDup(dbGetInfoString(&entry));
    } else {
        priv->cmd = epicsStrDup(prec->out.value.instio.string);
    }
    if(launcherVerbose || !priv->cmd || strlen(priv->cmd)==0)
        errlogPrintf("%s: cmd '%s'\n", prec->name, priv->cmd);
    assert(priv->cmd);

    dbFinishEntry(&entry);

    return 0;
}

static long del_record(dbCommon* pcom)
{
    boRecord *prec=(boRecord*)pcom;
    work *priv=prec->dpvt;
    if(!priv)
        return -1;

    assert(prec->out.type == INST_IO);

    if(priv->running)
        return -1;

    free(priv->cmd);
    priv->cmd = NULL;

    return 0;
}

static long write(boRecord* prec)
{
    work *priv=prec->dpvt;
    if(!priv || !priv->cmd)
        return -1;

    prec->udf = 0;

    if(!prec->pact) {

        /* ensure 1-> 0 transition on completion */
        prec->mlst = prec->val;
        prec->val = 1;
        db_post_events(prec, &prec->val, DBE_VALUE|DBE_LOG);

        priv->running = 1;
        epicsEventSignal(priv->start);

        prec->pact = TRUE;
        if(launcherVerbose)
            errlogPrintf("%s: start\n", prec->name);

        return 0;

    } else {
        prec->pact = FALSE;

        if(WIFSIGNALED(priv->ret)) {
            recGblSetSevr(prec, WRITE_ALARM, INVALID_ALARM);

        } else if(WEXITSTATUS(priv->ret)) {
            recGblSetSevr(prec, WRITE_ALARM, MAJOR_ALARM);
        }

        prec->mlst = prec->val;
        prec->val = 0;
        if(launcherVerbose)
            errlogPrintf("%s: done\n", prec->name);
    }

    return 0;
}

static void worker(void *raw)
{
    work *priv=raw;

    while(1) {
        char *cmd;
        int ret;

        epicsEventMustWait(priv->start);

        if(priv->done) {
            priv->done = 0;
            return;
        }

        dbScanLock((dbCommon*)priv->prec);
        if(launcherVerbose)
            errlogPrintf("%s: run\n", priv->prec->name);
        cmd = epicsStrDup(priv->cmd);
        dbScanUnlock((dbCommon*)priv->prec);

        ret = system(cmd);

        dbScanLock((dbCommon*)priv->prec);
        if(launcherVerbose)
            errlogPrintf("%s: complete\n", priv->prec->name);

        if(WIFSIGNALED(priv->ret)) {
            unsigned int d=WTERMSIG(ret);
            errlogPrintf("%s: command terminated with signal %u\n   '%s'\n",
                         priv->prec->name, d, cmd);
        }
        free(cmd);

        priv->ret = ret;
        priv->running = 0;

        (*priv->prec->rset->process)(priv->prec);

        dbScanUnlock((dbCommon*)priv->prec);
    }
}

static void cleanup(void *raw)
{
    work *priv=raw;

    priv->done = 1;

    while(priv->done)
        epicsEventSignal(priv->start);
    if(launcherVerbose)
        errlogPrintf("%s: worker stopped\n", priv->prec->name);
}

static
dsxt extLauncher =
{
    &add_record, &del_record
};

static long init(int pass)
{
    if(pass==0)
        devExtend(&extLauncher);
    return 0;
}


static
struct {
    dset com;
    DEVSUPFUN write;
} devLauncher =
{
{5, NULL,
    (DEVSUPFUN) &init,
    (DEVSUPFUN) &init_record,
    NULL
},
    (DEVSUPFUN) &write
};

#include <epicsExport.h>

epicsExportAddress(dset, devLauncher);
epicsExportAddress(int, launcherVerbose);
