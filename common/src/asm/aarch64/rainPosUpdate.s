.text
.global      rainPosUpdateNEON
.type        rainPosUpdateNEON, @function

rainPosUpdateNEON:
  ldr x10,=rainCount
  ldr w9,[x10]
  lsr x9,x9,#1
  add w9,w9,#1

  ldr x10,=rainVel
  ld1 {v4.4s},[x10]

  ldr x10,=glRainDrops
  ldr x11,=rainDrops
.rainPosUpdateLoopNEON:
  ld1 {v0.4s},[x10]
  ld1 {v1.4s},[x11]
  fadd v0.4s,v0.4s,v1.4s
  st1 {v0.4s},[x10],#16
  fadd v1.4s,v1.4s,v4.4s
  st1 {v1.4s},[x11],#16

  ld1 {v2.4s},[x10]
  ld1 {v3.4s},[x11]
  fadd v2.4s,v2.4s,v3.4s
  st1 {v2.4s},[x10],#16
  fadd v3.4s,v3.4s,v4.4s
  st1 {v3.4s},[x11],#16

  subs w9,w9,#1
  bne .rainPosUpdateLoopNEON
  ret

particlePosUpdateNEON:
  ldr x10,=particleCount
  ldr w9,[x10]
  lsr x9,x9,#2
  add w9,w9,#1
  ldr x10,=glParticles
  ldr x11,=particles
.particlePosUpdateLoopNEON:

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
  bne .particlePosUpdateLoopNEON
  ret


sparticlePosUpdateNEON:
  ldr x10,=sparticleCount
  ldr w9,[x10]
  lsr x9,x9,#2
  add w9,w9,#1

  ldr x10,=sparticleVV
  ld1 { v8.4s},[x10],#16
  ld1 { v9.4s},[x10],#16
  ld1 {v10.4s},[x10],#16
  ld1 {v11.4s},[x10],#16

  ldr x10,=glSparticles
  ldr x11,=sparticles
.sparticlePosUpdateLoopNEON:

  ld1 {v0.4s},[x10]
  ld1 {v1.4s},[x11]
  fadd v0.4s,v0.4s,v1.4s
  fadd v1.4s,v8.4s,v1.4s
  st1 {v0.4s},[x10],#16
  st1 {v1.4s},[x11],#16

  ld1 {v2.4s},[x10]
  ld1 {v3.4s},[x11]
  fadd v2.4s,v2.4s,v3.4s
  fadd v3.4s,v9.4s,v3.4s
  st1 {v2.4s},[x10],#16
  st1 {v3.4s},[x11],#16

  ld1 {v4.4s},[x10]
  ld1 {v5.4s},[x11]
  fadd v4.4s,v4.4s,v5.4s
  fadd v5.4s,v10.4s,v5.4s
  st1 {v4.4s},[x10],#16
  st1 {v5.4s},[x11],#16

  ld1 {v6.4s},[x10]
  ld1 {v7.4s},[x11]
  fadd v6.4s,v6.4s,v7.4s
  fadd v7.4s,v11.4s,v7.4s
  st1 {v6.4s},[x10],#16
  st1 {v7.4s},[x11],#16

  subs w9,w9,#1
  bne .sparticlePosUpdateLoopNEON
  ret
