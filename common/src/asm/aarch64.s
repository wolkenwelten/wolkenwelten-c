.text
.global particlePosUpdate
.type particlePosUpdatem, @function

particlePosUpdate:
  ldr x10,=particleCount
  ldr w9,[x10]
  lsr x9,x9,#2
  add w9,w9,#1
  ldr x10,=glParticles
  ldr x11,=particles
.particlePosUpdateLoop:

  ld1 {v0.4s},[x10]
  ld1 {v1.4s},[x11],#16
  fadd v0.4s,v0.4s,v1.4s
  st1 {v0.4s},[x10],#16

  ld1 {v2.4s},[x10]
  ld1 {v3.4s},[x11],#16
  fadd v2.4s,v2.4s,v3.4s
  st1 {v2.4s},[x10],#16

  ld1 {v4.4s},[x10]
  ld1 {v5.4s},[x11],#16
  fadd v4.4s,v4.4s,v5.4s
  st1 {v4.4s},[x10],#16

  ld1 {v6.4s},[x10]
  ld1 {v7.4s},[x11],#16
  fadd v6.4s,v6.4s,v7.4s
  st1 {v6.4s},[x10],#16

  subs w9,w9,#1
  bne .particlePosUpdateLoop
  ret
