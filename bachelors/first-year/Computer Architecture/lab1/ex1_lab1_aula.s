.data
a: .word 4
x: .word 4
z: .word 4
w: .word 4
y: .word 0

.text

lw x11, x
lw x12, z
lw x13, w
lw x14, a

li x15, 4

div x14, x14, x13 #a/w
add x14, x14, x12 #z+a/w
mul x14, x14, x11 # (z+a/w)*x
div x14, x14, x13 #(z+a/w)/w

div x13, x15, x13 #4/w
add x13, x13, x11 #x+4/w
mul x14, x14, x13 # ((z+a/w)/w)* (x+4/w)

la x10, y
sw x14, 0(x10)


