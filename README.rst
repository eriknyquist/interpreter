Interpreter project
-------------------

An interpreted programming language that relies on a stack-based virtual machine
to execute compiled bytecode.

Source code is compiled into tightly-packed binary instructions that describe
a sequence of atomic operations understood by the "virtual machine". The virtual
machine itself is just a program that reads these instructions, and the data
encoded with them, and performs the operations they describe.

Some inspiration and advice taken from `this excellent guide <http://craftinginterpreters.com/>`_


Project file structure
----------------------

* `source/runtime`: contains code related to interpreting and executing bytecode.
  Doing most of my work in here right now.

* `source/backend`: contains code related to generating bytecode
* `source/frontend`: contains code related to  parsing source files

