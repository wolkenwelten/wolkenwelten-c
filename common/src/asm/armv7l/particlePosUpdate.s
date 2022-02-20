.text
.global  particlePosUpdateNEON

particlePosUpdateNEON:
  ldr r3,=particleCount
  ldr r1,[r3]
  lsr r1,r1,#2
  add r1,r1,#1

  ldr r2,=glParticles
  ldr r3,=particles
  mov r12,#16
.particlePosUpdateLoopNEON:

  vld1.32 {q0},[r2]
  vld1.32 {q1},[r3],r12
  vadd.f32 q0,q0,q1
  vst1.32 {q0},[r2],r12

  vld1.32 {q2},[r2]
  vld1.32 {q3},[r3],r12
  vadd.f32 q2,q2,q3
  vst1.32 {q2},[r2],r12

  vld1.32 {q4},[r2]
  vld1.32 {q5},[r3],r12
  vadd.f32 q4,q4,q5
  vst1.32 {q4},[r2],r12

  vld1.32 {q6},[r2]
  vld1.32 {q7},[r3],r12
  vadd.f32 q6,q6,q7
  vst1.32 {q6},[r2],r12

  subs r1,r1,#1
  bne .particlePosUpdateLoopNEON
  mov pc, lr
