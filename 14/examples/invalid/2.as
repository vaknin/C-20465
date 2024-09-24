.entry MAIN
.extern func
MAIN: mov r1, #23
      jsr func
      mov r2, r1
      add r2, #5
      clr r3, r4
LOOP: inc r3
      cmp r3, #10
      bne LOOP
      red r9
      jmp MAIN
1DATA: .data -1, 0, 1
STR: .string "test"