TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================


LIBRARY_IOC = elauncherSupport
DBD += elauncherSupport.dbd

#elauncher.c: device support for elauncher
elauncherSupport_SRCS += elauncher.c

#=============================
# Build the IOC application

PROD_IOC = elauncher

DBD += elauncher.dbd

# elauncher.dbd will be made up from these files:
elauncher_DBD += base.dbd
elauncher_DBD += elauncherSupport.dbd
elauncher_DBD += system.dbd

elauncher_LIBS += elauncherSupport

# elauncher_registerRecordDeviceDriver.cpp derives from elauncher.dbd
elauncher_SRCS += elauncher_registerRecordDeviceDriver.cpp

# Build the main IOC entry point on workstation OSs.
elauncher_SRCS_DEFAULT += elauncherMain.cpp
elauncher_SRCS_vxWorks += -nil-

# Finally link to the EPICS Base libraries
elauncher_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

