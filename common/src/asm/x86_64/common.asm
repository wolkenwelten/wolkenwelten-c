bits 64
default rel

section .data
align 16
global vecOneLiteral
vecOneLiteral: times 16 db 1

section .bss
global savedFloats

align 16
savedFloats: resb 512
