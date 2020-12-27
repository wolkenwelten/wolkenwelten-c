.data
.align 16
savedFloats: .zero 512

.text
.global particlePosUpdate

particlePosUpdate:
  fxsave savedFloats(%rip)
  xor %rcx,%rcx
  movl particleCount(%rip), %ecx
  cmp $0,%rcx
  shrq $2,%rcx
  jz .particleUpdateEnd
  leaq glParticles(%rip), %rdx
  leaq particles(%rip), %rax

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
  dec %rcx
  jnz .particleUpdateLoop
.particleUpdateEnd:
  fxrstor savedFloats(%rip)
  ret
