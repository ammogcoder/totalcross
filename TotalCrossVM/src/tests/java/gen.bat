@echo off
javac GenTestCalls.java -source 1.2 -target 1.1
cd P:\gitrepo\TotalCross\TotalCrossVM\src
java -classpath p:\gitrepo\TotalCross\TotalCrossSDK\output\eclipse;P:\gitrepo\TotalCross\TotalCrossVM\src\tests\java GenTestCalls .
pause
copy tc_tests.c tests
del tc_tests.c