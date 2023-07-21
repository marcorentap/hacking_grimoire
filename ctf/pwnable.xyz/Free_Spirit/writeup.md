## Observation
pHeapChunk is allocated when main() starts

When input is 0, free pHeapChunk

When input is 1, read(stdin, pHeapChunk, 0x20) is called
- So we can write to heap by entering 1

When input is 2, print &pHeapChunk

When input is 3, (i128) *pHeapChunk is written to vulnVar, allowing us to modify the return address
- Done through movdqu instruction
