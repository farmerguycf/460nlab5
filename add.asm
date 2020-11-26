.orig x3000

and r0, r0, #0		
and r4, r4, #0	
add r4, r4, #10	
add r4, r4, #10
lea r5, hex4
ldw r5, r5, #0
and r2, r2, #0
add r2, r2, #1
stb r2, r5, #0
lea r5, begin		
ldw r5, r5, #0	

rep ldb r1, r5, #0	
    add r5, r5, #1
    add r0, r0, r1	
    add r4, r4, #-1	
    brp rep

lea r5, store
ldw r5, r5, #0
stw r0, r5, #0
lea r3, zero		
ldw r3, r3, #0		
stw r0, r3, #0	
jmp r0

halt

begin 	.fill xc000
zero 	.fill x0000
store  	.fill xc014
unal  	.fill xc017
hex4  	.fill x4000
.end
