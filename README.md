Device support for easy script launcher: one can run a Python (or shell, or anything you can use in a Linux terminal) script inside an IOC through a bo record.


You may need to create a file "RELEASE.local" in the directory "configure" to redefine EPICS_BASE.

Type "make" to build this application as a standalone IOC (or you can include this application as a support module to your IOC). 

You may need to replace the first line with "#!bin/linux-x86/elauncher" in "st.cmd" if you use 32-bit Linux (you may also need to type "chmod +x st.cmd" if you see permission problems). 

Type "./st.cmd" to start the IOC for a quick test.

The source codes were originally developed by Michael Davidsaver.

