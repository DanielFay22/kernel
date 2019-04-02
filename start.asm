[BITS 32]		; set nasm __BITS__ macro
global start
start:
    mov esp, _sys_stack		; Set esp (stack pointer) to point to new stack.
    jmp run

; Multiboot header must be 4byte aligned
ALIGN 4
mboot:
    ; Multiboot macros
    MULTIBOOT_PAGE_ALIGN	equ 1<<0
    MULTIBOOT_MEMORY_INFO	equ	1<<1
    MULTIBOOT_AOUT_KLUDGE	equ	1<<16
    MULTIBOOT_HEADER_MAGIC	equ	0x1BADB002
    MULTIBOOT_HEADER_FLAGS	equ	MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_AOUT_KLUDGE
    MULTIBOOT_CHECKSUM		equ	-(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
    EXTERN code, bss, end

    ; GRUB Multiboot header.
    dd MULTIBOOT_HEADER_MAGIC
    dd MULTIBOOT_HEADER_FLAGS
    dd MULTIBOOT_CHECKSUM

    ; Must be physical addresses.
    dd mboot
    dd code
    dd bss
    dd end
    dd start

run:
    	extern main
    	call main
    	cli
.hang:	hlt
    	jmp .hang


global gdt_flush
extern gp
gdt_flush:
    lgdt [gp]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:flush2
flush2:
    ret
    
    
; Loads the IDT defined in 'idtp' into the processor.
global idt_load
extern idtp
idt_load:
    lidt [idtp]
    ret

; ISR's
global isr0
global isr1
global isr2
global isr3
global isr4
global isr5
global isr6
global isr7
global isr8
global isr9
global isr10
global isr11
global isr12
global isr13
global isr14
global isr15
global isr16
global isr17
global isr18
global isr19
global isr20
global isr21
global isr22
global isr23
global isr24
global isr25
global isr26
global isr27
global isr28
global isr29
global isr30
global isr31


;   0: Divide By Zero Exception
isr0:
    cli
    push byte 0
    push byte 0
    jmp isr_common_stub

;	1: Debug
isr1:
    cli
    push byte 0
    push byte 1
    jmp isr_common_stub

;	2: Non Maskable Interrupt
isr2:
	cli
	push byte 0
	push byte 2
	jmp isr_common_stub

;	3: Breakpoint Exception
isr3:
	cli
	push byte 0
	push byte 3
	jmp isr_common_stub

;	4: Into Detected Overflow Exception
isr4:
	cli
	push byte 0
	push byte 4
	jmp isr_common_stub

;	5: Out of Bounds Exception
isr5:
	cli
	push byte 0
	push byte 5
	jmp isr_common_stub

;	6: Invalid Opcode Exception
isr6:
	cli
	push byte 0
	push byte 6
	jmp isr_common_stub

;	7: No Coprocessor Exception
isr7:
	cli
	push byte 0
	push byte 7
	jmp isr_common_stub

;	8: Double Fault Exception
isr8:
    cli
    push byte 8
    jmp isr_common_stub

;	9: Coprocessor Segment Overrun Exception
isr9:
    cli
    push byte 0
    push byte 9
    jmp isr_common_stub

;	10: Bad TSS Exception
isr10:
    cli
    push byte 10
    jmp isr_common_stub

;	11: Segment Not Present Exception
isr11:
    cli
    push byte 11
    jmp isr_common_stub

;	12: Stack Fault Exception
isr12:
    cli
    push byte 12
    jmp isr_common_stub

;	13: General Protection Fault Exception
isr13:
    cli
    push byte 13
    jmp isr_common_stub

;	14: Page Fault Exception
isr14:
    cli
    push byte 14
    jmp isr_common_stub

;	15: Unknown Interrupt Exception
isr15:
    cli
    push byte 0
    push byte 15
    jmp isr_common_stub

;	16: Coprocessor Fault Exception
isr16:
    cli
    push byte 0
    push byte 16
    jmp isr_common_stub

;	17: Alignment Check Exception
isr17:
    cli
    push byte 0
    push byte 17
    jmp isr_common_stub

;	18: Machine Check Exception
isr18:
    cli
    push byte 0
    push byte 18
    jmp isr_common_stub

;	19-31: Reserved
isr19:
    cli
    push byte 0
    push byte 19
    jmp isr_common_stub
isr20:
    cli
    push byte 0
    push byte 20
    jmp isr_common_stub
isr21:
    cli
    push byte 0
    push byte 21
    jmp isr_common_stub
isr22:
    cli
    push byte 0
    push byte 22
    jmp isr_common_stub
isr23:
    cli
    push byte 0
    push byte 23
    jmp isr_common_stub
isr24:
    cli
    push byte 0
    push byte 24
    jmp isr_common_stub
isr25:
    cli
    push byte 0
    push byte 25
    jmp isr_common_stub
isr26:
    cli
    push byte 0
    push byte 26
    jmp isr_common_stub
isr27:
    cli
    push byte 0
    push byte 27
    jmp isr_common_stub
isr28:
    cli
    push byte 0
    push byte 28
    jmp isr_common_stub
isr29:
    cli
    push byte 0
    push byte 29
    jmp isr_common_stub
isr30:
    cli
    push byte 0
    push byte 30
    jmp isr_common_stub
isr31:
    cli
    push byte 0
    push byte 31
    jmp isr_common_stub

extern fault_handler

; Common handler for all interupts. It saves the processor state, sets
; up for kernel mode segments, calls the C-level fault handler,
; and finally restores the stack frame.
isr_common_stub:
    pusha
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp
    push eax
    mov eax, fault_handler
    call eax
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret



global irq0
global irq1
global irq2
global irq3
global irq4
global irq5
global irq6
global irq7
global irq8
global irq9
global irq10
global irq11
global irq12
global irq13
global irq14
global irq15

;	PIT
irq0:
	cli
	push 0
	push 32
	jmp irq_common_stub
;	Keyboard
irq1:
	cli
	push 0
	push 33
	jmp irq_common_stub

irq2:
	cli
	push 0
	push 34
	jmp irq_common_stub

irq3:
	cli
	push 0
	push 35
	jmp irq_common_stub

irq4:
	cli
	push 0
	push 36
	jmp irq_common_stub

irq5:
	cli
	push 0
	push 37
	jmp irq_common_stub

irq6:
	cli
	push 0
	push 38
	jmp irq_common_stub

irq7:
	cli
	push 0
	push 39
	jmp irq_common_stub

irq8:
	cli
	push 0
	push 40
	jmp irq_common_stub

irq9:
	cli
	push 0
	push 41
	jmp irq_common_stub

irq10:
	cli
	push 0
	push 42
	jmp irq_common_stub

irq11:
	cli
	push 0
	push 43
	jmp irq_common_stub

irq12:
	cli
	push 0
	push 44
	jmp irq_common_stub

irq13:
	cli
	push 0
	push 45
	jmp irq_common_stub

irq14:
	cli
	push 0
	push 46
	jmp irq_common_stub

irq15:
	cli
	push 0
	push 47
	jmp irq_common_stub



extern irq_handler

irq_common_stub:
	pusha
	push ds
	push es
	push fs
	push gs
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov eax, esp
	push eax
	mov eax, irq_handler
	call eax
	pop eax
	pop gs
	pop fs
	pop es
	pop ds
	popa
	add esp, 8
	iret





SECTION .bss
	align 16
	resb 16384		; 16 KiB stack
_sys_stack: