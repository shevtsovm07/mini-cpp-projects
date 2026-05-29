# HamArc

A command-line archiver that stores files with Hamming error-correction codes. Archives can survive single-bit corruption in any block and detect two-bit errors — without any external ECC library.

## Usage

```sh
# Create an archive
hamarc -c -f archive.haf file1.txt file2.txt image.jpg

# List contents
hamarc -l -f archive.haf

# Extract all files
hamarc -x -f archive.haf

# Append a file to existing archive
hamarc -a -f archive.haf newfile.txt

# Delete a file from archive
hamarc -d -f archive.haf file1.txt

# Merge two archives
hamarc -A -f result.haf archive1.haf archive2.haf
```

## Archive format

Each file is stored as a sequence of Hamming(8,4) encoded blocks. Every 4 bits of original data become 8 bits in the archive — 4 parity bits are interleaved at positions that are powers of 2 (bits 1, 2, 4, 8 in 1-indexed terms).

```
Original:  [d3 d5 d6 d7]  (4 data bits)
Encoded:   [p1 p2 d3 p4 d5 d6 d7 p8]  (8 bits with parity)
```

On extraction, parity is recomputed and compared. A single corrupted bit is located by XOR-ing the failed parity positions and corrected in place. Two corrupted bits are detected but not corrected.

## Implementation notes

- Encoding and decoding operate at the bit level using direct byte manipulation
- The archive stores a header per file: original filename, original byte size, encoded block count
- `Merge` concatenates two archive files at the block level without re-encoding
- `Delete` rewrites the archive, skipping the target file's blocks

## Build

```sh
cmake -S . -B build
cmake --build build
./build/bin/hamarc --help
```

Requires: C++17, CMake ≥ 3.14

## Tech

C++17 · Hamming(8,4) error correction · bitwise encoding · binary file I/O · CMake
