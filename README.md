Device support for easy script launcher: one can run a Python (or shell, etc.) script inside an IOC through a bo record.


You may need to create a file "RELEASE.local" in the directory "configure" to redefine EPICS_BASE.

Type "make" to build this application as a standalone IOC. 

You may need to modify the fist line in "st.cmd" if you use 32-bit Linux. You may also need to type "chmod +x st.cmd" to change its permission. 

Type "./st.cmd" to start the IOC for a quick test.

The source codes were originally developed by Michael Davidsaver.
 
