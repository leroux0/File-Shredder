# Secure File Shredder

A simple command-line utility written in C that securely shreds files by overwriting them multiple times with a specified pattern (zeros or random data) before deletion. This helps prevent data recovery and supports data privacy compliance (e.g., GDPR).

## Features
- Overwrite files with zeros or pseudo-random data.
- Configurable number of passes (default: 3).
- Low-level file I/O for control and efficiency.
- Cross-platform support (Unix-like systems and Windows via MinGW).

## Usage
Compile the program (e.g., `gcc shredder.c -o shredder`) and run it with options:

```
./shredder -f <file> [-n <passes>] [-p <pattern>]
```

- `-f <file>`: Path to the file to shred (required).
- `-n <passes>`: Number of overwrite passes (default: 3, must be >0).
- `-p <pattern>`: "zeros" or "random" (default: "zeros").

Example:
```
./shredder -f sensitive.txt -n 5 -p random
```

This will overwrite `sensitive.txt` 5 times with random data, sync to disk, and delete it.

## Compilation
- On Unix-like: `gcc shredder.c -o shredder`
- On Windows (MinGW): Same command; uses equivalents for POSIX functions.

## Limitations
- **Storage Type Dependency**: Effective on traditional HDDs (magnetic drives) but unreliable on SSDs, USB flash drives (pendrives), or other solid-state storage due to wear-leveling, over-provisioning, and TRIM/garbage collection. Old data may persist in hidden areas—use hardware-level secure erase or full-disk encryption instead.
- **Randomness**: Uses `rand()` for "random" pattern, which is pseudo-random and not cryptographically secure (seeded by time). For stronger randomness, consider `/dev/urandom` (Unix-only).
- **Scope**: Handles single files only; no support for directories, recursive deletion, or multiple files. Empty files are deleted without overwriting.
- **Platform Issues**: Synchronous writes (`O_SYNC`) are not supported on Windows, potentially reducing reliability. File systems with journaling (e.g., NTFS, ext4) may leave traces.
- **Security**: Not guaranteed against advanced forensic recovery. For critical data, combine with encryption tools like VeraCrypt.
- **Portability**: Tested on basic setups; may need adjustments for very large files or exotic file systems.

Always test on non-critical files. This is a educational/demo tool—not a replacement for professional secure deletion software like GNU `shred` or BleachBit.

## License
MIT License. Feel free to modify and distribute.
