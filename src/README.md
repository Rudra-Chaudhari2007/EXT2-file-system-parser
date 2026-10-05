# EXT2 Filesystem Parser and File Manager

A C++ program that reads and modifies an **EXT2 filesystem disk image**. The project works directly with the filesystem structures stored inside the image instead of relying on the operating system's filesystem APIs.

## Features

The program currently supports:

- Reading and displaying the EXT2 **Superblock**
- Reading and displaying **Block Group Descriptors**
- Traversing directories recursively
- Displaying directory entries and inode numbers
- Changing the current working directory using `cd`
- Finding a file inside the current working directory
- Reading and displaying file contents
- Overwriting an existing file
- Appending data to an existing file
- Finding free blocks using the block usage bitmap
- Allocating blocks and updating filesystem free-block counts
- Updating the inode's file size
- Handling direct, single-indirect, double-indirect, and triple-indirect blocks while traversing directories

> **Current file read/write limitation:** file reading and updating currently use the 12 direct inode block pointers. The directory traversal code can additionally follow indirect pointers.

---

## Project Structure

The main program is implemented in a single C++ source file.

Important components:

```text
Superblock
Block_Group_Descriptor
Inode
converter()
name_converter()

read_superblock()
print_superblock()

read_block_group_descriptor()
print_block_group_descriptor()

read_file()

process_block()
process_indirect()
traverser()

find_file()
change_directory()

find_free_block()
allocate_block()
update_inode_size()
update_file()

main()
```

---

## EXT2 Concepts Used

The program works directly with important EXT2 filesystem structures.

### 1. Superblock

The Superblock contains global information about the filesystem, such as:

- Total number of inodes
- Total number of blocks
- Number of free blocks
- Number of free inodes
- First data block
- Block size information
- Blocks per group
- Inodes per group
- Inode size

The program reads the Superblock from byte offset `1024` in the disk image.

```cpp
file.seekg(1024);
```

The block size is calculated using:

```cpp
uint32_t block_size = 1024 << sb.log_block_size;
```

---

### 2. Block Group Descriptor

Each block group has a descriptor containing addresses of important structures:

- Block usage bitmap
- Inode usage bitmap
- Inode table
- Number of unallocated blocks
- Number of unallocated inodes
- Number of directories

The program reads block group descriptors starting from byte offset `2048`.

---

### 3. Inodes

An inode stores information about a file or directory.

This project uses:

- `i_size` at offset `+4`
- `i_block[]` starting at offset `+40`

The inode contains 15 block pointers:

```text
i_block[0]  - i_block[11]  → Direct blocks
i_block[12]               → Single indirect
i_block[13]               → Double indirect
i_block[14]               → Triple indirect
```

---

## Directory Entries

An EXT2 directory contains directory entries.

Each entry contains:

```text
+0   : inode number      (4 bytes)
+4   : record length     (2 bytes)
+6   : name length       (1 byte)
+7   : file type         (1 byte)
+8   : filename
```

The program reads these fields using:

```cpp
uint32_t entry_inode = converter(buffer + len, 4);
uint32_t rec_len = converter(buffer + len + 4, 2);
uint32_t name_len = converter(buffer + len + 6, 1);
uint32_t file_type = converter(buffer + len + 7, 1);
```

File types used by the program:

```text
1 → Regular file
2 → Directory
```

`rec_len` tells the program how many bytes to move forward to reach the next directory entry.

---

## Directory Traversal

The `traverser()` function starts from an inode and follows its block pointers.

It processes:

```text
Direct blocks
     ↓
Single indirect
     ↓
Double indirect
     ↓
Triple indirect
```

The `process_block()` function reads directory entries from a single directory data block.

The `process_indirect()` function recursively follows indirect block pointers.

The traversal also uses a `depth` value to indent nested directories.

Example:

```text
Name: folder1
Inode Number : 12

    Name: folder2
    Inode Number : 15

        Name: file.txt
        Inode Number : 18
```

The special directory entries `.` and `..` are skipped during recursive traversal.

---

## Finding a File

The `find_file()` function searches for a filename inside the **current working directory**.

It:

1. Finds the inode of the current directory.
2. Reads its direct data blocks.
3. Examines every directory entry.
4. Compares the entry name with the requested filename.
5. Returns the inode number if found.
6. Returns `0` if the file is not found.

This function is used by both the `read` and `update` commands.

---

## Changing Directory

The `change_directory()` function searches the current directory for a requested name.

It checks:

```cpp
if(file_type == 2)
```

to make sure that the requested entry is a directory.

If it is a directory, its inode number becomes the new `current_dir`.

Example:

```text
prompt => cd folder1
changed directory to: folder1
```

If the entry exists but is a regular file:

```text
file.txt is not a directory!
```

If the directory does not exist:

```text
Directory not found!
```

---

## Reading a File

The `read_file()` function reads and displays the contents of a file.

The process is:

```text
Filename
   ↓
find_file()
   ↓
Inode number
   ↓
Read inode
   ↓
Read file size
   ↓
Read direct block pointers
   ↓
Read data blocks
   ↓
Print contents
```

The function uses the file size stored in the inode so that it does not print unused bytes from the final block.

The current implementation reads the first 12 direct data blocks.

---

## Finding a Free Block

The `find_free_block()` function searches the block usage bitmap.

In the bitmap:

