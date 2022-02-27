bits 64
default rel

section .bss
extern savedFloats

section .text
global  lightBlurXSSE

lightBlurXSSE:
	fxsave [savedFloats]
	push rbx
	push rcx
	push rdx
	push rdi

%ifidn __?OUTPUT_FORMAT?__, win64
	mov rdi, rcx
%endif

	xorps xmm15, xmm15
	mov al, 02
	movd xmm0, eax
	vpbroadcastb xmm14, xmm0

	mov rbx, 48             ; ebx == y
.lightBlurXSSEOuterLoop:
	mov rax, 32

        mov rcx, 48
        sub rcx, rbx
        imul rcx, rcx, 48
        add rcx, rdi

        lea rdx, [rcx + (48 * 48 * 47)]

        xorps xmm0, xmm0
	xorps xmm1, xmm1
	xorps xmm2, xmm2
	xorps xmm6, xmm6
	xorps xmm7, xmm7
	xorps xmm8, xmm8

.lightBlurXSSEInnerLoop:
	movdqa xmm3,  [rcx]
	movdqa xmm4,  [rcx + 16]
	movdqa xmm5,  [rcx + 32]

	movdqa xmm9,  [rdx]
	movdqa xmm10, [rdx + 16]
	movdqa xmm11, [rdx + 32]

	pmaxsb xmm0, xmm3
	pmaxsb xmm1, xmm4
	pmaxsb xmm2, xmm5

	pmaxsb xmm6, xmm9
	pmaxsb xmm7, xmm10
	pmaxsb xmm8, xmm11

	movdqa [rcx], xmm0
	movdqa [rcx + 16], xmm1
	movdqa [rcx + 32], xmm2

	movdqa [rdx], xmm6
	movdqa [rdx + 16], xmm7
	movdqa [rdx + 32], xmm8

	psubb xmm0, xmm14
	psubb xmm1, xmm14
	psubb xmm2, xmm14

	psubb xmm6, xmm14
	psubb xmm7, xmm14
	psubb xmm8, xmm14

	pmaxsb xmm0, xmm15
	pmaxsb xmm1, xmm15
	pmaxsb xmm2, xmm15

	pmaxsb xmm6, xmm15
	pmaxsb xmm7, xmm15
	pmaxsb xmm8, xmm15

        add rcx, 48 * 48
	sub rdx, 48 * 48
	dec rax

	jnz .lightBlurXSSEInnerLoop
	dec rbx
	jnz .lightBlurXSSEOuterLoop
	pop rdi
	pop rdx
	pop rcx
	pop rbx
	fxrstor [savedFloats]
	ret
