
.orig x3000
add r0, r0, #7
and r2, r0, r0
add r4, r4, #2
xor r1, r1, #10
shf r2, r2, #3
lea r2, test
lea r2, r2, #0
halt
.end

test .fill 0x3050

; ending status r0 = 7
;		r2 = 14
;		r4 = 2
;		r1 = 10
halt
.end
