pushc 3
pushc 5
pushc 1
sub
popreg 0
popreg 1
pushreg 1
pushreg 0
popreg 0
pushreg 1
add
pushreg 0
pushc 1
sub
brz 2
jmp -7
pop
popout
halt
