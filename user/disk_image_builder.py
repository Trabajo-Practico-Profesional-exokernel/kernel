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

    for entry in os.scandir(build_dir):
        if entry.is_dir():
            app_bin_path = os.path.join(entry.path, "app.bin")
            if os.path.isfile(app_bin_path):
                name_bytes = entry.name.encode()
                if len(name_bytes) >= name_max_len:
                    raise ValueError(f"App name '{entry.name}' too long")

                apps.append((entry.name, app_bin_path))
    return apps






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
    f.write(b"\x00" * PADDING)