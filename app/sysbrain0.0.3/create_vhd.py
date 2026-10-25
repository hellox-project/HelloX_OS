#!/usr/bin/env python3
"""
Create a QEMU-runnable VHD for HelloX that includes:
  - bootsect + realinit + miniker + master kernel images
  - A FAT32 partition with files: hellox_app.exe, ssl/ca.pem, ssl/deepseek.key
"""

import struct
import os
import sys
import math

HELLOX_DIR = os.path.dirname(os.path.abspath(__file__))
BIN_DIR = os.path.join(os.path.dirname(os.path.dirname(HELLOX_DIR)), 
                       'v190', 'kernel', 'bin')

OUTPUT_VHD = os.path.join(HELLOX_DIR, 'hellox.vhd')

# Files from kernel bin directory
KERNEL_FILES = {
    'bootsect': ('bootsect', 0, 1),       # sector 0
    'realinit': ('realinit.bin', 2, 8),    # sectors 2-9 (4KB)
    'miniker':  ('miniker.bin', 10, 96),   # sectors 10-105 (48KB)
}

# Master.bin layout
MASTER_OFFSET_SECT = 264   # start sector for master part 1
MASTER_SECTOR_COUNT = None  # calculated from file size

def read_file_or_fallback(path):
    """Read a file, returning empty bytes if it doesn't exist."""
    if os.path.exists(path):
        with open(path, 'rb') as f:
            return f.read()
    return b''

