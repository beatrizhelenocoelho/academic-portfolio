#if (res>0)
#res=1
#else
#res=-1

.data 
res: .word 5

.text
#carrega para um registo o endereço da variavel res 
la x10, res 
#guardar o valor de x11 no endereco de x10
lw x11, 0(x10) 

sgtz x15,x11 #se res>0, guarda 1, senão guarda 0 
beq x15,x0, else #se for 0, else
li x11, 1 #res=1
j endif

else: li x11, -1 #é negativo, logo res=-1
endif:
 sw x11, 0(x10)