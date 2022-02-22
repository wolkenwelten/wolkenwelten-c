bits 64
default rel

section .bss
extern particleCount, particles, glParticles, savedFloats

section .text
global particlePosUpdateSSE, particlePosUpdateAVX

particlePosUpdateSSE:
	fxsave   [savedFloats]
	mov ecx, [particleCount]
	shr ecx, 2
	inc ecx
	lea rax, [particles]
	lea rdx, [glParticles]

.particleUpdateLoopSSE:
	movaps xmm0, [rdx]
	movaps xmm1, [rax]
	movaps xmm2, [rdx + 16]
	movaps xmm3, [rax + 16]
	movaps xmm4, [rdx + 32]
	movaps xmm5, [rax + 32]
	movaps xmm6, [rdx + 48]
	movaps xmm7, [rax + 48]

	addps  xmm0, xmm1
	addps  xmm2, xmm3
	addps  xmm4, xmm5
	addps  xmm6, xmm7

	movaps [rdx], xmm0
	movaps [rdx + 16], xmm2
	movaps [rdx + 32], xmm4
	movaps [rdx + 48], xmm6

	add rdx, 64
	add rax, 64
	dec ecx
	jnz .particleUpdateLoopSSE
	fxrstor [savedFloats]
	ret

particlePosUpdateAVX:
	fxsave [savedFloats]
	mov ecx, [particleCount]
	shr ecx, 2
	inc ecx
	lea rax, [particles]
	lea rdx, [glParticles]

.particleUpdateLoopAVX:
	vmovaps ymm0, [rdx]
	vmovaps ymm1, [rax]
	vmovaps ymm2, [rdx + 32]
	vmovaps ymm3, [rax + 32]

	vaddps ymm4, ymm1, ymm0
	vaddps ymm3, ymm2, ymm5

	vmovaps [rdx], ymm4
	vmovaps [rdx + 32], ymm5

	add rdx, 64
	add rax, 64
	dec ecx
	jnz .particleUpdateLoopAVX
	fxrstor [savedFloats]
	ret
