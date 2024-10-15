bits 64
default rel

section .bss
    x resd 1        ; Reserve 4 bytes for int x
    

section .data
    fmt db "%d", 10, 0                ; Null terminator for the buffer


segment .text
    global main
    extern ExitProcess

    extern printf

    main:


        ; parang int main
        ; prologe
        push    rbp
        mov     rbp, rsp
        sub     rsp, 32

        mov dword [x], 5
        mov rax, [x]
        add rax, 5

        mov [x], rax

        ; Prepare arguments for printf
        mov rcx, fmt            ; First argument: pointer to format string
        mov rdx, [x]           ; Second argument: integer value to print
        call printf             ; Call printf

        
        ; epilogue
        ; parang return 0;
        mov rax, 0
        call    ExitProcess