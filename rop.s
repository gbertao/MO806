	.code16
.macro KCALL par0
	mov $e_\par0, %esi
	sub $s_\par0, %esi
	mov $0x50, %dx
	mov $0x41, %al
	out %al, %dx
s_\par0:
	call \par0
e_\par0:
.endm

.macro KRET
	mov (%esp), %esi
	mov $0x51, %dx
	mov $0x41, %al
	out %al, %dx
	ret
.endm

	.global mal
	.text
mal:
	KCALL func
        hlt

func:
	mov $0xa, %ebx
	lea rop_tgt, %eax
	mov %eax, (%esp)
	KRET

rop_tgt:
	mov $0x0, %ebx
	hlt
