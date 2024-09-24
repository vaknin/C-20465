.entry DATA
.extern read
      mov #5, r0
      lea DATA, r1
LOOP: jsr read
      mov r2, *r1
      inc r1
      dec r0
      bne LOOP
      stop
DATA: .data 0, 0, 0, 0, 0
mov:  .string "Error"
.extern read