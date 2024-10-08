# CUnit
CUnit is an incomplete unit testing library for C/C++. The motivation for creating this library is to provide something more compact than [Unity](https://github.com/ThrowTheSwitch/Unity/tree/master) while providing more features than something like [Xtal](https://github.com/PalmeseMattia/Xtal). Here are the characteristics I'm looking for in a C unit testing library:
1. Runs on POSIX compliant operating systems with GCC or Clang.
2. Provides a simple building mechanism that remains build system agnostic.
3. Provides the ability to separate unit tests along arbitrary boundaries.
4. Provides the ability to run a subset of unit tests through simple command line arguments.
5. Proivides the option to output the results in an easily parsable format (probably JSON).
6. Runs a customizable amount of testing suites concurrently.
7. Runs individual tests in testing suites serially.
There is probably something out there that loosely fulfills these requirements, but writing my own allows me to tailor it to my exact needs.
## Arguments
These are the command line arguments I'd like to add to alter the program's behavior.
+ *--no-color* : This would be nice if you are redirecting the output to a file.
+ *--interactive* : This would allow you to reprint certain suites and/or tests after the tests finish.
+ *-jN* : N is the number of suites that can run concurrently.
+ *-s <comma separated suite names>* : Only run the suites in the list.
+ *--suite=<suite name> <command separated test names>* : In the suite *suite name*, only run the tests in the list of test names. This can be passed multiple times.
+ *-f <'default' or 'json'>* : Alter how the results are output.
