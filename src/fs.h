#include <cstdint>

/*

-------------------------------------------------------------
| Superblock | Block Group Descriptor Table | Block Group 0 |
-------------------------------------------------------------
|   1 Block  |          N Blocks            |    N Blocks   |
-------------------------------------------------------------


----------------------------------------------------------------
| Data Block Bitmap | Inode Bitmap | Inode Table | Data Blocks |
----------------------------------------------------------------
|      1 Block      |    1 Block   |   N Blocks  |   N Blocks  |
----------------------------------------------------------------

*/

// Number of blocks that a inode can point to
const int NUM_BLOCK_PTR = 15;

/*
    Filesystem state, 16 bits wide
*/
enum class State : uint16_t
{
    OK,
    ERR,
};

/*
    In a real ext2 system, the superblock is located 1024 bytes from
    the beginning of disk. This is to make room for the bootloader
    and the MBR, but since I don't have to worry about that this will
    be the first block of the FS
*/
struct Superblock
{
    uint32_t num_inodes;
    uint32_t num_blocks;
    uint32_t num_free_blocks;
    uint32_t num_free_inodes;
    // 1024 << log_block_size will give you the block size
    // in units of 1024 bytes
    uint32_t log_block_size;
    uint32_t blocks_per_group;
    uint32_t inodes_per_group;
    State state;
};

/*
    Right after the superblock a table of these will describe every block
    in the filesystem
*/
struct BlockGroupDescriptor
{
    uint32_t block_bitmap;
    uint32_t inode_bitmap;
    uint32_t inode_table;
    uint16_t free_blocks;
    uint16_t free_inodes;
    uint16_t num_dirs;
};

/*
    This is specific to rufs, ext2 doesn't always have aa filetype
    in a directory entry and ext2 uses a 16-bit short and uses
    that number for the type of dirEntry and for permissions
*/
enum class FileType : uint16_t
{
    Directory,
    Program,
    Text
};

/*
    Actually points the block containing the file
*/
struct Inode
{
    FileType type;
    uint64_t size;
    uint16_t link_count;
    uint32_t block_ptrs[NUM_BLOCK_PTR];
};

/*
    Describes the layout of a entry into a directory
*/
struct DirEntry
{
    uint32_t inode;
    uint16_t entry_size;
    FileType type;
    std::string name;
};