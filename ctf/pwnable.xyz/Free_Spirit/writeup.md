## Observation
pHeapChunk is allocated when main() starts

When input is 0, free pHeapChunk and exit

When input is 1, read(stdin, pHeapChunk, 0x20) is called

When input is 2, print &pHeapChunk

When input is 3, (i128) *pHeapChunk is written to vulnVar and overflows, allowing us to modify pHeapChunk
- Done through movdqu instruction



steps
// Input 2. Get &pHeapChunk and calculate &returnAddr

// Input 1
*pHeapChunk = [deadc0de&winAddr]

// Input 3, overflow vulnVar and write pHeapChunk
pHeapChunk = &returnAddr

// Input 1. Null is there so we can set pHeapChunk to NULL
*pHeapChunk = [&winAddr&NULL]

// Input 3, oerflow vulnvar and write pHeapChunk
pHeapChunk = NULL

// Input 0
Now we can exit since free() won't do anything with null pointer :)
A little leak is ok...

the pointer cannot be null!


create first fake chunk at 0x0000000000601050