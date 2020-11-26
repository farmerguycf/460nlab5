
.orig x3000
add r0, r0, #7
halt
add r2, r0, r0
add r4, r4, #2
add r1, r1, #10
; ending status r0 = 7
;		r2 = 14
;		r4 = 2
;		r1 = 10
halt
.end
