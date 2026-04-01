#ifndef EXT2_H
#define EXT2_H

#include <stdint.h>

#define EXT2_MAGIC 0xEF53

/* Optional (compatible) feature flags */
#define EXT2_OPT_FEAT_PREALLOC_DIR  0x0001   /* Preallocate blocks for new directories */
#define EXT2_OPT_FEAT_AFS_INODES    0x0002   /* AFS server inodes exist */
#define EXT2_OPT_FEAT_JOURNAL       0x0004   /* File system has a journal (ext3) */
#define EXT2_OPT_FEAT_INODE_EXT     0x0008   /* Inodes have extended attributes */
#define EXT2_OPT_FEAT_RESIZE        0x0010   /* File system can resize itself */
#define EXT2_OPT_FEAT_HASH_DIR      0x0020   /* Directories use hash index */

/* Required (incompatible) feature flags */
#define EXT2_FEAT_COMPRESSION        0x0001   /* Compression is used */
#define EXT2_FEAT_DIR_TYPE_FIELD     0x0002   /* Directory entries contain a type field */
#define EXT2_FEAT_FS_REPLAY_JOURNAL  0x0004   /* File system needs to replay its journal */
#define EXT2_FEAT_FS_JOURNALED_DEV   0x0008   /* File system uses a journal device */

/* Read-only compatible feature flags */
#define EXT2_RO_FEAT_SPARSE_SB_GDT   0x0001   /* Sparse superblocks and group descriptor tables */
#define EXT2_RO_FEAT_FS_64BIT_FSIZE  0x0002   /* File system uses a 64-bit file size */
#define EXT2_RO_FEAT_BTREE_DIR       0x0004   /* Directory contents stored as a Binary Tree */

/* Inode type flags (top 4 bits of type_perm) */
#define EXT2_INODE_TYPE_FIFO      0x1000
#define EXT2_INODE_TYPE_CHAR_DEV  0x2000
#define EXT2_INODE_TYPE_DIR       0x4000
#define EXT2_INODE_TYPE_BLK_DEV   0x6000
#define EXT2_INODE_TYPE_FILE      0x8000
#define EXT2_INODE_TYPE_SYMLINK   0xA000
#define EXT2_INODE_TYPE_SOCKET    0xC000

/* Inode permission flags (bottom 12 bits of type_perm) */
#define EXT2_PERM_OTHER_EXEC   0x001
#define EXT2_PERM_OTHER_WRITE  0x002
#define EXT2_PERM_OTHER_READ   0x004
#define EXT2_PERM_GROUP_EXEC   0x008
#define EXT2_PERM_GROUP_WRITE  0x010
#define EXT2_PERM_GROUP_READ   0x020
#define EXT2_PERM_USER_EXEC    0x040
#define EXT2_PERM_USER_WRITE   0x080
#define EXT2_PERM_USER_READ    0x100
#define EXT2_PERM_STICKY       0x200
#define EXT2_PERM_SET_GID      0x400
#define EXT2_PERM_SET_UID      0x800

/* Inode flags */
#define EXT2_INODE_SECURE_DEL    0x00000001   /* Secure deletion (not used) */
#define EXT2_INODE_KEEP_COPY     0x00000002   /* Keep a copy of data when deleted (not used) */
#define EXT2_INODE_COMPRESSION   0x00000004   /* File compression (not used) */
#define EXT2_INODE_SYNC_UPDATE   0x00000008   /* Synchronous updates - new data written immediately to disk */
#define EXT2_INODE_IMMUTABLE     0x00000010   /* Immutable file - content cannot be changed */
#define EXT2_INODE_APPEND_ONLY   0x00000020   /* Append only */
#define EXT2_INODE_NO_DUMP       0x00000040   /* File is not included in 'dump' command */
#define EXT2_INODE_NO_ATIME      0x00000080   /* Last accessed time should not be updated */
/* 0x00000100 - 0x00008000: Reserved */
#define EXT2_INODE_HASH_DIR      0x00010000   /* Hash indexed directory */
#define EXT2_INODE_AFS_DIR       0x00020000   /* AFS directory */
#define EXT2_INODE_JOURNAL_DATA  0x00040000   /* Journal file data */

/* Directory entry type indicators */
#define EXT2_DIR_UNKNOWN_TYPE 0
#define EXT2_DIR_REG_FILE     1
#define EXT2_DIR_DIRECTORY    2
#define EXT2_DIR_CHAR_DEV     3
#define EXT2_DIR_BLOCK_DEV    4
#define EXT2_DIR_FIFO         5
#define EXT2_DIR_SOCKET       6
#define EXT2_DIR_SOFT_LINK    7

/* Creator OS IDs */
#define EXT2_FROM_LINUX   0
#define EXT2_FROM_HURD    1
#define EXT2_FROM_MASIX   2
#define EXT2_FROM_FREEBSD 3
#define EXT2_FROM_BSDLITE 4

