.include "./common/src/asm/x86_64/macros.s"

.data
.align 16
savedFloats: .zero 512

.text
.global   particlePosUpdateSSE
.global   particlePosUpdateAVX


particlePosUpdateSSE:
  fxsave savedFloats(%rip)
  movVarl particleCount, %ecx
  shrl $2,%ecx
  inc %ecx

  movVarq   particles, %rax
  movVarq glParticles, %rdx

.particleUpdateLoopSSE:
  movaps   (%rdx),%xmm0
  movaps   (%rax),%xmm1
  movaps 16(%rdx),%xmm2
  movaps 16(%rax),%xmm3
  movaps 32(%rdx),%xmm4
  movaps 32(%rax),%xmm5
  movaps 48(%rdx),%xmm6
  movaps 48(%rax),%xmm7

  addps  %xmm1,%xmm0
  addps  %xmm3,%xmm2
  addps  %xmm5,%xmm4
  addps  %xmm7,%xmm6

  movaps %xmm0,  (%rdx)
  movaps %xmm2,16(%rdx)
  movaps %xmm4,32(%rdx)
  movaps %xmm6,48(%rdx)

  add $64,%rdx
  add $64,%rax
  dec %ecx
  jnz .particleUpdateLoopSSE
  fxrstor savedFloats(%rip)
  ret

particlePosUpdateAVX:
  fxsave savedFloats(%rip)
  movVarl particleCount, %ecx
  shrl $2,%ecx
  inc %ecx

  movVarq   particles, %rax
  movVarq glParticles, %rdx

.particleUpdateLoopAVX:
  vmovaps   (%rdx),%ymm0
  vmovaps   (%rax),%ymm1
  vmovaps 32(%rdx),%ymm2
  vmovaps 32(%rax),%ymm3

  vaddps  %ymm1,%ymm0,%ymm4
  vaddps  %ymm3,%ymm2,%ymm5

  vmovaps %ymm4,  (%rdx)
  vmovaps %ymm5,32(%rdx)

  add $64,%rdx
  add $64,%rax
  dec %ecx
  jnz .particleUpdateLoopAVX
  fxrstor savedFloats(%rip)
  ret
