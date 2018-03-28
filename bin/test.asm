section .data
var dd 0

section .text
global _start:function (_start.end - _start)
_start:
	jmp _start
.end:

