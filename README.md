scpspt
========
Is a simple cross-platform serial port terminal, thus the name...  
Should at least work on Windows 7 and Ubuntu 14.04. If you make it compile in MacOS, pull requests are welcome.

License
========
[BSD-2-Clause](http://opensource.org/licenses/BSD-2-Clause), see [LICENSE.md](LICENSE.md).  
The icons are from or are based on icons from the [Tango Icon Library](http://tango.freedesktop.org/Tango_Icon_Library) v0.8.90, which seems to be PD. Thanks a bunch!

Building
========
**Use CMake**

<pre>
cd scpspt
cmake .
make
</pre>

The Qt framework version 5.3 or higher is required for GUI and serial port functionality. Make sure your CMAKE_PREFIX_PATH is set to the proper Qt installation or use the CMake GUI to configure (actually simpler).  
G++ 4.7 or higher (for C++11) will be needed to compile scpspt. For installing G++ 4.7 see [here](http://lektiondestages.blogspot.de/2013/05/installing-and-switching-gccg-versions.html).

Overview
========
![GUI overview](scpspt_gui.png?raw=true)

FAQ
========
**Q:** I'm on Windows get a linker error after setting the subsystem to "Windows" (/SUBSYSTEM:WINDOWS) to hide the console...  
**A:** Set your entry point to "mainCRTStartup" (linker settings).  

**Q:** I'm on linux and I can not access the serial port somehow...  
**A:** You might need to add your USERNAME to the dialout group: ```sudo usermod -a -G dialout USERNAME```.  

I found a bug or have a suggestion
========
The best way to report a bug or suggest something is to post an issue on GitHub. Try to make it simple, but descriptive and add ALL the information needed to REPRODUCE the bug. **"Does not work" is not enough!** If you can not compile, please state your system, compiler version, etc! You can also contact me via email if you want to.