```text
0 → Free block
1 → Allocated block
```

For every block:

```cpp
uint32_t byte = i / 8;
uint32_t bit = i % 8;
```

The program checks whether the corresponding bit is zero.

When a free block is found, the actual filesystem block number is calculated and returned.

---

## Allocating a Block

The `allocate_block()` function:

1. Calls `find_free_block()`.
2. Finds the free block.
3. Marks the corresponding bitmap bit as allocated.
4. Decreases the block group's free-block count.
5. Decreases the Superblock's free-block count.
6. Writes the updated values back to the disk image.

The bitmap update is performed using:

```cpp
buffer[byte] |= (1 << bit);
```

This sets only the selected bit to `1` while preserving the other bits in that byte.

---

## Updating Files

The `update_file()` function supports two operations:

### Overwrite

```text
Old:
Hello World

New data:
Goodbye

Result:
Goodbye
```

The new data starts at position `0`.

### Append

```text
Old:
Hello

New data:
 World

Result:
Hello World
```

The new data starts at the old file size.

The function calculates:

```cpp
required_blocks =
    (new_size + block_size - 1) / block_size;
```

This determines how many blocks are required to store the new file.

If additional blocks are needed, `allocate_block()` is called and the new block pointer is written into the inode.

Finally, the inode's file size is updated.

---

## Important Current Limitation

The current `update_file()` implementation allocates and uses the **12 direct block pointers**:

```text
i_block[0] ... i_block[11]
```

Therefore, the current file update functionality should not be used for files requiring more than 12 data blocks.

The directory traversal implementation, however, already contains support for:

- Single indirect blocks
- Double indirect blocks
- Triple indirect blocks

Future work can extend `read_file()` and `update_file()` to use those indirect blocks as well.

---

## Disk Image

The program currently opens:

```text
/home/rudra/projects/disk-backpup.img
```

The image is opened using:

```cpp
fstream file(
    "/home/rudra/projects/disk-backpup.img",
    ios::in | ios::out | ios::binary
);
```

This allows both reading and writing.

Make sure the disk image exists at this location before running the program.

---

## Compilation

Compile using a C++ compiler such as `g++`:

```bash
g++ main.cpp -o ext2
```

Run:

```bash
./ext2
```

---

## Available Commands

After starting the program, the program displays:

```text
prompt =>
```

### `superblock`

Displays the contents of the Superblock.

```text
prompt => superblock
```

---

### `bgd`

Displays the Block Group Descriptors.

```text
prompt => bgd
```

---

### `traverser`

Traverses the filesystem starting from the root directory (inode `2`).

```text
prompt => traverser
```

---

### `cd`

Changes the current working directory.

```text
prompt => cd folder_name
```

Example:

```text
prompt => cd test
changed directory to: test
```

---

### `read`

Reads and displays a file from the current working directory.

```text
prompt => read
filename
```

Example:

```text
prompt => read
readthis.txt
```

If the file exists, its contents are printed.

If it does not exist:

```text
File Not Found!!
```

---

### `update`

Updates an existing file.

```text
prompt => update
Enter Filename: test.txt

1. Overwrite
2. Append
Enter choice:
```

For overwrite:

```text
Enter choice: 1
Enter data: New contents
```

For append:

```text
Enter choice: 2
Enter data: Additional contents
```

If the file does not exist:

```text
File Not Found
```

---

### `exit`

Exits the program.

```text
prompt => exit
```

---

## Example Usage

A typical session can look like:

```text
prompt => superblock

===== SUPERBLOCK =====
Inodes count      : ...
Blocks count      : ...
Free blocks       : ...
Free inodes       : ...
Block size        : ...
Blocks per group  : ...
Inodes per group  : ...
Inode size        : ...

prompt => traverser

Name: folder
Inode Number : 12

    Name: test.txt
    Inode Number : 15

prompt => cd folder
changed directory to: folder

prompt => read
test.txt
Hello from EXT2!

prompt => update
Enter Filename: test.txt

1. Overwrite
2. Append
Enter choice: 2
Enter data: More data

prompt => read
test.txt
Hello from EXT2!More data

prompt => exit
```

---

## Byte Conversion

EXT2 stores multi-byte values in little-endian format.

The program uses:

```cpp
uint32_t converter(uint8_t* buffer, int n)
```

to convert bytes from the disk image into an integer.

It also uses an overloaded version:

```cpp
void converter(uint8_t* buffer, uint32_t value, int n)
```

to convert an integer back into bytes before writing it to the disk image.

This is used when updating:

- Inode file size
- Inode block pointers
- Block Group Descriptor free-block count
- Superblock free-block count

---

## Technologies Used

- C++
- File streams (`fstream`)
- Binary file I/O
- EXT2 filesystem structures
- Recursion
- Bitmaps
- Inode and block-pointer handling

---

## Project Goal

The goal of this project is to understand how a filesystem works internally by directly interacting with an EXT2 disk image.

Instead of using normal filesystem functions such as:

```cpp
fopen()
fread()
fwrite()
```

on normal files, the program manually locates:

```text
Superblock
    ↓
Block Group Descriptor
    ↓
Inode Table
    ↓
Inode
    ↓
Data Blocks
    ↓
Directory Entries / File Contents
```

This demonstrates how an operating system can locate, read, and modify files using filesystem metadata.
