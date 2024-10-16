default rel
extern scanf
extern printf

section .data
    
    format_in db "%d", 0
    format_out db "%d", 10, 0

section .bss
    number resq 1
    num resq 1

section .text
global main

main:
    ; Preserve stack alignment
    push rbp
    mov rbp, rsp
    sub rsp, 32  ; Shadow space for Windows API calls

    ; Call scanf
    lea rcx, [format_in]
    lea rdx, [number]
    call scanf

    lea rcx, [format_in]
    lea rdx, [num]
    call scanf

    ; Print the result
    mov rax, [number]
    add rax, [num]

    lea rcx, [format_out]
    mov rdx, rax
    call printf

    ; Exit the program
    xor eax, eax
    leave
    ret
