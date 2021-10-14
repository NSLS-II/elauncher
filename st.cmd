#!bin/linux-x86_64/elauncher

dbLoadDatabase "dbd/elauncher.dbd"
elauncher_registerRecordDeviceDriver pdbbase

## Load record instances
dbLoadRecords "db/elauncher.db"


#- Set this to see debug messages from elauncher
#var launcherVerbose 1

#- Run this to trace the stages of iocInit
#traceIocInit

iocInit