def make_fat32_partition(total_sectors):
    """Create a minimal FAT32 partition starting at a given LBA.
    
    Returns (bytes, partition_size_sectors) for the FAT32 data area.
    For a VHD target ~64MB, we'll make a reasonable FAT32.
    """
    SECTOR_SIZE = 512
    
    # FAT32 parameters for a small but reasonable partition
    SECTORS_PER_CLUSTER = 1  # 512 bytes per cluster (minimal)
    RESERVED_SECTORS = 32    # enough space for boot sector + FSInfo
    FAT_COUNT = 2
    
    # Available sectors for data
    data_sectors_needed = total_sectors - RESERVED_SECTORS
    # Root directory is in cluster chain for FAT32, so no root dir sectors
    # FAT size: each FAT entry is 4 bytes, number of clusters determines FAT size
    # clusters = (data_sectors - fat_sectors * fat_count) / sectors_per_cluster
    # This is circular. We need to iterate.
    
    for fat_sectors in range(1, 256):
        clusters = (data_sectors_needed - (FAT_COUNT * fat_sectors)) // SECTORS_PER_CLUSTER
        if clusters <= 0:
            continue
        # Each FAT sector holds 512/4 = 128 entries
        needed_fat_sectors = math.ceil((clusters + 2) / 128)
        if needed_fat_sectors == fat_sectors:
            break
    
    # Calculate exact layout
    fat_start_lba = RESERVED_SECTORS
    root_cluster_start_lba = fat_start_lba + (FAT_COUNT * fat_sectors)
    total_fs_sectors = root_cluster_start_lba + (clusters * SECTORS_PER_CLUSTER)
    
    print(f"FAT32 layout:")
    print(f"  Reserved sectors: {RESERVED_SECTORS}")
    print(f"  FAT sectors: {fat_sectors} x {FAT_COUNT}")
    print(f"  Clusters: {clusters}")
    print(f"  Sectors per cluster: {SECTORS_PER_CLUSTER}")
    print(f"  Total FS sectors: {total_fs_sectors}")
    
    # Total size needed
    total_size = total_fs_sectors * SECTOR_SIZE
    
    # Create the file system region
    fs = bytearray(total_size)
    
    # Boot sector (DBR) - basic FAT32 header
    # BPB
    struct.pack_into('<H', fs, 11, SECTOR_SIZE)   # bytes per sector
    fs[13] = SECTORS_PER_CLUSTER
    struct.pack_into('<H', fs, 14, RESERVED_SECTORS)
    fs[16] = FAT_COUNT
    struct.pack_into('<H', fs, 17, 0)              # root entries (0 for FAT32)
    struct.pack_into('<H', fs, 19, 0)              # total sectors 16 (0 if > 65535)
    fs[21] = 0xF8                                  # media descriptor
    struct.pack_into('<H', fs, 22, 0)              # FAT size 16 (0 for FAT32)
    struct.pack_into('<H', fs, 24, 63)             # sectors per track
    struct.pack_into('<H', fs, 26, 255)            # heads
    struct.pack_into('<I', fs, 28, 0)              # hidden sectors
    struct.pack_into('<I', fs, 32, total_fs_sectors)  # total sectors 32
    struct.pack_into('<I', fs, 36, fat_sectors)    # FAT size 32
    struct.pack_into('<H', fs, 40, 0)              # ext flags
    struct.pack_into('<H', fs, 42, 0)              # FS version
    # Root dir cluster - usually cluster 2
    struct.pack_into('<I', fs, 44, 2)              # root cluster
    struct.pack_into('<H', fs, 48, 1)              # FSInfo sector
    struct.pack_into('<H', fs, 50, 6)              # backup boot sector
    fs[80:90] = b'FAT32   ' + b'\x20\x20\x20'
    
    # Boot code and signature
    fs[510] = 0x55
    fs[511] = 0xAA
    
    # FSInfo sector (sector 1)
    struct.pack_into('<I', fs, SECTOR_SIZE + 0, 0x41615252)  # lead signature
    struct.pack_into('<I', fs, SECTOR_SIZE + 484, 0x61417272)  # struct signature
    struct.pack_into('<I', fs, SECTOR_SIZE + 488, 2)  # free cluster count hint (cluster 2 used for root)
    struct.pack_into('<I', fs, SECTOR_SIZE + 492, 3)  # next free cluster hint
    struct.pack_into('<I', fs, SECTOR_SIZE + 508, 0xAA550000)  # trail signature
    
    # Initialize FAT tables
    def init_fat(fat_offset):
        """Initialize a FAT table at the given byte offset."""
        # Cluster 0: media descriptor padding
        struct.pack_into('<I', fs, fat_offset + 0, 0x0FFFFFF8)  # media + pad
        struct.pack_into('<I', fs, fat_offset + 4, 0x0FFFFFFF)  # EOC for cluster 1 (reserved)
        # Cluster 2 = root directory
        struct.pack_into('<I', fs, fat_offset + 8, 0x0FFFFFFF)  # EOC
    
    init_fat(fat_start_lba * SECTOR_SIZE)       # FAT1
    init_fat((fat_start_lba + fat_sectors) * SECTOR_SIZE)  # FAT2
    
    # Create root directory in cluster 2
    root_dir_offset = root_cluster_start_lba * SECTOR_SIZE
    
    def add_file_entry(dir_offset, filename_8_3, attr=0x20, 
                       first_cluster=0, file_size=0,
                       create_time=0x0000, create_date=0x4A6F,
                       access_date=0x4A6F):
        """Add a short filename entry to a directory."""
        # Filename must be 11 bytes (8+3), space-padded, uppercase
        if isinstance(filename_8_3, str):
            name = filename_8_3.upper().encode('ascii')
        name = name.ljust(11, b' ')
        
        # Use a dummy entry offset within the directory
        # We'll scan for first free entry
        for i in range(0, 16 * 32, 32):  # 16 entries max for simplicity
            pos = dir_offset + i
            if fs[pos] == 0 or fs[pos] == 0xE5:  # free or deleted
                # Write entry
                fs[pos:pos+11] = name
                fs[pos+11] = attr
                fs[pos+12] = 0  # reserved (NT)
                fs[pos+13] = 0  # creation time tenths
                struct.pack_into('<H', fs, pos+14, create_time)
                struct.pack_into('<H', fs, pos+16, create_date)
                struct.pack_into('<H', fs, pos+18, access_date)
                # high 16 bits of first cluster (FAT32)
                struct.pack_into('<H', fs, pos+20, (first_cluster >> 16) & 0xFFFF)
                # write time
                struct.pack_into('<H', fs, pos+22, create_time)
                struct.pack_into('<H', fs, pos+24, create_date)
                # low 16 bits of first cluster
                struct.pack_into('<H', fs, pos+26, first_cluster & 0xFFFF)
                struct.pack_into('<I', fs, pos+28, file_size)
                return pos
        return None  # directory full (unlikely for our use)
    
    root_dir_offset_val = root_cluster_start_lba * SECTOR_SIZE
    
    # We'll write files directly to clusters and track them
    # Cluster chain tracker
    next_free_cluster = 3
    
    def write_file_to_clusters(file_path, file_name_83, dir_offset):
        nonlocal next_free_cluster
        
        if not os.path.exists(file_path):
            print(f"  WARNING: {file_path} not found, skipping")
            return None
        
        file_data = read_file(file_path)
        if not file_data:
            print(f"  WARNING: {file_path} is empty, skipping")
            return None
        
        file_size = len(file_data)
        clusters_needed = (file_size + SECTOR_SIZE * SECTORS_PER_CLUSTER - 1) // (SECTOR_SIZE * SECTORS_PER_CLUSTER)
        
        if clusters_needed == 0:
            return None
        
        # Allocate clusters and write data
        first_cluster = next_free_cluster
        current_cluster = first_cluster
        
        data_start_lba = root_cluster_start_lba  # cluster 2 is root, 3+ is data
        
        data_offset = 0
        cluster_list = []
        
        for i in range(clusters_needed):
            cluster_offset = (data_start_lba + (current_cluster - 2) * SECTORS_PER_CLUSTER) * SECTOR_SIZE
            chunk = file_data[data_offset:data_offset + SECTOR_SIZE * SECTORS_PER_CLUSTER]
            fs[cluster_offset:cluster_offset + len(chunk)] = chunk
            data_offset += SECTOR_SIZE * SECTORS_PER_CLUSTER
            cluster_list.append(current_cluster)
            current_cluster += 1
        
        next_free_cluster = current_cluster
        
        # Update FAT tables for both copies
        for fat_idx in range(FAT_COUNT):
            fat_base = (fat_start_lba + fat_idx * fat_sectors) * SECTOR_SIZE
            for j, cl in enumerate(cluster_list):
                if j == len(cluster_list) - 1:
                    fat_entry_val = 0x0FFFFFFF  # EOC
                else:
                    fat_entry_val = cluster_list[j + 1]
                struct.pack_into('<I', fs, fat_base + cl * 4, fat_entry_val)
        
        # Add directory entry
        add_file_entry(dir_offset, file_name_83, 0x20, first_cluster, file_size)
        print(f"  Written {file_name_83}: cluster={first_cluster}, size={file_size}")
        return first_cluster
    
    # Add directories
    def add_directory_entry(parent_dir_offset, dir_name_83):
        nonlocal next_free_cluster
        
        cluster = next_free_cluster
        next_free_cluster += 1
        
        # Allocate cluster for directory
        dir_cluster_offset = (data_start_lba + (cluster - 2) * SECTORS_PER_CLUSTER) * SECTOR_SIZE
        
        # Mark in FAT
        for fat_idx in range(FAT_COUNT):
            fat_base = (fat_start_lba + fat_idx * fat_sectors) * SECTOR_SIZE
            struct.pack_into('<I', fs, fat_base + cluster * 4, 0x0FFFFFFF)
        
        # Initialize directory with . and ..
        # . entry
        dot_entry = bytearray(32)
        dot_entry[0:11] = b'.         '  # 8.3 name
        dot_entry[11] = 0x10  # directory attribute
        struct.pack_into('<H', dot_entry, 20, (cluster >> 16) & 0xFFFF)
        struct.pack_into('<H', dot_entry, 26, cluster & 0xFFFF)
        fs[dir_cluster_offset:dir_cluster_offset+32] = dot_entry
        
        # .. entry (points to parent, cluster 2 for root)
        dotdot_entry = bytearray(32)
        dotdot_entry[0:11] = b'..        '
        dotdot_entry[11] = 0x10
        # For now, point .. to root (cluster 2)
        struct.pack_into('<H', dotdot_entry, 20, 0)
        struct.pack_into('<H', dotdot_entry, 26, 2)
        fs[dir_cluster_offset+32:dir_cluster_offset+64] = dotdot_entry
        
        # Add entry in parent dir
        add_file_entry(parent_dir_offset, dir_name_83, 0x10, cluster, 0)
        print(f"  Created directory {dir_name_83}, cluster={cluster}")
        return cluster
    
    # Write files to root
    # hellox_app.exe
    write_file_to_clusters(
        os.path.join(HELLOX_DIR, 'hellox_app.exe'),
        'HELLOX_A.EXE',
        root_dir_offset_val
    )
    
    # Create SSL directory
    ssl_dir_cluster = add_directory_entry(root_dir_offset_val, 'SSL')
    ssl_dir_offset = (data_start_lba + (ssl_dir_cluster - 2) * SECTORS_PER_CLUSTER) * SECTOR_SIZE
    
    # Write ca.pem and deepseek.key into SSL directory
    write_file_to_clusters(
        os.path.join(HELLOX_DIR, 'ssl', 'ca.pem'),
        'CA     PEM',
        ssl_dir_offset
    )
    write_file_to_clusters(
        os.path.join(HELLOX_DIR, 'ssl', 'deepseek.key'),
        'DEEPSEE.KEY',
        ssl_dir_offset
    )
    
    return bytes(fs), total_fs_sectors


