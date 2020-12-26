.text
.global particlePosUpdate

particlePosUpdate:
  xor %rcx,%rcx
  movl particleCount(%rip), %ecx
  cmp $0,%rcx
  shrq $3,%rcx
  jz .particleUpdateEnd
  leaq glParticles(%rip), %rdx
  leaq particles(%rip), %rax

.particleUpdateLoop:
  vmovaps (%rdx),%ymm0
  vmovaps (%rax),%ymm1
  vaddps  %ymm1,%ymm0,%ymm2
  vmovaps %ymm0,(%rdx)

  vmovaps 32(%rdx),%ymm0
  vmovaps 32(%rax),%ymm1
  vaddps  %ymm1,%ymm0,%ymm2
  vmovaps %ymm2,32(%rdx)

  vmovaps 64(%rdx),%ymm0
  vmovaps 64(%rax),%ymm1
  vaddps  %ymm1,%ymm0,%ymm2
  vmovaps %ymm2,64(%rdx)

  vmovaps 96(%rdx),%ymm0
  vmovaps 96(%rax),%ymm1
  vaddps  %ymm1,%ymm0,%ymm2
  vmovaps %ymm2,96(%rdx)

  add $128,%rdx
  add $128,%rax
  dec %rcx
  jnz .particleUpdateLoop
.particleUpdateEnd:
  ret
