#RSHELL
**---**

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


Bugs & Fixes
---
