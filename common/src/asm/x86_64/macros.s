.ifdef USE_GOT

.macro movVarl sym reg
  movq \sym@GOTPCREL(%rip), %rdx
  movl (%rdx), \reg
.endm

.macro movVarq sym reg
  movq \sym@GOTPCREL(%rip), %rdx
  leaq (%rdx), \reg
.endm

.else

.macro movVarl sym reg
  movl \sym(%rip), \reg
.endm

.macro movVarq sym reg
  leaq \sym(%rip), \reg
.endm

.endif
