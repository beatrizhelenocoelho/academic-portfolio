#res = (A[0] * B[0] -A[1] *B[1]) +C
.data
a: .half 1, 2
b: .word -4, -2 
c: .byte 2
res: .zero 4

.text 
la x10,a
la x11,b
lb x13,c

lh x14, 0(x10)#a[0]
lh x15, 2(x10)#a[1], 2 pois é half

lw x16, 0(x11)#b[0]
lw x17, 4(x11)#b[1]

mul x14, x14, x16 #a[0] * b[0]
mul x15, x15, x17 #a[1] * b[1]
sub x14, x14, x15 # a[0] * b[0] - a[1] * b[1]
add x14, x14, x13 #(a[0] * b[0] - a[1] * b[1]) +c


la x7, res
sw x14, 0(x7)

