Lemvos is supposed to do hydro static stability calculations on 3D boat models.
It is not finished and needs lots of things to do.

Lemvos can create propriatery 3D models using the tool gc by a parameter file <lemvos>/models/*.config.
These models can be viewed by gv wich calculates and shows some stability data.

*** License

This version of lemvos is published under the GPL-3.0. You can find the newest version of the GPL at gnu.org.


*** Warranty

There is not any at all! Please don't trust the data unles you know what you do.
I don't want your boat to sink :-) because I did not do much testing here and I know there is flaws in lemvos.


*** Building Lemvos

I have only tested on Linux. A windows build is not yet possible but it should be possible to port to windows.

Compile with gcc 5.3.0 (the one I am using) using:
> cd <lemvos>/src
> make
...

This will create two tools <lemvos>/bin/gc and <lemvos>/bin/gv 
and run a test creating <lemvos>/models/lemvos.model 
using <lemvos>/models/lemvos.config to create a model by gc
and run gv to view <lemvos>/models/lemvos.model

In case you fail building (may be some new compiler) you can edit <lemvos>/src/config.mk.
Newer compiler will fail with the compiler flags and create eventualy warning. 
You can disable all of these in config.mk.

In <lemvos>/lib you can find some static binary libs needed to compile lemvos.


*** Author

Andrej Georgi: I am a free software engineer in germany and I love sailing and boats. 
You can reach out for me by axgeorgi@gmail.com
Let me know if you like lemvos.