import struct
import os

import sys
def parse_args(build_dir, output_file, padding):
    argv = sys.argv[1:]

    if len(argv) >= 1:
        build_dir = argv[0]

    if len(argv) >= 2:
        output_file = argv[1]

    if len(argv) >= 3:
        padding = int(argv[2])
    
    return build_dir, output_file, padding

def discover_apps(build_dir, name_max_len=32):
    apps = []

    for root, dirs, files in os.walk(build_dir):
        if "app.bin" in files:
            app_bin_path = os.path.join(root, "app.bin")

            # Only the immediate parent folder name
            app_name = os.path.basename(root)

            name_bytes = app_name.encode()
            if len(name_bytes) >= name_max_len:
                raise ValueError(f"App name '{app_name}' too long")

            apps.append((app_name, app_bin_path))

    return apps


def load_existing_padding_if_valid(path):
    if not os.path.exists(path):
        return None

    with open(path, "rb") as f:
        header = f.read(8)
        if len(header) != 8:
            return None

        magic, count = struct.unpack("<II", header)
        if magic != MAGIC_NUMBER:
            return None

        # Read entries
        entries = []
        for _ in range(count):
            data = f.read(ENTRY_STRUCT.size)
            if len(data) != ENTRY_STRUCT.size:
                return None
            name, offset, size = ENTRY_STRUCT.unpack(data)
            entries.append((offset, size))

        if not entries:
            return None

        # Padding starts after the last app
        last_offset, last_size = max(entries, key=lambda x: x[0])
        padding_start = last_offset + last_size

        f.seek(0, os.SEEK_END)
        file_size = f.tell()

        if padding_start > file_size:
            return None

        f.seek(padding_start)
        padding_data = f.read()

        print(f"Existing image valid. Preserving {len(padding_data)} bytes of padding.")
        return padding_data




BUILD_DIR = "build"
PADDING = 0#2 * 1024 * 1024
OUT_FILE = "apps.img"

BUILD_DIR, OUT_FILE, PADDING = parse_args(BUILD_DIR, OUT_FILE, PADDING)



MAGIC_NUMBER = 0x41505053

ENTRY_LEN = 40
NAME_MAX_LEN = ENTRY_LEN - 8  # 32 bytes for name, 4 for offset, 4 for size

ENTRY_STRUCT = struct.Struct(f"<{NAME_MAX_LEN}sII")

APPS= sorted(discover_apps(BUILD_DIR, name_max_len=NAME_MAX_LEN))

print("Discovered apps:")
ind= 1
for name, path in APPS:
    print(f"{ind}: {name} -> {path}")
    ind+=1

print(f"Total apps: {len(APPS)}, add padding: {PADDING} bytes, total size: {sum(os.path.getsize(p) for _, p in APPS) + PADDING} bytes")

padding_data = load_existing_padding_if_valid(OUT_FILE);

with open(OUT_FILE, "wb") as f:

    header_pos = f.tell()

    # Placeholder header
    f.write(struct.pack("<II", MAGIC_NUMBER, len(APPS)))

    entries_pos = f.tell()

    # Placeholder entries
    for _ in APPS:
        f.write(b"\x00" * ENTRY_STRUCT.size)



    offsets = []
    for name, path in APPS:
        offsets.append(f.tell())
        with open(path, "rb") as app:
            while chunk := app.read(4096):
                f.write(chunk)

    end_pos = f.tell()

    # Now go back and write real entries
    f.seek(entries_pos)
    for i, (name, path) in enumerate(APPS):
        size = os.path.getsize(path)

        
        f.write(ENTRY_STRUCT.pack(name.encode(),
                            offsets[i],
                            size))

    f.seek(end_pos)

    if padding_data:
        f.write(padding_data)
        if len(padding_data) < PADDING:
            f.write(b"\x00" * (PADDING- len(padding_data)))
    else:
        f.write(b"\x00" * PADDING)
            