Sockcmd Library
---------------
A simple socket based RPC implementation.
It's useful when binder is not available, e.g., Recovery environment.
And sometimes, binder is just too complex for a simple use case.

It refers to "installd.c" in Android heavily.


Howto Run the Demo
------------------
1) Copy the code to the Android source tree
   e.g., the PRODUCT folder.

2) Build the demo
   $ cd .../sockcmd
   $ mm
   demosockcmd-client/demosockcmd-server will be generated.

3) Push the binaries to the target
   $ adb push .../demosockcmd-client /data
   $ adb push .../demosockcmd-server /system/bin

4) Define a new service in the init.rc
   Add below lines at the end of the init.rc.
     service demosockcmd /system/bin/demosockcmd-server
         class main
         socket demosockcmd stream 660 root root
         user root
         group system
         oneshot

   Usually, init.rc is in the readonly romfs, maybe you need to regenerate
   and reburn the image in which init.rc is included.

5) Reboot the target and run the demo
   $ /data/demosockcmd-client

   You will see below output on the console.
     #0
     Ping: 0
     3 + 7 = 10
     #1
     Ping: 0
     3 + 7 = 10
     ...

   And you can check the debug log by logcat.
   $ logcat -v time | grep sockcmd

