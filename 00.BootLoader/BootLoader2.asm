[ORG 0x00]
[BITS 16]

SECTION .text

jmp 0x1000:START

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;	Preferences data about MINT64 OS
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
TOTALSECTORCOUNT: dw 0x02 ; size of MINT64 OS ima except boot loader
KERNEL32SECTORCOUNT: dw 0x02


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;	Code
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
START:
    mov ax, 0x1000
    mov ds, ax
    mov ax, 0xB800
    mov es, ax

	push MESSAGE1
	push 0
	push 0
	call PRINTMESSAGE
	add sp, 6

	push CURRENT
	push 1
	push 0
	call PRINTMESSAGE
	add sp, 6

	;	Print Date
	mov ah, 0x04
	int 0x1A

	mov bx, DATE
	mov ah, dl
	call READDATE
	add sp, 6

	mov ah, dh
	call READDATE
	add sp, 6

	mov ah, ch
	call READDATE
	add sp, 6

	sub bx, 1
	mov ah, cl
	call READDATE
	add sp, 6

	push DATE
	push 1
	push 14
	call PRINTMESSAGE
	add sp, 6

	push IMAGELOADINGMESSAGE
	push 2 
	push 0
	call PRINTMESSAGE
	add sp, 6
	
	;	RAM

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;	OS img loading from Disk
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;	Reset before read Disk
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

RESETDISK:
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;	Call BIOS Reset Function
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;	Service num 0, drive num(0 = Floppy)
	mov ax, 0
	mov dl, 0
	int 0x13
	;	If error occured, call HANDLEDISKERROR
	jc HANDLEDISKERROR

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;	Read sector from Disk
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;	Set the address to copy the contents
	;	of the disk into memory at 0x1020
	mov si, 0x1020
	mov es, si
	mov bx, 0x0000

	mov di, word [ TOTALSECTORCOUNT ]

READDATA: ;	Read the disk
	cmp di, 0
	je READEND
	sub di, 0x1

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;	Call Bios Read Function
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov ah, 0x02
	mov al, 0x1
	mov ch, byte [ TRACKNUMBER ]
	mov cl, byte [ SECTORNUMBER ]
	mov dh, byte [ HEADNUMBER ]
	mov dl, 0x00
	int 0x13
	jc HANDLEDISKERROR

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;	Calculate the address to copy, track,
	;	head, address of the sector
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	add si, 0x0020
	mov es, si

	mov al, byte [ SECTORNUMBER ]
	add al, 0x01
	mov byte [ SECTORNUMBER ], al
	cmp al, 19
	jl READDATA

	xor byte [ HEADNUMBER ], 0x01
	mov byte [ SECTORNUMBER ], 0x01

	cmp byte [ HEADNUMBER ], 0x00
	jne READDATA

	add byte [ TRACKNUMBER ], 0x01
	jmp READDATA

READEND:
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;	Print message 'OS img complete..'
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	push LOADINGCOMPLETEMESSAGE
	push 2
	push 20
	call PRINTMESSAGE
	add sp, 6

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;	Execute OS img
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	jmp 0x1020:0x0000

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;	Function code
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
HANDLEDISKERROR:
	push DISKERRORMESSAGE
	push 2 
	push 20
	call PRINTMESSAGE

	jmp $

READDATE:
	mov al, ah
	shr al, 4
	add al, 0x30
	mov [bx], al
	inc bx
	mov al, ah
	shl al, 4
	shr al, 4
	add al, 0x30
	mov [bx], al
	add bx, 2
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
DISKERRORMESSAGE:		db 'DISK Error~!!', 0
IMAGELOADINGMESSAGE:	db 'OS Image Loading...', 0
LOADINGCOMPLETEMESSAGE:	db 'Complete~!!', 0
MESSAGE1: db 'MINT64 OS Boot Loader Start~!!', 0
CURRENT: db 'Current Data:', 0
DATE: db '00/00/0000', 0
RSIZE: db 'RAM Size: XX MB', 0
LENGTH db 0
;	Var about reding disk
SECTORNUMBER:	db 0x03
HEADNUMBER:		db 0x00
TRACKNUMBER:	db 0x00

times 510 - ( $ - $$ )    db    0x00

db 0x55
db 0xAA
