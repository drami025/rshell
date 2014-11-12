#RSHELL

Licensing Information: READ LICENSE
---

Project source can be downloaded from https://github.com/drami025/rshell.git
---


Author & Contributor List
----
Daniel Ramirez

All other known bugs and fixes can be sent to drami025@ucr.edu

Reported bugs/fixes will be submitted to correction.



Synopsis
---
The purpose of this program is to create and execute a simple command shell, which is called rshell.

Rshell can:

* Execute most of the commands of a normal BASH shell 
* Execute commands chained together using connectors
* Exit from rshell to your normal BASH shell using command `` exit ``.

As of now, rshell cannot built in BASH commands, like `` cd ``.

File List
----

```
.:

Makefile

LICENSE

README.md

./src

./tests
```
```
./src:

rshell.cpp
ls.cpp
cp.cpp

```
```
./test:

exec.script
```

How to run the file
----

In order to run this program, you must have the Boost library installed. If you are using a Linux Operating System, run this command to install this library:
```
$ sudo apt-get install libboost-all-dev
```


After cloning repository into your local machine
---
Run these commands from the local repository's root directory (.../rshell/):
```
$ make

$ bin/rshell
```
To run ``cp`` or ``ls`` run these commands after ``make``:

```
$bin/cp
$bin/ls
```

Bugs & Fixes
---

* Typing in more than one connector with no command (i.e, typing just ``;;``  and hitting ``enter``)  will only give you the proper error for one connector. 
* Programs do not run in the background.
* Typing in ``$ ls && && && ls`` will actually run ``ls`` twice, as opposed to just returning an error.
* Typing in a command and then a connector like ``&&`` or ``||`` will actually cause the command before the connector to execute, as opposed to waiting till a second command to inputted. 
* Typing in ``ls | echo test`` does not cause piping as of now.
* Typing in something like ``ls &&& ls`` will actually run the commands while still giving an error for an unexpected token, as opposed to just returning an error in a normal BASH shell.
* Quote features do not function as they are suppose to. For example, if you type in ``echo "hello        world"`` you would receive ``hello       world`` on a normal BASH shell. On this shell, you will see ``hello world``.
* Formatting feature is still not up to par with the BASH shell's ls
* Some folders do not appear alphabetically when using recursive flag ``-R``
