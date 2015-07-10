
## libusb

http://libusb.info/

Download latest file: libusb-1.0.19-rc1-win.7z

1. Copy files compiled in previous step to their appropriate locations

Copy libusb-1.0.19-win\include\libusb-1.0\libusb.h  to C:\MingW\include\
Copy libusb-1.0.19-win\MinGW32\dll\libusb-1.0.dll.a to C:\MingW\lib\
Copy libusb-1.0.19-win\MinGW32\static\libusb-1.0.a  to C:\MingW\lib\
Copy libusb-1.0.19-win\MinGW32\dll\libusb-1.0.dll  	to C:\MingW\bin\

2. MinGW C Linker libraries add: usb-1.0 

 Eclipse properties> C/C++ Build> Settings> MinGW C Linker>
    Libraries: usb-1.0
    
## boost library
    
  The steps I used were

- install MinGW and Msys (bash etc) using mingw-get-setup (the easy way), add bin/ to path
- install the Windows Driver Kit (for W7 I used WDK 7) -- GRMWDK_EN_7600_1.ISO
- downloading the ISO image and extracting the files with WinRAR worked for me
  the installer advises against installing the DSF, so skip that
   add the directories of ML64.exe and ML.exe to the path (both required AFAIK)
C:\Windows\WinDDK\7600.16385.1\bin\x86\amd64;C:\Windows\WinDDK\7600.16385.1\bin\x86


C:\boost_1_52_0> bootstrap.bat mingw
...
C:\boost_1_52_0> b2 toolset=gcc

I use Eclipse IDE to program in C/C++. So in order to compile a project that uses Boost libraries, you will need to add this directory to your project library includes under C/C++ Build -> Settings section of your project properties window:

C:\Program Files (x86)\boost\boost_1_53_0

and this one to your MinGW linker libraries search paths:

C:\Program Files (x86)\boost\boost_1_53_0\stage\lib

You will also need to add the individual libraries to the list of used library files. Those libraries are the names of the files in the C:\Program Files (x86)\boost\boost_1_53_0\stage\lib directory, without the lib prefix and the .a postfix in the file name. So for an example, if you want to use the libboost_atomic-mgw46-mt-1_53.a library, you will need to add boost_atomic-mgw46-mt-1_53 to your linker libraries list. Once you add all the needed libraries, you will be able to compile a project using Boost.
