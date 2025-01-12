section .text
global _start

extern MessageBoxA
extern ExitProcess

_start:
    ; Example assembly code to print a message (for debugging purposes)
    ; Using Windows API for printing
    push 0          ; uType = MB_OK
    push message    ; lpCaption
    push message    ; lpText
    push 0          ; hWnd
    call MessageBoxA

    ; Exit the kernel
    push 0          ; uExitCode
    call ExitProcess

section .data
message db "Hello from assembly!", 0