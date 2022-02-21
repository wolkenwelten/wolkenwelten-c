bits 64
default rel

section .bss
extern savedFloats

section .text
global sparticlePosUpdateSSE, sparticlePosUpdateAVX
extern sparticleCount, sparticleVV, sparticles, glSparticles


sparticlePosUpdateSSE:
        fxsave [savedFloats]
        mov ecx, [sparticleCount]
        shr ecx, 2
        inc ecx

        lea rdx, [sparticleVV]
        movaps xmm4, [rdx]
        movaps xmm5, [rdx + 16]
        movaps xmm6, [rdx + 32]
        movaps xmm7, [rdx + 48]

        lea rax, [sparticles]
        lea rdx, [glSparticles]

.sparticleUpdateLoopSSE:
        movaps xmm0, [rdx]
        movaps xmm1, [rax]
        movaps xmm2, [rdx + 16]
        movaps xmm3, [rax + 16]

        addps xmm0, xmm1
        addps xmm1, xmm4
        addps xmm2, xmm3
        addps xmm3, xmm5

        movaps [rdx], xmm0
        movaps [rax], xmm1
        movaps [rdx + 16], xmm2
        movaps [rax + 16], xmm3

        movaps xmm0, [rdx + 32]
        movaps xmm1, [rax + 32]
        movaps xmm2, [rdx + 48]
        movaps xmm3, [rax + 48]

        addps xmm0, xmm1
        addps xmm1, xmm6
        addps xmm2, xmm3
        addps xmm3, xmm7

        movaps [rdx + 32], xmm0
        movaps [rax + 32], xmm1
        movaps [rdx + 48], xmm2
        movaps [rax + 48], xmm3

        add rdx, 64
        add rax, 64
        dec ecx
        jnz .sparticleUpdateLoopSSE
        fxrstor [savedFloats]
        ret

sparticlePosUpdateAVX:
        fxsave [savedFloats]
        mov ecx, [sparticleCount]
        shr ecx, 2
        inc ecx

        lea rdx, [sparticleVV]
        vmovaps ymm4, [rdx]
        vmovaps ymm5, [rdx + 32]

        lea rax, [sparticles]
        lea rdx, [glSparticles]

.sparticleUpdateLoopAVX:
        vmovaps ymm0, [rdx]
        vmovaps ymm1, [rax]
        vmovaps ymm2, [rdx + 32]
        vmovaps ymm3, [rax + 32]

        vaddps  ymm6, ymm1, ymm0
        vaddps  ymm7, ymm1, ymm4
        vaddps  ymm8, ymm2, ymm3
        vaddps  ymm9, ymm3, ymm5

        vmovaps [rdx], ymm6
        vmovaps [rax], ymm7
        vmovaps [rdx + 32], ymm8
        vmovaps [rax + 32], ymm9

        add rdx, 64
        add rax, 64
        dec ecx
        jnz .sparticleUpdateLoopAVX
        fxrstor [savedFloats]
        ret
