# EXT2 Filesystem Parser & File Manager

A C++ program that directly reads, parses, and modifies an **EXT2 filesystem disk image** at the byte level.

Instead of using normal operating-system filesystem APIs, this project manually locates and interprets EXT2 structures such as the **Superblock, Block Group Descriptors, inode tables, directory entries, bitmaps, and data blocks**.

The filesystem image used by the project is:

```text
disk-backpup.img
```

> **Warning:** This program opens the filesystem image with read/write permissions. Always keep a backup of the image before testing file update operations.

---

## Project Overview

The project provides a small command-line interface for exploring and modifying an EXT2 filesystem image.

The main filesystem flow implemented by the project is:

```text
                 EXT2 Disk Image
                       |
                       v
                  Superblock
                       |
                       v
             Block Group Descriptor
                       |
          +------------+------------+
          |            |            |
          v            v            v
     Block Bitmap  Inode Bitmap  Inode Table
                                      |
                                      v
                                    Inode
                                      |
                              +-------+-------+
                              |               |
                              v               v
                       Directory Entries   Data Blocks
                              |               |
                              v               v
                           Filename       File Contents
```

The program can:

- Inspect filesystem metadata
- Traverse directories
- Change the current directory
- Find files
- Read file contents
- Overwrite existing files
- Append data to existing files
- Allocate additional data blocks when required
- Update filesystem metadata after allocation

---

# Features

## 1. Superblock Parsing

The program reads the EXT2 Superblock from its standard location at byte offset:

```text
1024
```

It extracts important filesystem information including:

- Total number of inodes
- Total number of blocks
- Reserved blocks
- Free blocks
- Free inodes
- First data block
- Block size
- Blocks per group
- Inodes per group
- Inode size

Run:

```text
prompt => superblock
```

Example:

```text
===== SUPERBLOCK =====
Inodes count      : ...
Blocks count      : ...
Free blocks       : ...
Free inodes       : ...
First data block  : ...
Block size        : ...
Blocks per group  : ...
Inodes per group  : ...
Inode size        : ...
```

---

## 2. Block Group Descriptor Reading

EXT2 divides the filesystem into block groups. Each group has a Block Group Descriptor containing locations and information about important filesystem structures.

The program displays:

- Block usage bitmap location
- Inode usage bitmap location
- Inode table location
- Number of free blocks
- Number of free inodes
- Number of directories

Run:

```text
prompt => bgd
```

---

## 3. Recursive Directory Traversal

The `traverser()` functionality starts from the root directory, inode `2`, and recursively explores directories.

The traversal follows:

```text
Directory Inode
      |
      v
Directory Data Blocks
      |
      v
Directory Entries
      |
      v
Filename + Inode Number
      |
      v
Child Directory
      |
      v
Repeat
```

Directory entries contain:

```text
+0   inode number
+4   record length (rec_len)
+6   name length (name_len)
+7   file type
+8   filename
```

The program uses `rec_len` to locate the next directory entry.

The special entries:

```text
.
..
```

are skipped during recursive traversal.

Example:

```text
Name: home
Inode Number : 12

    Name: user
    Inode Number : 15

        Name: notes.txt
        Inode Number : 18
```

### Indirect Block Support

Unlike file reading/updating, directory traversal supports all levels of EXT2 block pointers:

```text
Direct
   |
   v
Single Indirect
   |
   v
Double Indirect
   |
   v
Triple Indirect
```

This is handled using the project's block-processing functions such as:

```text
process_block()
process_indirect()
traverser()
```

---

# 4. Change Directory

The program maintains a `current_dir` inode, initially set to the root inode:

```text
2
```

The command:

```text
cd <directory_name>
```

changes the current directory.

Example:

```text
prompt => cd home
changed directory to: home
```

Before changing directories, the program checks the directory entry's file type.

Therefore, attempting:

```text
cd file.txt
```

will not treat a regular file as a directory.

---

# 5. Finding Files

The `find_file()` function searches for a filename inside the current working directory.

The process is:

```text
Current Directory
       |
       v
Directory Data Blocks
       |
       v
Directory Entries
       |
       v
Compare Filename
       |
       v
Inode Number
```

If the file is found, its inode number is returned.

