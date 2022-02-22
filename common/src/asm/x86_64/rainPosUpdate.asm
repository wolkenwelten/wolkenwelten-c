bits 64
default rel

section .bss
extern rainCount, rainVel, rainDrops, glRainDrops, savedFloats

section .text
global rainPosUpdateSSE, rainPosUpdateAVX

rainPosUpdateSSE:
	fxsave [savedFloats]
	mov ecx, [rainCount]
	shr ecx, 1
	inc ecx

	lea rdx, [rainVel]
	movaps xmm4, [rdx]
	lea rax, [rainDrops]
	lea rdx, [glRainDrops]

.rainPosUpdateLoopSSE:
	movaps xmm0, [rdx]
	movaps xmm1, [rax]
	movaps xmm2, [rdx + 16]
	movaps xmm3, [rax + 16]

	addps  xmm0, xmm1
	addps  xmm1, xmm4
	addps  xmm2, xmm3
	addps  xmm3, xmm4

	movaps [rdx], xmm0
	movaps [rax], xmm1
	movaps [rdx + 16], xmm2
	movaps [rax + 16], xmm3

	add rdx, 32
	add rax, 32
	dec ecx
	jnz .rainPosUpdateLoopSSE
.rainPosUpdateEndSSE:
	fxrstor [savedFloats]
	ret

rainPosUpdateAVX:
	fxsave [savedFloats]
	mov ecx, [rainCount]
	shr ecx, 2
	inc ecx

	lea rdx, [rainVel]
	vmovaps ymm8, [rdx]
	lea rax, [rainDrops]
	lea rdx, [glRainDrops]

.rainPosUpdateLoopAVX:
	vmovaps ymm0, [rdx]
	vmovaps ymm1, [rax]
	vmovaps ymm2, [rdx + 32]
	vmovaps ymm3, [rax + 32]

	vaddps ymm0, ymm0, ymm1
	vaddps ymm1, ymm1, ymm8
	vaddps ymm2, ymm2, ymm3
	vaddps ymm3, ymm3, ymm8

	vmovaps [rdx], ymm0
	vmovaps [rax], ymm1
	vmovaps [rdx + 32], ymm2
	vmovaps [rax + 32], ymm3

	add rdx, 64
	add rax, 64
	dec ecx
	jnz .rainPosUpdateLoopAVX
	fxrstor [savedFloats]
	ret
