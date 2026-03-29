# ironGit

A lightweight, from-scratch implementation of core Git functionalities, written entirely in C. 

This project aims to recreate the foundational mechanics of Git to understand its internal version control system. It features a custom-built SHA-1 hashing algorithm, CLI argument parsing, and native `zlib` compression for object storage, mirroring how the real Git handles blobs.

## Features

Currently, `irongit` supports repository initialization, raw object database manipulation, advanced file ignoring, and recursively staging files to the Index.

| Command | Description |
| :--- | :--- |
| <nobr>`irongit init`</nobr> | Initializes or reinitializes an empty Git repository in the current directory (`.ironGit/`). |
| <nobr>`irongit hash-object -w <file>`</nobr> | Hashes a file, compresses it, and writes it to the object database. |
| <nobr>`irongit cat-file -p <hash>`</nobr> | Pretty-prints the uncompressed content of a specific object. |
| <nobr>`irongit cat-file -s <hash>`</nobr> | Prints the size of the object in bytes. |
| <nobr>`irongit cat-file -t <hash>`</nobr> | Prints the type of the object (e.g., `blob`). |
| <nobr>`irongit add <file>`</nobr> | Stages a single file. Generates its Blob and updates/creates the binary `.ironGit/index` file. |
| <nobr>`irongit add .`</nobr> | Recursively scans the directory tree, applies `.irongitignore` rules, sorts files alphabetically, and writes the entire `.ironGit/index` binary in one go. |

## Technical Details

* **Language:** C
* **Hashing:** Uses a hand-crafted, custom SHA-1 implementation to generate 40-character hex hashes.
* **Compression:** Utilizes `zlib` at compression level 1 (`Z_BEST_SPEED`) for fast read/write operations.
* **Object Storage Format:** Objects are stored exactly like real Git. The format is `<type> <size>\0<content>`.
* **File Structure:** When an object is saved, the first 2 characters of the SHA-1 hash are used as the directory name, and the remaining 38 characters become the compressed file's name.
* **Index Architecture:** The `.ironGit/index` binary file is generated using a dynamic buffer approach (`malloc`/`realloc`) to perfectly pre-calculate the global 20-byte SHA-1 hash before writing to disk.
* **Directory Traversal & Ignore System:** Implements fully recursive directory scanning. It automatically skips restricted internal directories (like `.git`) and parses `.irongitignore.txt` to safely exclude files based on exact matches or wildcard extensions (e.g., `*.exe`).
* **Sorting:** Includes a custom-built Merge Sort algorithm to order staging area entries alphabetically by filename, ensuring 100% compatibility with the original Git's index parser.

## Building the Project

### Prerequisites
You need a C compiler (like `gcc`) and the `zlib` development libraries installed on your system.

### Compilation
A `Makefile` is provided to easily compile the source code. If you are on Windows using MinGW, run the following command in the root directory:

```bash
mingw32-make
```
*(Note: On Linux or macOS environments, you can simply use `make`)*

This executes the following build command, linking the `zlib` library:
`gcc -Wall index.c main.c objects.c utils/*.c -o irongit.exe -lz -lws2_32`

## Usage Example

```bash
# Initialize the repository
$ irongit init
Reinitialized existing Git repository.

# Stage all files recursively, ignoring what's in .irongitignore.txt
$ irongit add .

# Verify compatibility with the REAL Git!
$ git ls-files --stage
100644 9fb7bccf006507a7f90d438dc442e60a73cf601d 0       ./test/dwa.txt
100644 438fc5d996b802d973388a4f6a45f5dfc5242921 0       ./test/wad.txt
100644 a6f34c09cbe1eab96ff2932dc839798ec6c3f694 0       ./utils/utils.c

# Inspect an object type from the staging area
$ irongit cat-file -t 9fb7bccf006507a7f90d438dc442e60a73cf601d
blob
```

## Work In Progress 🚧

1. **The Commit Command:** Parsing the generated index to create `tree` objects (directory snapshots) and `commit` objects (author metadata, timestamp, and messages) to complete the basic Git lifecycle.
2. **Reading the History:** Implementing a basic `irongit log` to traverse the commit chain and display the repository's history.