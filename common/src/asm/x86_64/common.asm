bits 64
default rel

section .bss
global savedFloats

align 16
savedFloats: resb 512
