
.orig x3000

and r0, r0, #0		
and r4, r4, #0	
add r4, r4, #10	
add r4, r4, #10
lea r5, unal
ldw r5, r5, #0
ldw r1, r5, #0

halt

begin 	.fill xc000
zero 	.fill x0000
store  	.fill xc014
unal  	.fill xc017
hex4  	.fill x4000
.end
