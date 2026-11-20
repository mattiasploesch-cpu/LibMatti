#!/usr/bin/env python3
"""Create a small content-defined delta between two game binaries.

The launcher understands the ``MATTICRAFTPATCH1`` format.  A release keeps the
full portable archive, but also publishes this delta so an installed launcher
can update the game executable, verify the result and start it immediately.

    python3 tools/make_binary_patch.py old/matticraft new/matticraft \\
        dist/Matticraft-0.1.0-to-0.2.0-linux-x86_64.patch \\
        --from-version 0.1.0 --to-version 0.2.0
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import random
import struct
import sys
import zlib
from datetime import datetime, timezone

MAGIC = b"MATTICRAFTPATCH1"
MIN_CHUNK = 4 * 1024
MAX_CHUNK = 128 * 1024
MASK = (1 << 14) - 1
GEAR = [random.Random(0x4D415454 + i).getrandbits(64) for i in range(256)]


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def chunks(data: bytes):
    start = 0
    rolling = 0
    for index, byte in enumerate(data):
        rolling = ((rolling << 1) + GEAR[byte]) & 0xFFFFFFFFFFFFFFFF
        size = index - start + 1
        if size >= MIN_CHUNK and ((rolling & MASK) == 0 or size >= MAX_CHUNK):
            yield start, data[start : index + 1]
            start = index + 1
            rolling = 0
    if start < len(data):
        yield start, data[start:]


def digest(chunk: bytes) -> bytes:
    return hashlib.blake2b(chunk, digest_size=16).digest()


def varint(value: int) -> bytes:
    out = bytearray()
    while True:
        byte = value & 0x7F
        value >>= 7
        if value:
            out.append(byte | 0x80)
        else:
            out.append(byte)
            return bytes(out)


def build(old: bytes, new: bytes) -> bytes:
    index: dict[bytes, int] = {}
    for offset, chunk in chunks(old):
        index.setdefault(digest(chunk), offset)

    body = bytearray()
    for _, chunk in chunks(new):
        where = index.get(digest(chunk))
        if where is not None:
            body.append(0x01)
            body += varint(where)
            body += varint(len(chunk))
        else:
            body.append(0x02)
            body += varint(len(chunk))
            body += chunk
    return bytes(body)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("old_binary")
    parser.add_argument("new_binary")
    parser.add_argument("output")
    parser.add_argument("--from-version", default="")
    parser.add_argument("--to-version", default="")
    parser.add_argument("--json-out", default="")
    args = parser.parse_args()

    old = open(args.old_binary, "rb").read()
    new = open(args.new_binary, "rb").read()
    if not old or not new:
        raise SystemExit("empty input binary")

    body = zlib.compress(build(old, new), 9)
    header = {
        "fromVersion": args.from_version,
        "toVersion": args.to_version,
        "fromSha256": sha256(old),
        "toSha256": sha256(new),
        "toSize": len(new),
        "createdAt": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
    }
    header_bytes = json.dumps(header, separators=(",", ":")).encode("utf-8")
    os.makedirs(os.path.dirname(os.path.abspath(args.output)) or ".", exist_ok=True)
    with open(args.output, "wb") as handle:
        handle.write(MAGIC)
        handle.write(struct.pack("<I", len(header_bytes)))
        handle.write(header_bytes)
        handle.write(body)

    stats = {
        "patch": args.output,
        "patchSize": os.path.getsize(args.output),
        "fromSize": len(old),
        "toSize": len(new),
        **header,
    }
    if args.json_out:
        with open(args.json_out, "w", encoding="utf-8") as handle:
            json.dump(stats, handle, indent=2)
            handle.write("\n")
    print(json.dumps(stats, indent=2))
    return 0


if __name__ == "__main__":
    sys.exit(main())
