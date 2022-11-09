section .data
put_int_buffer dq 0
;--------------------------------------- put_int --------------------------------------------
;function:Implement integer printing
;---------------------------------------------------------------------------------------------
global put_int
put_int:
        pushad
        mov ebp, esp
        mov eax, [ebp + 4*9]
        mov edx, eax
        mov edi, 7
        mov ecx, 8
        mov ebx, put_int_buffer

.16based_4bits:
        and edx, 0x0000000F

        cmp edx, 9
        jg .is_A2F
        add edx, '0'
        jmp .store

.is_A2F:
        sub edx, 10

        add edx, 'A'

.store:
        mov [ebx + edi] , dl
        dec edi
        shr eax, 4
        mov edx, eax
        loop .16based_4bits

.ready_to_print:
        inc edi ;before edi is 0xffffffff,inc makes it 0

.skip_prefix_0:
        cmp edi, 8      ;if edi = 8,that means the number is 0
        je .full0

.go_on_skip:
        mov cl, [put_int_buffer + edi]
        inc edi
        cmp cl, '0'
        je .skip_prefix_0
        dec edi
        jmp .put_each_num

.full0:
        mov cl, '0'

.put_each_num:
        push ecx        ;Pass the parameters for put_char
        call put_char
        add esp, 4
        inc edi
        mov cl, [put_int_buffer + edi]
        cmp edi, 8
        jl .put_each_num
        popad
        ret

