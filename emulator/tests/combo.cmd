pushc 1
pushc 2
jmp 2
pushc 100
pushc 3
pushc 1
pushc 2
add
pushc 1
brlz -1
pushc -2
pushc 1
add
brlz -2
brz 2
pushc 10
halt
