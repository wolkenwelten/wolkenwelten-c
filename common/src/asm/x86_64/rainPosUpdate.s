.include "macros.s"

.data
.align 16
savedFloats: .zero 512

.text
.global       rainPosUpdateSSE
.global       rainPosUpdateAVX


rainPosUpdateSSE:
  fxsave savedFloats(%rip)
  movVarl rainCount, %ecx
  shrl $1,%ecx
  inc %ecx

  movVarq rainVel, %rdx
  movaps (%rdx),%xmm4


  movVarq rainDrops, %rax
  movVarq glRainDrops, %rdx

.rainPosUpdateLoopSSE:
  movaps   (%rdx),%xmm0
  movaps   (%rax),%xmm1
  movaps 16(%rdx),%xmm2
  movaps 16(%rax),%xmm3

  addps  %xmm1,%xmm0
  addps  %xmm4,%xmm1
  addps  %xmm3,%xmm2
  addps  %xmm4,%xmm3

  movaps %xmm0,  (%rdx)
  movaps %xmm1,  (%rax)
  movaps %xmm2,16(%rdx)
  movaps %xmm3,16(%rax)

  add $32,%rdx
  add $32,%rax
  dec %ecx
  jnz .rainPosUpdateLoopSSE
.rainPosUpdateEndSSE:
  fxrstor savedFloats(%rip)
  ret

rainPosUpdateAVX:
  fxsave savedFloats(%rip)
  movVarl rainCount, %ecx
  shrl $2,%ecx
  inc %ecx

  movVarq rainVel, %rdx
  vmovaps (%rdx),%ymm8

  movVarq   rainDrops, %rax
  movVarq glRainDrops, %rdx

.rainPosUpdateLoopAVX:
  vmovaps   (%rdx),%ymm0
  vmovaps   (%rax),%ymm1
  vmovaps 32(%rdx),%ymm2
  vmovaps 32(%rax),%ymm3

  vaddps  %ymm1,%ymm0,%ymm0
  vaddps  %ymm8,%ymm1,%ymm1
  vaddps  %ymm3,%ymm2,%ymm2
  vaddps  %ymm8,%ymm3,%ymm3

  vmovaps %ymm0,  (%rdx)
  vmovaps %ymm1,  (%rax)
  vmovaps %ymm2,32(%rdx)
  vmovaps %ymm3,32(%rax)

  add $64,%rdx
  add $64,%rax
  dec %ecx
  jnz .rainPosUpdateLoopAVX
  fxrstor savedFloats(%rip)
  ret
