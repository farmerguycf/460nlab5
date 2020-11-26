;isr
.orig x1200		
add r6, r6, #-2 ;push regs
stw r1, r6, #0
add r6, r6, #-2
stw r2, r6, #0
add r6, r6, #-2
stw r3, r6, #0

ldw r3, r6, #0; pop regs
add r6, r6, #2;
ldw r2, r6, #0;
add r6, r6, #2;
ldw r1, r6, #0;
add r6, r6, #2;		
		
rti ;return
.end
