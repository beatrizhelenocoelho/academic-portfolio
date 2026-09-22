
#    1B. Resolver com NOPs [1 ponto]
#Racio=24/45=0,53
#ipc=0,918

.data

# IMPORTANT: do not change this section

a:    .word 1, 5, 9, 2, 3, 4, 2, 8, 7, 5, 3, 3

b:    .word 2, 2, -1, 6, 5, 2, 3, 4, 2, 4, 7, 5

res1:  .word 0

res2:  .word 1



.text

# NOTE: Upon start, the Global-Pointer (gp=x3) points to the beginning of .data section

addi x12, x3, 0 #x12 = a's left index
nop
nop
addi x13, x12, 92 #x13 = b's right index
nop
nop
addi x11, x13, 4 #x11 = &res1
nop
nop
addi x14, x11, 4 #x14 = &res2
nop
lw x22, 0(x11) #x22 = res1
lw x23, 0(x14) #x23 = res2
li x16, 0 #x16 = i



while:
    nop
    nop
    sub x18, x13, x16 #x18 = &b[N-i-1]
    nop
    nop
    lw x18, 0(x18) #x18 = b[N-i-1]
    nop
    nop    
    blez x18, end #if !(b[N-i-1] > 0) exit loop
    nop
    nop
    add x19, x12, x16 #x19 = &a[i]
    nop
    nop
    lw x19, 0(x19) #x19 = a[i] 
    nop
    nop
    add x19, x19, x18 #x19 = a[i] + b[N-i-1]
    srai x20,x16,2    #i/4
    nop
    nop
    add x20, x18, x20 #x20 = b[N-i-1] + i
    nop
    add x22, x22, x19 #res1 += x19
    mul x23, x23, x20 #res *= x20
    addi x16, x16, 4 #i++
    jal x0, while



end:   

    nop
    nop
    sw x22, 0(x11)
    sw x23, 0(x14)

    

    

# Expected result: M[res1] = 79 = 4Fh

#                  M[res2] = 103219200 = 6270000h