If it is not found, the function returns:

```text
0
```

This functionality is used by both:

```text
read
update
```

---

# 6. Reading File Contents

The program can locate and display the contents of an existing file.

The process is:

```text
Filename
   |
   v
find_file()
   |
   v
Inode Number
   |
   v
Read Inode
   |
   v
Read i_size
   |
   v
Read i_block[]
   |
   v
Data Blocks
   |
   v
File Contents
```

The inode's `i_size` is used to determine the exact number of bytes that belong to the file.

This prevents unused bytes in the final filesystem block from being printed.

Run:

```text
prompt => read
filename
```

Example:

```text
prompt => read
readthis.txt

Hello from EXT2!
```

### Current Limitation

File reading currently uses the inode's **12 direct block pointers**:

```text
i_block[0] ... i_block[11]
```

Therefore, files requiring indirect block addressing are not currently supported by the file-reading implementation.

---

# 7. Updating Existing Files

The `update` command allows an existing file to be modified in two ways:

```text
1. Overwrite
2. Append
```

Run:

```text
prompt => update
```

The program asks for:

1. Filename
2. Update type
3. Data to write

Example:

```text
prompt => update

Enter Filename: test.txt

1. Overwrite
2. Append
Enter choice: 2

Enter data: More data
```

---

## Overwrite

Overwrite starts writing from the beginning of the file.

Example:

```text
Old:
Hello World

New data:
Goodbye

Result:
Goodbye
```

The inode's `i_size` is updated to reflect the new file size.

---

## Append

Append starts writing at the current end of the file.

Example:

```text
Existing:
Hello

Data:
 World

Result:
Hello World
```

The new file size becomes:

```text
old_size + new_data_size
```

---

# 8. Block Allocation

When an update requires more filesystem space, the program searches the EXT2 block bitmap for a free block.

The allocation process is:

```text
File requires more space
          |
          v
Read Block Bitmap
          |
          v
Find free bit
          |
          v
Calculate filesystem block number
          |
          v
Mark block as allocated
          |
          v
Update inode block pointer
          |
          v
Write file data
          |
          v
Update filesystem counters
```

In the block bitmap:

```text
0 -> Free
1 -> Allocated
```

The program checks each bit using:

```cpp
byte = i / 8;
bit  = i % 8;
```

A free block is marked as allocated using:

```cpp
buffer[byte] |= (1 << bit);
```

---

# 9. Updating Filesystem Metadata

When a new block is allocated, the program updates the relevant filesystem metadata.

This includes:

```text
Block Bitmap
      |
      v
Block Group Descriptor
      |
      v
Superblock
      |
      v
Inode
```

Specifically, it updates:

- Block bitmap
- Block Group Descriptor free-block count
- Superblock free-block count
- Inode block pointer
- Inode file size

This is important because filesystem metadata must remain consistent with the actual data stored in the image.

---

# EXT2 Architecture Used

## Superblock

The Superblock stores global filesystem information.

The project reads it from:

```text
byte offset = 1024
```

The block size is calculated using:

```cpp
uint32_t block_size = 1024 << s_log_block_size;
```

For example:

```text
s_log_block_size = 0 -> 1024 bytes
s_log_block_size = 1 -> 2048 bytes
s_log_block_size = 2 -> 4096 bytes
```

---

## Block Group Descriptor

The Block Group Descriptor provides the locations of structures such as:

```text
Block Bitmap
Inode Bitmap
Inode Table
```

It also contains free-block, free-inode, and directory counts.

---

## Inodes

An inode represents a file or directory and stores metadata about it.

Important fields used by this project include:

```text
i_size
i_block[]
```

The inode contains 15 block pointers:

```text
i_block[0]  - i_block[11] -> Direct blocks
i_block[12]              -> Single indirect
i_block[13]              -> Double indirect
i_block[14]              -> Triple indirect
```

The project uses indirect pointers for directory traversal.

File reading and updating currently use only the 12 direct pointers.

---

# Directory Entry Structure

An EXT2 directory is composed of directory entries.

Each entry follows this layout:

```text
Offset 0   : inode number   (4 bytes)
Offset 4   : rec_len        (2 bytes)
Offset 6   : name_len       (1 byte)
Offset 7   : file_type      (1 byte)
Offset 8   : filename
```

