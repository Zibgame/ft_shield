#!/usr/bin/env python3
"""Strip TLS callbacks from a PE32+ binary so MPRESS can pack it."""
import struct, sys

path = sys.argv[1]
with open(path, 'rb') as f:
    data = bytearray(f.read())

pe_off = struct.unpack_from('<I', data, 0x3C)[0]
opt_off = pe_off + 24
magic = struct.unpack_from('<H', data, opt_off)[0]
if magic != 0x20b:
    sys.exit('not PE32+')
dd_off = opt_off + 112
tls_rva, tls_size = struct.unpack_from('<II', data, dd_off + 9*8)
if tls_rva == 0:
    sys.exit('no TLS directory')
num_sec = struct.unpack_from('<H', data, pe_off + 6)[0]
sec_off = opt_off + struct.unpack_from('<H', data, pe_off + 20)[0]
def rva2off(rva):
    for i in range(num_sec):
        s = sec_off + i*40
        vsize, vaddr, rsize, roff = struct.unpack_from('<IIII', data, s+8)
        if vaddr <= rva < vaddr + max(vsize, rsize):
            return roff + (rva - vaddr)
    sys.exit('bad TLS rva')
tls_off = rva2off(tls_rva)
struct.pack_into('<Q', data, tls_off + 24, 0)  # AddressOfCallBacks = NULL
with open(path, 'wb') as f:
    f.write(data)
print(f'TLS callbacks nulled ({path})')
