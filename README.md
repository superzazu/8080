# 8080

A complete emulation of the Intel 8080 processor written in C99. Goals:

- accuracy: it passes all test roms at my disposal
- readability
- portability: tested on debian 8 with gcc 5, macOS 10.12+ with clang and emscripten 1.39

You can see it in action in my [Space Invaders emulator](https://github.com/superzazu/invaders).

## Running tests

You can run the tests by running `make && ./i8080_tests`. The emulator passes the following tests:

- [x] TST8080.COM
- [x] CPUTEST.COM
- [x] 8080PRE.COM
- [x] 8080EXM.COM

The test roms (`cpu_tests` folder) are taken from [here](http://altairclone.com/downloads/cpu_tests/) and take approximately 30 seconds on my computer (MacBook Pro mid-2014) to run.

The standard output is as follows:

```
*** TEST: cpu_tests/TST8080.COM
MICROCOSM ASSOCIATES 8080/8085 CPU DIAGNOSTIC
 VERSION 1.0  (C) 1980

 CPU IS OPERATIONAL
*** 651 instructions executed on 4924 cycles (expected=4924, diff=0)

*** TEST: cpu_tests/CPUTEST.COM

DIAGNOSTICS II V1.2 - CPU TEST
COPYRIGHT (C) 1981 - SUPERSOFT ASSOCIATES

ABCDEFGHIJKLMNOPQRSTUVWXYZ
CPU IS 8080/8085
BEGIN TIMING TEST
END TIMING TEST
CPU TESTS OK

*** 33971311 instructions executed on 255653383 cycles (expected=255653383, diff=0)

*** TEST: cpu_tests/8080PRE.COM
8080 Preliminary tests complete
*** 1061 instructions executed on 7817 cycles (expected=7817, diff=0)

*** TEST: cpu_tests/8080EXM.COM
8080 instruction exerciser
dad <b,d,h,sp>................  PASS! crc is:14474ba6
aluop nn......................  PASS! crc is:9e922f9e
aluop <b,c,d,e,h,l,m,a>.......  PASS! crc is:cf762c86
<daa,cma,stc,cmc>.............  PASS! crc is:bb3f030c
<inr,dcr> a...................  PASS! crc is:adb6460e
<inr,dcr> b...................  PASS! crc is:83ed1345
<inx,dcx> b...................  PASS! crc is:f79287cd
<inr,dcr> c...................  PASS! crc is:e5f6721b
<inr,dcr> d...................  PASS! crc is:15b5579a
<inx,dcx> d...................  PASS! crc is:7f4e2501
<inr,dcr> e...................  PASS! crc is:cf2ab396
<inr,dcr> h...................  PASS! crc is:12b2952c
<inx,dcx> h...................  PASS! crc is:9f2b23c0
<inr,dcr> l...................  PASS! crc is:ff57d356
<inr,dcr> m...................  PASS! crc is:92e963bd
<inx,dcx> sp..................  PASS! crc is:d5702fab
lhld nnnn.....................  PASS! crc is:a9c3d5cb
shld nnnn.....................  PASS! crc is:e8864f26
lxi <b,d,h,sp>,nnnn...........  PASS! crc is:fcf46e12
ldax <b,d>....................  PASS! crc is:2b821d5f
mvi <b,c,d,e,h,l,m,a>,nn......  PASS! crc is:eaa72044
mov <bcdehla>,<bcdehla>.......  PASS! crc is:10b58cee
sta nnnn / lda nnnn...........  PASS! crc is:ed57af72
<rlc,rrc,ral,rar>.............  PASS! crc is:e0d89235
stax <b,d>....................  PASS! crc is:2b0471e9
Tests complete
*** 2919050698 instructions executed on 23803381171 cycles (expected=23803381171, diff=0)

```

## Resources used

- [CPU instructions](http://nemesis.lonestar.org/computers/tandy/software/apps/m4/qd/opcodes.html) and [this table](http://www.pastraiser.com/cpu/i8080/i8080_opcodes.html)
- [MAME i8085](https://github.com/mamedev/mame/blob/6c0fdfc5257ca20555fbc527203710d5af5401d1/src/devices/cpu/i8085/i8085.cpp)
- [thibaultimbert's Intel8080](https://github.com/thibaultimbert/Intel8080/blob/master/8080.js) and [begoon's i8080-js](https://github.com/begoon/i8080-js)
