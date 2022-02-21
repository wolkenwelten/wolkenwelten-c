.include "macros.s"

.data
.align 16
savedFloats: .zero 512

.text
.global  lightBlurYSSE

lightBlurYSSE:
  fxsave savedFloats(%rip)
  pushq %rbx
  pushq %rcx
  pushq %rdx

  xor %eax, %eax
  xorps %xmm15, %xmm15
  mov $02, %al
  movd %eax, %xmm0
  vpbroadcastb %xmm0, %xmm14

  mov $49,%ebx

.lightBlurYSSEOuterLoop:
  mov $(32),%eax

  lea (48 * 47)(%rcx), %rdx
  movdqa   (%rcx),%xmm0
  movdqa 16(%rcx),%xmm1
  movdqa 32(%rcx),%xmm2
  movdqa   (%rdx),%xmm6
  movdqa 16(%rdx),%xmm7
  movdqa 32(%rdx),%xmm8

.lightBlurYSSEInnerLoop:
  addq $48, %rcx
  subq $48, %rdx
  movdqa  0(%rcx),%xmm3
  movdqa 16(%rcx),%xmm4
  movdqa 32(%rcx),%xmm5

  movdqa  0(%rdx),%xmm9
  movdqa 16(%rdx),%xmm10
  movdqa 32(%rdx),%xmm11

  pmaxsb %xmm3, %xmm0
  pmaxsb %xmm4, %xmm1
  pmaxsb %xmm5, %xmm2

  pmaxsb %xmm9, %xmm6
  pmaxsb %xmm10, %xmm7
  pmaxsb %xmm11, %xmm8

  movdqa %xmm0,  0(%rcx)
  movdqa %xmm1, 16(%rcx)
  movdqa %xmm2, 32(%rcx)

  movdqa %xmm6,  0(%rdx)
  movdqa %xmm7, 16(%rdx)
  movdqa %xmm8, 32(%rdx)

  psubb %xmm14, %xmm0
  psubb %xmm14, %xmm1
  psubb %xmm14, %xmm2

  psubb %xmm14, %xmm6
  psubb %xmm14, %xmm7
  psubb %xmm14, %xmm8

  pmaxsb %xmm15, %xmm0
  pmaxsb %xmm15, %xmm1
  pmaxsb %xmm15, %xmm2

  pmaxsb %xmm15, %xmm6
  pmaxsb %xmm15, %xmm7
  pmaxsb %xmm15, %xmm8

  dec %eax
  jnz .lightBlurYSSEInnerLoop
  addq $(48 * 16), %rcx
  subq $(48 * 16), %rdx
  dec %ebx
  jnz .lightBlurYSSEOuterLoop
  popq %rdx
  popq %rcx
  popq %rbx
  fxrstor savedFloats(%rip)
  ret
