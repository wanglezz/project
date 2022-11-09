TI_GDT  equ     0
RPL0    equ     0
SELECTOR_VIDEO  equ     (0x0003<<3) + TI_GDT + RPL0

[bits 32]
section .text
;--------------------------------------- put_char --------------------------------------------
;function:Writes a character from the stack to the cursor position
;---------------------------------------------------------------------------------------------
global put_char
put_char:
        pushad
        mov ax, SELECTOR_VIDEO
        mov gs, ax

;-----------get_cursor_pos-----------
        mov dx, 0x03d4
        mov al, 0x0e
        out dx, al
        mov dx, 0x03d5
        in al, dx
        mov ah, al

        mov dx, 0x03d4
        mov al, 0x0f
        out dx, al
        mov dx, 0x03d5
        in al, dx

        mov bx, ax      ;bx is cursor position
        mov ecx, [esp + 36]

        cmp cl, 0xd     ;CR is 0xd
        jz .is_carriage_return
        cmp cl, 0xa     ;LF is 0x0a
        jz .is_line_feed

        cmp cl, 0x8     ;BS(Backspace) is 8
        jz .is_backspace
        jmp .put_other

.is_backspace:
        dec bx
        shl bx, 1

        mov byte [gs:bx], 0x20
        inc bx
        mov byte [gs:bx], 0x7
        shr bx, 1
        jmp .set_cursor

.put_other:
        shl bx, 1

	mov [gs:bx], cl
	inc bx
	mov byte [gs:bx], 0x07
	shr bx, 1
	inc bx
	cmp bx, 2000
	jl .set_cursor

.is_line_feed:
.is_carriage_return:
	xor dx, dx
	mov ax, bx
	mov si, 80

	div si	;ax save quotient,dx save remainder
	sub bx, dx	;\r

.is_carriage_return_end:
	add bx, 80	;\r + \n
	cmp bx, 2000
.is_line_feed_end
	jl .set_cursor
	
;--------------line1~24 -> line0~23--------------
.roll_screen:
	cld
	mov ecx,960	;(2000-80)*2/4 = 960

	mov esi, 0xc00b80a0	;first line 
	mov edi, 0xc00b8000	;zero line
	rep movsb

	mov ebx, 3840	;In preparation for .cls, the cursor points to the last line
	mov ecx, 80
.cls:
	mov word [gs:bx], 0x0720
	add ebx, 2
	loop .cls
	mov bx, 1920	;The cursor points to the beginning of the last line

.set_cursor:
;----------the higher eight-----------
	mov dx, 0x03d4
	mov al, 0x0e
	out dx, al
	mov dx, 0x03d5
	mov al, bh
	out dx, al

;----------the lower eight---------
	mov dx, 0x03d4
        mov al, 0x0f
        out dx, al
        mov dx, 0x03d5
        mov al, bl
        out dx, al

.put_char_done:
	popad
	ret
