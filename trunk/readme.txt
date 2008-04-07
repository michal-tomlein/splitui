What does it do?
---------------------------------------------------------------
SplitUI is a tool which extends the functionality of Qt's uic.
It splits the compiled ui file (the header file) into a header
file and a cpp file, which can save a lot of time, especially
under Windows with MinGW. I decided to write a tool like this
because I was experiencing an extremely long compile time.
My uic-generated header file is 3000 lines long. If I make any
change to one of the files which #includes the header,
recompiling takes around 10 - 15 minutes. However, with
a separate cpp and header file, the compiler only needs to
recompile the UI when I make a change to it.
As you can see, this tool is intended primarily for developers,
people who compile downloaded source packages won't benefit
from it.

Compiling & Installing
---------------------------------------------------------------
To compile, open the terminal ("Qt 4.x.y Command Prompt" under
Windows) and type:
qmake -config release
make
In order to install SplitUI, do the following:
Under Windows, copy the executable (splitui.exe located in the
release directory) to the Qt bin directory, for example
C:\Qt\4.3.0\bin
Under Linux, copy the binary file (splitui) to /usr/bin. This
will require root privileges.

Using
---------------------------------------------------------------
Imagine you have 2 ui files in your project:
main_window.ui and about.ui
To enable the use of SplitUI, add the following into your
project file:

win32 {
	exists($(QTDIR)/bin/splitui.exe) {
		QMAKE_UIC     = splitui.exe
		SOURCES      += ui_main_window.cpp \
		                ui_about.cpp
	}
}
unix {
	exists(/usr/bin/splitui) {
		QMAKE_UIC     = splitui
		SOURCES      += ui_main_window.cpp \
		                ui_about.cpp
	}
}

You should now delete any uic-generated headers, in this case
ui_main_window.h and ui_about.h, run qmake and recompile.
Please note that it is safe to distribute project files like
this, because SplitUI will only be used if it is found in the
specified locations.