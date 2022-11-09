[bits 32]
section .text
;--------------------------------------- put_str --------------------------------------------
;function:put_str prints a string ending in 0 characters by put_char
;---------------------------------------------------------------------------------------------


global put_str
put_str:
        push ebx
        push ecx        ;put_str only need ebx and ecx
        xor ecx, ecx
        mov ebx, [esp + 12]     ;ebx save the address of string to be printed

.goon:
        mov cl, [ebx]
        cmp cl, 0
        jz .str_over

        push ecx        ;Pass the parameters for put_char
        call put_char
        add esp, 4
        inc ebx
        jmp .goon

.str_over:
        pop ecx
        pop ebx
        ret

