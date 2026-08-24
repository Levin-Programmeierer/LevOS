.section .text
.global user_program

user_program:
	hlt
	jmp user_program