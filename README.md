# simplefs

A very simple filesystem - just to get things going.

* All files currently have a maximum fixed size of 64 kB. Files are stored in a contiguous area in the storage device.
* Up to 1008 files can be created in one partition.
* The volume must be of at least of 64 MB.
* There is no directory support.

The first 512 bytes are loaded into RAM on computer initialization, and the operating system needs to
bootstrap itself from it.

## Boot sector (sector zero)

| Byte  | Length | Description |
|-------|--------|-------------|
| `00` | 4      | Jump instruction |
| `04` | 4      | Signature (`9f 45 a8 c3`) |
| `08` | 2      | SimpleFS version |
| `0A` | 2      | Link to boot file entry |
| `0C` | Up to `1FF` | Boot code |

## File list (sectors 1 to 63)

Each file entry has 32 bytes, organized like this:

| Byte | Length | Description |
|------|--------|-------------|
| `00` | 1      | File status (0: end of list; 1: file, 2: deleted file) |
| `01` | 1      | Reserved |
| `02` | 2      | File size (up to 64k)
| `04` | 4      | File date |
| `08` | 24     | File name |

## File contents (sectors 64 to 129.088)

Formula to find file content sector: `C = (i * 128) + 64`.
