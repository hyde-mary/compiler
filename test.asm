bits 64
default rel

segment .data
    msg db "Tangina mo!", 0xd, 0xa, 0

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

    lea     rcx, [msg]
    call    printf
    
    ; epilogue
    ; parang return 0;
    xor     rax, rax
    call    ExitProcess