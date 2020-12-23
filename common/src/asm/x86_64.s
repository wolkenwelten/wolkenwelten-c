.global particlePosUpdate

.text
particlePosUpdate:
  xor %rcx,%rcx
  movl particleCount(%rip), %ecx
  jz .particleUpdateEnd
  leaq glParticles(%rip), %rdx
  leaq particles(%rip), %rbx

.particleUpdateLoop:
  movaps (%rdx),%xmm0
  movaps (%rbx),%xmm1
  addps  %xmm1,%xmm0
  movaps %xmm2,(%rdx)

  add $16,%rdx
  add $16,%rbx
  dec %rcx
  jnz .particleUpdateLoop
.particleUpdateEnd:
  ret
