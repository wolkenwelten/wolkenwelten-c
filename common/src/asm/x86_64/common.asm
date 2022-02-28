bits 64
default rel

section .data
align 16
global vecTwoLiteral
vecTwoLiteral: times 16 db 2

section .bss
global savedFloats

align 16
savedFloats: resb 512
