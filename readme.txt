Hi C++ programmers!

This is the second version of my C++ library for OS/2.
The example program will show, how to program OS/2 PM without using
any global variables and with using inheritance.
I get sick, if I see the lot of programs, written in C
- unsing one global context.
I've found a lot of bugs and changed a lot of stuff.
The example program for calculating a mandelbrot fractal image
has changed. Now you can zoom in rotated and you can select wether
you want to use a new window or the old one.
But the best think is, that you can use all machines in a network
to help calculate your image - that is distributed computing or multiprocessing.
I would be glad, if some one implements color palette animation.
I've already done this in a old version for the antiquated DOS -
is looks nice. I've used a random number generator
to change every palette entry, while shifting the palette entries.

Also an interesting idea is to put the iteration stuff into a DLL.
The user should be able to write his own DLL.
This dll should contain some vectors,
one including the addresses of iterations functions,
one including names for this iterations methods.
The user should be able to select such a name from a menu
- this is the way to select a iteration method.

There are two executables, server.exe and client.exe.
You can use client.exe without server.exe if there is only one machine.
server.exe should be started on every machine available (via autoexec.bat ?)
in the following way:

	server.exe \pipe\uniqueName 2> uniqueName.out

or

	server.exe \pipe\uniqueName 2> nul

if you don't want the error output saved.

"uniqueName" should be a name which is unique in the network.
This names should be collected in a file in the following way:

\pipe\uniqueName1
\pipe\uniqueName2
\pipe\uniqueName3
\pipe\uniqueName4
\pipe\uniqueName5
\pipe\uniqueName6

The name of this file should be passed on the commandline to client.exe.
If there is such a file for client.exe, client.exe will no more calculate
the image itself, but instead of this send requests to the servers.

I'VE NOT TESTED THIS IN A NETWORK - I HOPE THIS WORKS ON A NETWORK.
I would be pleased to hear something about this.
The server is able to services more then one client.
It was a nice experience, to write such stuff without using the antiquated
fork() unix system call, but instead of this using threads.

The client uses the following files:

object.cpp
chain.cpp
tree.cpp
process.cpp
pmprocess.cpp
apfel.cpp
apfel.rc

the server uses the following files:

object.cpp
chain.cpp
tree.cpp
process.cpp
server.cpp
server.def

Yes I know that code which is used by more then once executable
should be collected into a shared library or dynamic link library.
You can do this. if you want.

This is the class hierarchy:

object
  mtxObject
    chain
    chainElement
  construct
  string


    chainElement/string
      stringChainElement

    chain/chainElement
      tree

  construct/chainElement
    thread
      pmThread
	pmWindowThread
	pmMsgThread

  construct/chain
    process
      pmProcess

    tree
      windowTree
	processWindow

Some documentation is done in the include (*.h) files and also in the .cpp files.

I've used Borlands C++, caused of the fast compilation
and good debugging capabilities.
The optimzation of this compiler is quit poor - compared to EMX.
EMX uses all coprocessor registers, keeps local double variables in the
coprocessor. Borland C++ fetches local double variables always from memory.
I did not test the speed difference - but I would be pleased
if there is someone how does and make the values public.
Server.exe crashes if you compile it with optimization option "Fastest Code".
This is caused of that the function calculate() does the following:

	- allocates space on the stack for local variables
	- saves some registers on the stack
	- makes alloca
	- calculates the image
	- restores the saved registers
		(BUT THE STACK POINTS CURRENTLY TO THE MEMORY AREA RETURNED
		FROM alloca!!!)

It seems that the client contains the same bug,
if you compile using optimization.
I think I'll use the EMX for product versions.
The included version is the Borland version with optimizing option
"minimal optimizing".
The provided file c2mt.dll is the dll version of Borlands c library
(I don't know why Borland provides a static version of this library -
may be for people which own machines with to much memory.)

If someone wants to use this library, he can do it.
I don't claim anything.
And of course there is no guarantee for anything in this library or the example
programs.


If someone want to get some information or want to send some suggestions:
This is my telefon number (only voice)
	49 7345 22870
	(remember the time difference - this is in germany)
and EMAIL:
	peter@anacad.de
and CompuServe:
	100114,351


Peter Foelsche

Happy Multithreading
