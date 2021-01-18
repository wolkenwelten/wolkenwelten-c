.text
.global sparticlePosUpdate
.global  particlePosUpdate
.global      rainPosUpdate

rainPosUpdate:
  ldr r3,=rainCount
  ldr r1,[r3]
  lsr r1,r1,#1
  add r1,r1,#1

  ldr r3,=rainVel
  vld1.32 {q4},[r3]

  mov r12,#16
  ldr r2,=glRainDrops
  ldr r3,=rainDrops
.rainPosUpdateLoop:
  vld1.32 {q0},[r2]
  vld1.32 {q1},[r3]
  vadd.f32 q0,q0,q1
  vst1.32 {q0},[r2],r12
  vadd.f32 q1,q1,q4
  vst1.32 {q1},[r3],r12

  vld1.32 {q2},[r2]
  vld1.32 {q3},[r3]
  vadd.f32 q2,q2,q3
  vst1.32 {q2},[r2],r12
  vadd.f32 q3,q3,q4
  vst1.32 {q3},[r3],r12

  subs r1,r1,#1
  bne .rainPosUpdateLoop
  mov pc, lr


particlePosUpdate:
  ldr r3,=particleCount
  ldr r1,[r3]
  lsr r1,r1,#2
  add r1,r1,#1

  ldr r2,=glParticles
  ldr r3,=particles
  mov r12,#16
.particlePosUpdateLoop:

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
  bne .particlePosUpdateLoop
  mov pc, lr


sparticlePosUpdate:
  ldr r3,=sparticleCount
  ldr r1,[r3]
  lsr r1,r1,#2
  add r1,r1,#1

  mov r12,#16
  ldr r3,=sparticleVV
  vld1.32 { q8},[r3],r12
  vld1.32 { q9},[r3],r12
  vld1.32 {q10},[r3],r12
  vld1.32 {q11},[r3],r12

  ldr r2,=glSparticles
  ldr r3,=sparticles
.sparticlePosUpdateLoop:

  vld1.32 {q0},[r2]
  vld1.32 {q1},[r3]
  vadd.f32 q0,q0,q1
  vadd.f32 q1,q8,q1
  vst1.32 {q0},[r2],r12
  vst1.32 {q1},[r3],r12

  vld1.32 {q2},[r2]
  vld1.32 {q3},[r3]
  vadd.f32 q2,q2,q3
  vadd.f32 q3,q9,q3
  vst1.32 {q2},[r2],r12
  vst1.32 {q3},[r3],r12

  vld1.32 {q4},[r2]
  vld1.32 {q5},[r3]
  vadd.f32 q4, q4,q5
  vadd.f32 q5,q10,q5
  vst1.32 {q4},[r2],r12
  vst1.32 {q5},[r3],r12

  vld1.32 {q6},[r2]
  vld1.32 {q7},[r3]
  vadd.f32 q6, q6,q7
  vadd.f32 q7,q11,q7
  vst1.32 {q6},[r2],r12
  vst1.32 {q7},[r3],r12

  subs r1,r1,#1
  bne .sparticlePosUpdateLoop
  mov pc, lr
