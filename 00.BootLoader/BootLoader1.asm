[ORG 0x00]
[BITS 16]

SECTION .text

jmp 0x07C0:START

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;	Code
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
START:
    mov ax, 0x07C0
    mov ds, ax
    mov ax, 0xB800
    mov es, ax

	; Create stack to 0x0000:0000~0x0000:FFFF that size is 64KB
	mov ax, 0x0000 ; convert stack seg start address(0x0000) to seg reg data
	mov ss, ax	   ; set SS seg reg
	mov sp, 0xFFFE ; set SP reg address to 0xFFFE
	mov bp, 0xFFFE ; set BP reg address to 0xFFFE

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;	Clear screen, set the color green
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov si, 0
    
.SCREENCLEARLOOP:
    mov byte [ es: si ], 0
    mov byte [ es: si + 1 ], 0x0A

    add si, 2
    cmp si, 80 * 25 * 2

    jl .SCREENCLEARLOOP
	mov si, 0
	mov di, 0

	mov ah, 0x04
	int 0x1A

	mov byte [TEMP1], ch
	mov byte [TEMP2], ch
	call CALDATE
	mov bx, ax
	;bx = 20

	mov al, 100
	mul bl
	mov bx, ax
	mov ax, 0
	
	mov byte [TEMP1], cl
	mov byte [TEMP2], cl
	call CALDATE
	add bx, ax
	
	mov byte [TEMP1], dh
	mov byte [TEMP2], dl
	
	jmp .for
.for:
	cmp word[TEMP3], bx
	ja .END
	mov dx, 0
	mov ax, word [TEMP3]
	mov cx, 400
	div cx
	cmp dx, 0
	je .YO
	mov dx, 0
	mov ax, word [TEMP3]
	mov cx, 4
	div cx
	cmp dx, 0
	je .GUM
	jmp .PY
.YO:
	add word[TEMP4], 2
	add word [TEMP3], 1
	;
	mov byte [TEMP], 1
	;change
	jmp .for
.PY:
	add word[TEMP4], 1
	add word [TEMP3], 1
	;
	mov byte [TEMP], 0
	;change
	jmp .for
.GUM:
	mov dx, 0
	mov ax, word[TEMP3]
	mov cx, 100
	div cx
	cmp dx, 0
	je .PY
	jmp .YO
.END:
	sub word [TEMP4], 1
	mov dh, byte [TEMP1]
	mov dl, byte [TEMP2]
	mov eax, 0
	mov byte [TEMP1], dl
	call CALDATE
	add word [TEMP4], ax

	mov byte [TEMP1], dh
	mov byte [TEMP2], dh
	call CALDATE
	;
	cmp byte[TEMP], 1
	je .SY
	jmp .K
.SY:
	cmp ax, 3
	jb .SUBA
.SUBA:
	sub word [TEMP4], 1
	jmp .K
.K:
;change
	sub ax, 1
	cmp ax, 2
	mov cx, 2
	mul cx
	
	mov bx, P
	add bx, ax
	mov eax, 0
	mov ax, [bx]
	add ax, word [TEMP4]

	mov dx, 0
	mov ecx, 7
	div ecx
	mov al, 4
	mul dx

	mov bx, T
	add bx, ax

	push bx
	push 1
	push 25
	call PRINTMESSAGE

RESETDISK:
	mov ax, 0
	mov dl, 0
	int 0x13
	mov si, 0x1000
	mov es, si
	mov bx, 0x0000


READDATA: ;	Read the disk
	mov ah, 0x02
	mov al, 0x1
	mov ch, byte [ TRACKNUMBER ]
	mov cl, byte [ SECTORNUMBER ]
	mov dh, byte [ HEADNUMBER ]
	mov dl, 0x00
	int 0x13

	add si, 0x0020
	mov es, si

READEND:
	jmp 0x1000:0x0000

CALDATE:
	shr byte [TEMP1], 4
	shl byte [TEMP2], 4
	shr byte [TEMP2], 4
	mov al, 10
	mul byte [TEMP1]
	add al, byte [TEMP2]
	ret

PRINTMESSAGE:
	push bp
	mov bp, sp

	push es
	push si
	push di
	push ax
	push cx
	push dx

	mov ax, 0xB800
	mov es, ax

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;	Calculate the address of video to X,Y
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov ax, word [ bp + 6 ]
	mov si, 160
	mul si
	mov di, ax

	mov ax, word [ bp + 4 ]
	mov si, 2
	mul si
	add di, ax

	;	Print the word
	mov si, word [ bp + 8 ]


.MESSAGELOOP:
	mov cl, byte [ si ]
	cmp cl, 0
	je .MESSAGEEND

	mov byte [ es: di ], cl
	add si, 1
	add di, 2

	jmp .MESSAGELOOP

.MESSAGEEND:
	pop dx
	pop cx
	pop ax
	pop di
	pop si
	pop es
	pop bp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Data
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;	Var about reding disk
SECTORNUMBER:	db 0x02
HEADNUMBER:		db 0x00
TRACKNUMBER:	db 0x00
TEMP db 0
TEMP1 db 0x00
TEMP2 db 0x00
TEMP3 dw 1900
TEMP4 dw 0
T: dd 'Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'
P: dw 0,31,59,90,120,151,181,212,243,273,304,334

times 510 - ( $ - $$ )    db    0x00

db 0x55
db 0xAA
