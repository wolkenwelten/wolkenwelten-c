.data
.align 16
savedFloats: .zero 512

.text
.global       rainPosUpdateSSE
.global       rainPosUpdateAVX
.global   particlePosUpdateSSE
.global   particlePosUpdateAVX
.global  sparticlePosUpdateSSE
.global  sparticlePosUpdateAVX

particlePosUpdateSSE:
  fxsave savedFloats(%rip)
  xor %ecx,%ecx
  movl particleCount(%rip), %ecx
  shrl $2,%ecx
  jz .particleUpdateEndSSE
  leaq glParticles(%rip), %rdx
  leaq   particles(%rip), %rax

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
.particleUpdateEndSSE:
  fxrstor savedFloats(%rip)
  ret

particlePosUpdateAVX:
  fxsave savedFloats(%rip)
  xor %ecx,%ecx
  movl particleCount(%rip), %ecx
  shrl $2,%ecx
  jz .particleUpdateEndAVX
  leaq glParticles(%rip), %rdx
  leaq   particles(%rip), %rax

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
.particleUpdateEndAVX:
  fxrstor savedFloats(%rip)
  ret

sparticlePosUpdateSSE:
  fxsave savedFloats(%rip)
  xor %ecx,%ecx
  movl sparticleCount(%rip), %ecx
  shrl $2,%ecx
  jz .sparticleUpdateEndSSE

  leaq sparticleVV(%rip), %rdx
  movaps   (%rdx),%xmm4
  movaps 16(%rdx),%xmm5
  movaps 32(%rdx),%xmm6
  movaps 48(%rdx),%xmm7

  leaq glSparticles(%rip), %rdx
  leaq   sparticles(%rip), %rax

.sparticleUpdateLoopSSE:
  movaps   (%rdx),%xmm0
  movaps   (%rax),%xmm1
  addps  %xmm1,%xmm0
  addps  %xmm4,%xmm1
  movaps %xmm0,  (%rdx)
  movaps %xmm1,  (%rax)

  movaps 16(%rdx),%xmm2
  movaps 16(%rax),%xmm3
  addps  %xmm3,%xmm2
  addps  %xmm5,%xmm3
  movaps %xmm2,16(%rdx)
  movaps %xmm3,16(%rax)

  movaps 32(%rdx),%xmm0
  movaps 32(%rax),%xmm1
  addps  %xmm1,%xmm0
  addps  %xmm6,%xmm1
  movaps %xmm0,32(%rdx)
  movaps %xmm1,32(%rax)

  movaps 48(%rdx),%xmm2
  movaps 48(%rax),%xmm3
  addps  %xmm3,%xmm2
  addps  %xmm7,%xmm3
  movaps %xmm2,48(%rdx)
  movaps %xmm3,48(%rax)

  add $64,%rdx
  add $64,%rax
  dec %ecx
  jnz .sparticleUpdateLoopSSE
.sparticleUpdateEndSSE:
  fxrstor savedFloats(%rip)
  ret

sparticlePosUpdateAVX:
  fxsave savedFloats(%rip)
  xor %ecx,%ecx
  movl sparticleCount(%rip), %ecx
  shrl $2,%ecx
  jz .sparticleUpdateEndAVX

  leaq sparticleVV(%rip), %rdx
  vmovaps   (%rdx),%ymm4
  vmovaps 32(%rdx),%ymm5

  leaq glSparticles(%rip), %rdx
  leaq   sparticles(%rip), %rax

.sparticleUpdateLoopAVX:
  vmovaps   (%rdx),%ymm0
  vmovaps   (%rax),%ymm1
  vmovaps 32(%rdx),%ymm2
  vmovaps 32(%rax),%ymm3

  vaddps  %ymm1,%ymm0,%ymm6
  vaddps  %ymm4,%ymm1,%ymm7
  vaddps  %ymm3,%ymm2,%ymm8
  vaddps  %ymm5,%ymm3,%ymm9

  vmovaps %ymm6,  (%rdx)
  vmovaps %ymm7,  (%rax)
  vmovaps %ymm8,32(%rdx)
  vmovaps %ymm9,32(%rax)

  add $64,%rdx
  add $64,%rax
  dec %ecx
  jnz .sparticleUpdateLoopAVX
.sparticleUpdateEndAVX:
  fxrstor savedFloats(%rip)
  ret

rainPosUpdateSSE:
  fxsave savedFloats(%rip)
  movl rainCount(%rip), %ecx
  shrl $1,%ecx
  inc %ecx

  leaq rainVel(%rip), %rdx
  movaps (%rdx),%xmm4

  leaq glRainDrops(%rip), %rdx
  leaq   rainDrops(%rip), %rax

.rainPosUpdateLoopSSE:
  movaps   (%rdx),%xmm0
  movaps   (%rax),%xmm1
  addps  %xmm1,%xmm0
  addps  %xmm4,%xmm1
  movaps %xmm0,  (%rdx)
  movaps %xmm1,  (%rax)

  movaps 16(%rdx),%xmm2
  movaps 16(%rax),%xmm3
  addps  %xmm3,%xmm2
  addps  %xmm4,%xmm3
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
  movl rainCount(%rip), %ecx
  shrl $2,%ecx
  inc %ecx

  leaq rainVel(%rip), %rdx
  vmovaps (%rdx),%ymm8

  leaq glRainDrops(%rip), %rdx
  leaq   rainDrops(%rip), %rax

.rainPosUpdateLoopAVX:
  vmovaps   (%rdx),%ymm0
  vmovaps   (%rax),%ymm1
  vaddps  %ymm1,%ymm0,%ymm0
  vaddps  %ymm8,%ymm1,%ymm1
  vmovaps %ymm0,  (%rdx)
  vmovaps %ymm1,  (%rax)

  vmovaps 32(%rdx),%ymm2
  vmovaps 32(%rax),%ymm3
  vaddps  %ymm3,%ymm2,%ymm2
  vaddps  %ymm8,%ymm3,%ymm3
  vmovaps %ymm2,32(%rdx)
  vmovaps %ymm3,32(%rax)

  add $64,%rdx
  add $64,%rax
  dec %ecx
  jnz .rainPosUpdateLoopAVX
.rainPosUpdateEndAVX:
  fxrstor savedFloats(%rip)
  ret