def build_vhd():
    print("=== Building HelloX VHD ===")
    
    # Read kernel files
    bootsect_data = read_file_or_fallback(os.path.join(BIN_DIR, '..', 'bootsect'))
    if not bootsect_data:
        # Fallback: read from VFLOPPY.VFD
        vfd_path = os.path.join(BIN_DIR, 'VFLOPPY.VFD')
        if os.path.exists(vfd_path):
            with open(vfd_path, 'rb') as f:
                vfd = f.read()
            bootsect_data = vfd[0:512]
            realinit_data = vfd[1024:5120]
            miniker_data = vfd[5120:54272]
            master_start = 264 * 512
            master_data = vfd[master_start:]
            print(f"  Extracted kernel from VFLOPPY.VFD")
        else:
            print("  ERROR: No kernel sources found!")
            sys.exit(1)
    else:
        realinit_data = open(os.path.join(BIN_DIR, 'realinit.bin'), 'rb').read()
        miniker_data = open(os.path.join(BIN_DIR, 'miniker.bin'), 'rb').read()
        master_data = open(os.path.join(BIN_DIR, 'master.bin'), 'rb').read()
    
    print(f"  bootsect: {len(bootsect_data)} bytes")
    print(f"  realinit: {len(realinit_data)} bytes")
    print(f"  miniker: {len(miniker_data)} bytes")
    print(f"  master: {len(master_data)} bytes")
    
    # Calculate VHD layout
    # VHD total size must be a whole number of MB (for CHS compatibility)
    # We need:
    #   Sector 0: bootsect
    #   Sectors 2-9: realinit (8 sectors)
    #   Sectors 10-105: miniker (96 sectors)
    #   Sectors 264: master starts
    #   After master: FAT32 partition
    MASTER_SECTORS = (len(master_data) + 511) // 512
    MASTER_END_SECT = MASTER_OFFSET_SECT + MASTER_SECTORS
    
    print(f"  master sectors: {MASTER_SECTORS}")
    print(f"  master ends at sector: {MASTER_END_SECT}")
    
    # Align partition start to 1MB boundary for VHD
    PARTITION_START = max(MASTER_END_SECT, 2048)  # at least 1MB
    # Align to MB boundary
    MB = 2048  # 1024*1024/512
    PARTITION_START = ((PARTITION_START + MB - 1) // MB) * MB
    
    # Total VHD size: 64MB minimum, 128MB good
    VHD_MB = 128
    VHD_SECTORS = VHD_MB * 2048  # 2048 sectors per MB
    PARTITION_SIZE = VHD_SECTORS - PARTITION_START
    
    print(f"  partition start LBA: {PARTITION_START}")
    print(f"  partition size: {PARTITION_SIZE} sectors = {PARTITION_SIZE * 512} bytes")
    
    # Build FAT32 partition data
    fs_data, fs_sectors = make_fat32_partition(PARTITION_SIZE)
    
    if fs_sectors > PARTITION_SIZE:
        print(f"  ERROR: FS needs {fs_sectors} sectors but only {PARTITION_SIZE} available")
        sys.exit(1)
    
    # Get partition data volume label from ASCII
    # Patch the DBR to set some sensible values
    struct.pack_into('<H', fs_data, 24, 63)    # sectors per track (for CHS)
    struct.pack_into('<H', fs_data, 26, 255)   # heads
    
    # Now build the full VHD
    vhd_data = bytearray(VHD_SECTORS * 512)
    
    # Write bootsect sector
    vhd_data[0:512] = bootsect_data[0:512]
    
    # Write realinit
    for i in range(8):
        vhd_data[(2 + i) * 512:(2 + i) * 512 + 512] = realinit_data[i*512:(i+1)*512]
    
    # Write miniker
    for i in range(96):
        vhd_data[(10 + i) * 512:(10 + i) * 512 + 512] = miniker_data[i*512:(i+1)*512]
    
    # Write master
    for i in range(MASTER_SECTORS):
        offset = (MASTER_OFFSET_SECT + i) * 512
        vhd_data[offset:offset+512] = master_data[i*512:(i+1)*512]
    
    # Write partition at start
    vhd_data[PARTITION_START * 512 : PARTITION_START * 512 + len(fs_data)] = fs_data[:]
    
    # Create MBR partition table in boot sector
    # We need a proper MBR with a partition entry pointing to the FAT32 partition
    # The bootsect from HelloX is NOT a standard MBR but we can overlay
    
    # Write MBR-compatible partition table at end of boot sector
    # Partition 1: FAT32 type, starting at PARTITION_START
    chs_start_s = (PARTITION_START % 63) + 1
    chs_start_h = 0
    chs_start_c = PARTITION_START // 63
    
    # Calculate CHS for end
    end_sector = PARTITION_START + PARTITION_SIZE - 1
    chs_end_s = (end_sector % 63) + 1
    chs_end_h = 0
    chs_end_c = end_sector // 63
    
    mbr_partition = struct.pack('<B', 0x80)  # bootable
    mbr_partition += struct.pack('<BBB', chs_start_h, chs_start_s, chs_start_c)
    mbr_partition += struct.pack('<B', 0x0B)  # FAT32 with CHS
    mbr_partition += struct.pack('<BBB', chs_end_h, chs_end_s, chs_end_c)
    mbr_partition += struct.pack('<I', PARTITION_START)  # LBA start
    mbr_partition += struct.pack('<I', PARTITION_SIZE)   # LBA size
    
    # Write partition entry at offset 0x1BE
    # But wait - HelloX bootsect is already at offset 0 and has code there
    # The partition entry at 0x1BE is at the standard MBR location
    # This should be after the boot code
    vhd_data[0x1BE:0x1CE] = mbr_partition
    
    # Keep the boot signature
    vhd_data[510] = 0x55
    vhd_data[511] = 0xAA
    
    print(f"\n  VHD size: {VHD_SECTORS} sectors = {VHD_MB}MB")
    print(f"  VHD sectors {VHD_SECTORS}")
    
    # Write to file (raw VHD)
    with open(OUTPUT_VHD, 'wb') as f:
        f.write(vhd_data)
    print(f"\n  VHD written to {OUTPUT_VHD}")
    
    # Also create a VHD footer for compatibility
    create_vhd_footer(OUTPUT_VHD, VHD_SECTORS, PARTITION_START)
    
    return OUTPUT_VHD


def create_vhd_footer(vhd_path, total_sectors, data_offset):
    """Add a VHD footer (fixed-size VHD format)."""
    VHD_FOOTER_SIZE = 512
    
    with open(vhd_path, 'ab') as f:
        footer = bytearray(VHD_FOOTER_SIZE)
        # Cookie
        footer[0:8] = b'conectix'
        # Features
        struct.pack_into('<I', footer, 8, 0x00000002)  # Temporary
        # File format version
        struct.pack_into('<I', footer, 12, 0x00010000)  # 1.0
        # Data offset (0xFFFFFFFF = no parent, fixed disk)
        struct.pack_into('<Q', footer, 16, 0xFFFFFFFFFFFFFFFF)
        # Time stamp (Unix seconds since 2000-01-01)
        struct.pack_into('<I', footer, 24, 0)
        # Creator Application
        footer[28:32] = b'vhdx'
        # Creator Version
        struct.pack_into('<I', footer, 32, 0x000A0000)
        # Creator Host OS (0 = Windows, 1 = Mac)
        struct.pack_into('<I', footer, 36, 0)
        # Original Size
        struct.pack_into('<Q', footer, 40, total_sectors * 512)
        # Current Size
        struct.pack_into('<Q', footer, 48, total_sectors * 512)
        # Disk Geometry
        cylinders = total_sectors // (255 * 63)
        struct.pack_into('<H', footer, 56, cylinders)
        footer[58] = 255  # heads
        footer[59] = 63   # sectors per track
        # Disk Type (2 = Fixed)
        struct.pack_into('<I', footer, 60, 2)
        # Checksum (one's complement of everything except checksum field)
        footer[64] = 0  # padding
        # UUID
        import uuid
        footer[68:84] = uuid.uuid4().bytes
        # Saved State
        footer[84] = 0
        # Calculate checksum (one's complement sum of all UINT32s)
        checksum = 0
        for i in range(0, VHD_FOOTER_SIZE, 4):
            checksum += struct.unpack_from('<I', footer, i)[0]
        checksum = (~checksum) & 0xFFFFFFFF
        struct.pack_into('<I', footer, 64, checksum)
        
        f.write(footer)
        print(f"  VHD footer written ({VHD_FOOTER_SIZE} bytes)")
        print(f"  Total VHD file size: {total_sectors * 512 + VHD_FOOTER_SIZE}")


def read_file(path):
    with open(path, 'rb') as f:
        return f.read()


if __name__ == '__main__':
    vhd = build_vhd()
    print(f"\nDone: {vhd}")
    print(f"Run with: qemu-system-x86_64 -drive file={vhd},format=vhd -m 256")
