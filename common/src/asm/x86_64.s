.data
.align 16
savedFloats: .zero 512

.text
.global      rainPosUpdate
.global  particlePosUpdate
.global sparticlePosUpdate

particlePosUpdate:
  fxsave savedFloats(%rip)
  xor %ecx,%ecx
  movl particleCount(%rip), %ecx
  shrl $2,%ecx
  jz .particleUpdateEnd
  leaq glParticles(%rip), %rdx
  leaq   particles(%rip), %rax

.particleUpdateLoop:
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
  jnz .particleUpdateLoop
.particleUpdateEnd:
  ret



sparticlePosUpdate:
  xor %ecx,%ecx
  movl sparticleCount(%rip), %ecx
  shrl $2,%ecx
  jz .sparticleUpdateEnd

  leaq sparticleVV(%rip), %rdx
  movaps   (%rdx),%xmm4
  movaps 16(%rdx),%xmm5
  movaps 32(%rdx),%xmm6
  movaps 48(%rdx),%xmm7

  leaq glSparticles(%rip), %rdx
  leaq   sparticles(%rip), %rax

.sparticleUpdateLoop:
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
  jnz .sparticleUpdateLoop
.sparticleUpdateEnd:
  fxrstor savedFloats(%rip)
  ret

rainPosUpdate:
  fxsave savedFloats(%rip)
  movl rainCount(%rip), %ecx
  shrl $1,%ecx
  inc %ecx

  leaq rainVel(%rip), %rdx
  movaps (%rdx),%xmm4

  leaq glRainDrops(%rip), %rdx
  leaq   rainDrops(%rip), %rax

.rainPosUpdateLoop:
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
  jnz .rainPosUpdateLoop
.rainPosUpdateEnd:
  fxrstor savedFloats(%rip)
  ret
