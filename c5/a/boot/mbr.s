%include "boot.inc"
SECTION MBR vstart=0x7c00
	mov ax, cs
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov fs, ax
	mov sp, 0x7c00
	mov ax, 0xb800
	mov gs, ax


	mov ax, 0600h
	mov bx, 0700h
	mov cx, 0
 	mov dx, 184fh
	int 10h

	mov byte [gs:0x00], '1'
	mov byte [gs:0x01], 0xA4
        mov byte [gs:0x02], ' '
        mov byte [gs:0x03], 0xA4
        mov byte [gs:0x04], 'M'
        mov byte [gs:0x05], 0xA4
        mov byte [gs:0x06], 'B'
        mov byte [gs:0x07], 0xA4
        mov byte [gs:0x08], 'R'
        mov byte [gs:0x09], 0xA4

	mov eax, LOADER_START_SECTOR
	mov bx, LOADER_BASE_ADDR
	mov cx, 4
	call rd_disk_m_16
	
	jmp LOADER_BASE_ADDR

rd_disk_m_16:
  mov esi, eax	;backup eax
  mov di, cx	;backup cx

  ; sector count
  mov dx, 0x01f2
  mov al, cl
  out dx, al

  mov eax, esi

  ; LBA low
  mov dx, 0x1f3
  out dx, al

  ; LBA mid
  shr eax, 8
  mov dx, 0x1f4
  out dx, al

  ; LBA high
  shr eax, 8
  mov dx, 0x1f5
  out dx, al

  ; device reg: LBA[24:28]
  shr eax, 8
  and al, 0x0f

  or al, 0xe0  ; 0x1110, LBA mode
  mov dx, 0x1f6
  out dx, al

  ; command reg: 0x2 read, start reading
  mov dx, 0x1f7
  mov al, 0x20
  out dx, al

.not_ready:
  nop
  in al, dx
  and al, 0x88  ; bit 7 (busy), bit 3 (data ready)
  cmp al, 0x08
  jnz .not_ready

  ; di = cx = sector count
  ; read 2 bytes time, so loop (sector count) * 512 / 2 times
  mov ax, di
  mov dx, 256
  mul dx
  mov cx, ax

  mov dx, 0x1f0

.go_on_read:
  in ax, dx
  mov [bx], ax
  add bx, 2
  loop .go_on_read
  ret


times 510-($-$$) db 0
db 0x55, 0xaa

