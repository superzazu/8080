This folder contains 8080 CPU test programs that run under CP/M

TST8080
    8080/8085 CPU Diagnostic, version 1.0, by Microcosm Associates
    (Kelly Smith test)

8080PRE
    Preliminary test for 8080/8085 CPU Exerciser by Ian Bartholomew
    and Frank Cringles.

8080EXER
    8080/8085 CPU Exerciser by Ian Bartholomew and Frank Cringles.
    This is a very thorough test that generates a CRC code for each
    group of tests. The program will say "Error" for every test, but
    it's not actually an error. Instead, compare the reported CRC
    with results from tests against real silicon. See 8080EXER.PNG in
    this directory and http://www.idb.me.uk/sunhillow/8080.html. The
    full test takes several hours. The "aluop <b,c,d,e,h,l,m,a>" section
    takes especially long.

8080EXM
    8080/80805 Exerciser "M"odified with the correct result CRCs in
    the program so it can display "pass" or "fail" correctly.

CPUTEST
    SuperSoft Associates CPU test from the Diagnostic II
    suite. When it displays "ABCDEF..." those are actually 
    indications of test that have passed. Additional testing
    occurs during the "Begin Timing Test" and "End Timing Test"
    period. On a 2MHz 8080, the timing test period lasts about
    two minutes.
