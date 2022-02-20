.text
.global      rainPosUpdateNEON

rainPosUpdateNEON:
  ldr r3,=rainCount
  ldr r1,[r3]
  lsr r1,r1,#1
  add r1,r1,#1

  ldr r3,=rainVel
  vld1.32 {q4},[r3]

  mov r12,#16
  ldr r2,=glRainDrops
  ldr r3,=rainDrops
.rainPosUpdateLoopNEON:
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
  bne .rainPosUpdateLoopNEON
  mov pc, lr
