# file: main.s

.extern handler, mathAdd, mathSub, mathMul, mathDiv

.global my_start

.global value1, value2, value3, value4, value5, value6, value7

.section my_code
my_start:
    ld $0xFFFFFEFE, %sp 
    ld $handler, %r1
    csrwr %r1, %handler

    int # software interrupt # 0

    ld $1, %r1
    push %r1
    ld $1, %r1
    push %r1
    call 0xF0000000 # 4
    st %r1, value2

    ld $2, %r1
    push %r1
    ld $1, %r1
    push %r1
    call mathAdd # 8
    st %r1, value3

    ld $7, %r1
    push %r1
    ld $11, %r1
    push %r1
    call mathSub # 12
    st %r1, value4

    ld $5, %r1
    push %r1
    ld $25, %r1
    push %r1
    call mathDiv # 16
    st %r1, value5

    ld $4, %r1
    push %r1
    ld $24, %r1
    push %r1
    call mathDiv # 20
    st %r1, value6

    ld value1, %r1
    ld value2, %r2
    ld value3, %r3
    ld value4, %r4
    ld value5, %r5
    ld value6, %r6
    ld value7, %r7

    halt # 24

.section my_data
value1:
.word value5
value2:
.word value6
value3:
.word value7
value4:
.word value8
value5:
.word value1
value6:
.word value2
value7:
.word value3
value8:
.word value4

.end