struct ext2_super_block_t {
    uint32_t inodes_n;                 /* Total number of inodes */
    uint32_t blocks_n;                 /* Total number of blocks */
    uint32_t suser_block_n;            /* Blocks reserved for superuser */
    uint32_t unalloc_blocks_n;         /* Number of unallocated blocks */
    uint32_t unalloc_inodes_n;         /* Number of unallocated inodes */
    uint32_t starting_block_n;         /* Block number of superblock */
    uint32_t block_size;               /* log2(block size) - 10 */
    uint32_t frag_size;                /* log2(fragment size) - 10 */
    uint32_t n_blocks_per_block_group;
    uint32_t n_frags_per_block_group;
    uint32_t n_inodes_per_block_group;
    uint32_t last_mount;               /* POSIX time of last mount */
    uint32_t last_write;               /* POSIX time of last write */
    uint16_t n_mount_since_fsck;
    uint16_t n_mount_till_fsck;        /* Allowed mounts until fsck */
    uint16_t magic;                    /* Ext2 magic number: 0xEF53 */
    uint16_t state;                    /* 1: clean, 2: has errors */
    uint16_t err_flag;                 /* 1: ignore, 2: remount read-only, 3: kernel panic */
    uint16_t minor_version;
    uint32_t last_fsck;                /* POSIX time of last fsck */
    uint32_t i_forced_fsck;            /* Interval in seconds between forced fscks */
    uint32_t os_id;                    /* Operating system ID */
    uint32_t major_version;
    uint16_t uid;                      /* UID that can use reserved blocks */
    uint16_t gid;                      /* GID that can use reserved blocks */

    /* Extended fields: only valid if major_version >= 1 */
    uint32_t first_inode;            /* First non-reserved inode (< v1.0: fixed at 11) */
    uint16_t inode_size;             /* Size of inode structure in bytes (< v1.0: fixed at 128) */
    uint16_t block_group_nr;         /* Block group this superblock is part of (if backup) */
    uint32_t feature_compat;         /* Optional features (compatible) */
    uint32_t feature_incompat;       /* Required features (incompatible) */
    uint32_t feature_ro_compat;      /* Features requiring read-only mount if unsupported */
    uint8_t  uuid[16];               /* File system ID / volume UUID */
    char     volume_name[16];        /* Volume name (null-terminated C string) */
    char     last_mount_path[64];    /* Path last mounted to (null-terminated C string) */
    uint32_t compression_algs;       /* Compression algorithms used */
    uint8_t  prealloc_blocks_files;  /* Number of blocks to preallocate for files */
    uint8_t  prealloc_blocks_dirs;   /* Number of blocks to preallocate for directories */
    uint16_t unused;                 /* Unused */
    uint8_t  journal_uuid[16];       /* Journal ID (same format as UUID above) */
    uint32_t journal_inode;          /* Journal inode number */
    uint32_t journal_device;         /* Journal device number */
    uint32_t orphan_inode_head;      /* Head of orphan inode list */

    uint8_t  padding[788];           /* Pad to 1024 bytes */
} __attribute__((packed));

static_assert(sizeof(struct ext2_super_block_t) == 1024, "ext2 superblock needs to be 1024 bytes");

struct ext2_inode_t {
    uint16_t type_perm;          /* Type and permissions */
    uint16_t uid;                /* User ID */
    uint32_t size_low;           /* Lower 32 bits of size in bytes */
    uint32_t last_access;        /* POSIX time of last access */
    uint32_t creation_time;      /* POSIX time of creation */
    uint32_t last_mod;           /* POSIX time of last modification */
    uint32_t deletion_time;      /* POSIX time of deletion */
    uint16_t gid;                /* Group ID */
    uint16_t hard_link_count;    /* Number of hard links to this inode */
    uint32_t sector_count;       /* Disk sectors in use (not ext2 blocks) */
    uint32_t flags;              /* Inode flags */
    uint32_t os_val_1;           /* OS specific value #1 */
    uint32_t direct_ptr[12];     /* Direct block pointers 0-11 */
    uint32_t singly_indirect;    /* Singly indirect block pointer */
    uint32_t doubly_indirect;    /* Doubly indirect block pointer */
    uint32_t triply_indirect;    /* Triply indirect block pointer */
    uint32_t gen_number;         /* Generation number (used by NFS) */
    uint32_t ext_attr_block;     /* v0: reserved. v1+: extended attribute block (File ACL) */
    uint32_t size_high;          /* v0: reserved. v1+: upper 32 bits of size (file) or Dir ACL (dir) */
    uint32_t frag_block_addr;    /* Block address of fragment */
    uint8_t  os_val_2[12];       /* OS specific value #2 */
} __attribute__((packed));