The `file_type` value used by the program includes:

```text
1 -> Regular file
2 -> Directory
```

`rec_len` tells the program how many bytes to move forward to reach the next directory entry.

---

# Inode Location

To locate an inode, the program determines:

```text
Inode Number
      |
      v
Block Group
      |
      v
Index inside Block Group
      |
      v
Inode Table
      |
      v
Actual Inode
```

The inode table location comes from the corresponding Block Group Descriptor.

---

# Little-Endian Conversion

EXT2 stores multi-byte values using **little-endian byte order**.

The project contains conversion functionality for both:

```text
Raw bytes -> Integer
```

and:

```text
Integer -> Raw bytes
```

For example:

```text
80 00 00 00 -> 128
00 04 00 00 -> 1024
```

These conversions are important when reading and modifying filesystem metadata.

They are used for values such as:

- Inode file size
- Inode block pointers
- Free block counts
- Superblock values
- Block Group Descriptor values

---

# Project Structure

The main implementation is contained in a C++ source file.

Important structures and functions include:

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

# Available Commands

| Command | Description |
|---|---|
| `superblock` | Display EXT2 Superblock information |
| `bgd` | Display Block Group Descriptor information |
| `traverser` | Recursively traverse the filesystem |
| `cd <name>` | Change the current directory |
| `read` | Read an existing file |
| `update` | Overwrite or append to an existing file |
| `exit` | Exit the program |

---

# Compilation

The project requires a C++ compiler such as `g++`.

Compile:

```bash
g++ main.cpp -o ext2
```

Run:

```bash
./ext2
```

---

# Filesystem Image

The program currently opens:

```text
/home/rudra/projects/disk-backpup.img
```

The image is opened using:

```cpp
ios::in | ios::out | ios::binary
```

because the program performs both read and write operations.

If the image is located elsewhere, update the path in the source code before compilation.

---

# Example Session

```text
prompt => superblock

===== SUPERBLOCK =====
Inodes count      : ...
Blocks count      : ...
Free blocks       : ...
Free inodes       : ...
First data block  : ...
Block size        : ...
Blocks per group  : ...
Inodes per group  : ...
Inode size        : ...


prompt => traverser

Name: home
Inode Number : 12

    Name: test.txt
    Inode Number : 15


prompt => cd home

changed directory to: home


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

# Limitations

This is an **educational EXT2 filesystem implementation**, not a complete production filesystem driver.

Current limitations include:

- File reading uses only the 12 direct inode block pointers.
- File updating uses only the 12 direct inode block pointers.
- Files requiring indirect blocks cannot currently be read or updated by those functions.
- Directory traversal supports direct, single-indirect, double-indirect, and triple-indirect blocks.
- The filesystem image path is currently hard-coded.
- The implementation is designed around the project's EXT2 filesystem image rather than being a complete general-purpose EXT2 implementation.
- The program does not implement every EXT2 filesystem feature.

---

# Project Goal

The main goal of this project is to understand how a filesystem works internally by interacting directly with a raw filesystem image.

Instead of simply asking the operating system to open a file, the program manually performs the process:

```text
Raw Disk Image
      |
      v
Superblock
      |
      v
Block Groups
      |
      v
Bitmaps + Inode Tables
      |
      v
Inodes
      |
      v
Directory Entries
      |
      v
Data Blocks
      |
      v
File Contents
```

The project demonstrates how filesystem metadata connects a filename to the actual bytes stored on disk.

It also demonstrates why modifying a filesystem requires more than simply changing file data: **metadata such as block allocation information, inode size, block pointers, and free-block counters must also be kept consistent.**

---

# Safety

Because the program opens the filesystem image with write permissions, incorrect modifications can corrupt the image.

Always keep a backup of:

```text
disk-backpup.img
```

before testing:

```text
update
```

or any other operation that modifies filesystem data.

---

# Technologies Used

- C++
- `fstream`
- Binary file I/O
- EXT2 filesystem structures
- Inodes
- Directory entries
- Block and inode bitmaps
- Recursion
- Direct and indirect block handling
- Low-level byte manipulation
- Little-endian conversion
