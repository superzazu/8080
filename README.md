# 8080

8080 is a complete emulation of the Intel 8080 processor written in C. Its goals are:

- accuracy
- readability
- portability

# Running tests

You can run the tests by running `make && ./i8080_tests`. The emulator passes the following tests:

- [x] TST8080.COM
- [x] CPUTEST.COM
- [x] 8080PRE.COM
- [x] 8080EX1.COM

The test roms (`cpu_tests` folder) are taken from [here](http://altairclone.com/downloads/cpu_tests/) and take approximately 40 seconds on my computer (MacBook Pro mid-2014) to run.

The output is as follows:

```
*******************
MICROCOSM ASSOCIATES 8080/8085 CPU DIAGNOSTIC
 VERSION 1.0  (C) 1980

 CPU IS OPERATIONAL
Jumped to 0x0000 from 0x06BA

*******************

DIAGNOSTICS II V1.2 - CPU TEST
COPYRIGHT (C) 1981 - SUPERSOFT ASSOCIATES

ABCDEFGHIJKLMNOPQRSTUVWXYZ
CPU IS 8080/8085
BEGIN TIMING TEST
END TIMING TEST
CPU TESTS OK

Jumped to 0x0000 from 0x3B25

*******************
8080 Preliminary tests complete
Jumped to 0x0000 from 0x032F

*******************
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
Jumped to 0x0000 from 0x0137

Executed in 37.00s.
```

# Resources used

- [CPU instructions](http://nemesis.lonestar.org/computers/tandy/software/apps/m4/qd/opcodes.html) and [this table](http://www.pastraiser.com/cpu/i8080/i8080_opcodes.html)
- [MAME source code](https://github.com/mamedev/mame/blob/6c0fdfc5257ca20555fbc527203710d5af5401d1/src/devices/cpu/i8085/i8085.cpp)
- [thibaultimbert's Intel8080](https://github.com/thibaultimbert/Intel8080/blob/master/8080.js) and [begoon's i8080-js](https://github.com/begoon/i8080-js)