static_assert(sizeof(struct ext2_inode_t) == 128, "ext2 inode needs to be 128 bytes");

struct ext2_block_group_t {
    uint32_t block_addr;         /* Block address of block usage bitmap */
    uint32_t inode_addr;         /* Block address of inode usage bitmap */
    uint32_t inode_table_addr;   /* Starting block address of inode table */
    uint16_t n_unalloc_blocks;   /* Number of unallocated blocks in group */
    uint16_t n_unalloc_inodes;   /* Number of unallocated inodes in group */
    uint16_t n_group_dirs;       /* Number of directories in group */
    uint8_t  padding[14];        /* Unused */
} __attribute__((packed));

static_assert(sizeof(struct ext2_block_group_t) == 32, "ext2 block group needs to be 32 bytes");

struct ext2_dir_entry_t {
    uint32_t inode;           /* Inode number of entry */
    uint16_t entry_size;      /* Total size of this entry */
    uint8_t  name_len;        /* Name length (least significant 8 bits) */
    uint8_t  type_indicator;  /* Type indicator (or high 8 bits of name_len if feature not set) */
    char     name[];          /* Name characters (flexible array member) */
} __attribute__((packed));

/* Validation */
#define EXT2_IS_VALID(sb) \
    ((sb)->magic == EXT2_MAGIC)

/* Constants */
#define EXT2_ROOT_INODE 2

/* Size calculations */
#define EXT2_BLOCK_SIZE(sb) \
    (1024 << (sb)->block_size)
#define EXT2_INODE_SIZE(sb) \
    ((sb)->major_version >= 1 ? (sb)->inode_size : 128)
#define EXT2_PTRS_PER_BLOCK(sb) \
    (EXT2_BLOCK_SIZE(sb) / sizeof(uint32_t))
#define EXT2_N_BLOCK_GROUPS(sb) \
    (((sb)->blocks_n + (sb)->n_blocks_per_block_group - 1) / (sb)->n_blocks_per_block_group)

/* Inode lookup */
#define EXT2_INODE_BLOCK_GROUP(inode, sb) \
    (((inode) - 1) / (sb)->n_inodes_per_block_group)
#define EXT2_INODE_INDEX(inode, sb) \
    (((inode) - 1) % (sb)->n_inodes_per_block_group)
#define EXT2_INODE_CONTAINING_BLOCK(inode, sb) \
    (EXT2_INODE_INDEX(inode, sb) * EXT2_INODE_SIZE(sb) / EXT2_BLOCK_SIZE(sb))
#define EXT2_INODE_BLOCK_OFFSET(inode, sb) \
    ((EXT2_INODE_INDEX(inode, sb) * EXT2_INODE_SIZE(sb)) % EXT2_BLOCK_SIZE(sb))

/* Inode type checks */
#define EXT2_INODE_IS_DIR(inode) \
    (((inode)->type_perm & 0xF000) == EXT2_INODE_TYPE_DIR)
#define EXT2_INODE_IS_FILE(inode) \
    (((inode)->type_perm & 0xF000) == EXT2_INODE_TYPE_FILE)
#define EXT2_INODE_IS_SYMLINK(inode) \
    (((inode)->type_perm & 0xF000) == EXT2_INODE_TYPE_SYMLINK)

/* Inode size */
#define EXT2_INODE_SIZE_FULL(inode) \
    (((uint64_t)(inode)->size_high << 32) | (inode)->size_low)

/* Maximum file sizes by indirection level */
#define EXT2_MAX_DIRECT_SIZE(sb) \
    (12 * EXT2_BLOCK_SIZE(sb))
#define EXT2_MAX_SINGLY_SIZE(sb) \
    (EXT2_MAX_DIRECT_SIZE(sb) + EXT2_PTRS_PER_BLOCK(sb) * EXT2_BLOCK_SIZE(sb))
#define EXT2_MAX_DOUBLY_SIZE(sb) \
    (EXT2_MAX_SINGLY_SIZE(sb) + EXT2_PTRS_PER_BLOCK(sb) * EXT2_PTRS_PER_BLOCK(sb) * EXT2_BLOCK_SIZE(sb))

/* Bitmap operations */
#define EXT2_BITMAP_IS_SET(bitmap, bit) \
    (((uint8_t *)(bitmap))[(bit) / 8] & (1 << ((bit) % 8)))
#define EXT2_BITMAP_SET(bitmap, bit) \
    (((uint8_t *)(bitmap))[(bit) / 8] |= (1 << ((bit) % 8)))
#define EXT2_BITMAP_CLEAR(bitmap, bit) \
    (((uint8_t *)(bitmap))[(bit) / 8] &= ~(1 << ((bit) % 8)))

/* Block allocation check */
#define EXT2_BLOCK_IS_ALLOC(block_addr) \
    ((block_addr) != 0)

#endif /* EXT2_H */