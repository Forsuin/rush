

#include <cstdint>
#include <string>
#include <fstream>

#include "expected.hpp"
#include "optional.hpp"
#include "monostate.hpp"

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

#define BYTE_INFO(x) (char *)&x, sizeof(x)
#define WRITE(ofile, x) ofile.write(BYTE_INFO(x))

struct IFSWritable
{
    virtual void write(std::string fs_name, uint32_t block_addr) const = 0;
};

/*
    In a real ext2 system, the superblock is located 1024 bytes from
    the beginning of disk. This is to make room for the bootloader
    and the MBR, but since I don't have to worry about that this will
    be the first block of the FS
*/
struct Superblock : public IFSWritable
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
    uint32_t blocks_reserved;

    void write(std::string fs_name, uint32_t block_addr) const
    {
        std::ofstream ofile(fs_name, std::ios::binary | std::ios::app);
        ofile.seekp((block_addr * (1024 << log_block_size)));
        WRITE(ofile, num_inodes);
        WRITE(ofile, num_blocks);
        WRITE(ofile, num_free_blocks);
        WRITE(ofile, num_free_inodes);

        WRITE(ofile, log_block_size);
        WRITE(ofile, blocks_per_group);
        WRITE(ofile, inodes_per_group);
        WRITE(ofile, blocks_reserved);

        ofile.close();
    }
};

/*
    Right after the superblock a table of these will describe every block
    in the filesystem. Size of 32 bytes so we can pack multiple descriptors
    into a single block
*/
struct BlockGroupDescriptor
{
    uint32_t block_bitmap_addr;
    uint32_t inode_bitmap_addr;
    uint32_t inode_table;
    uint16_t free_blocks;
    uint16_t free_inodes;
    uint16_t num_dirs;
    char _pad[12] = {0};
};

/*
    This is specific to rufs, ext2 doesn't always have aa filetype
    in a directory entry and ext2 uses a 16-bit short and uses
    that number for the type of dirEntry and for permissions
*/
enum class FileType : uint16_t
{
    Unused = 0,
    Directory = 0x4000,
    Program,
    Text
};

/*
    Actually points the block containing the file
*/
struct Inode
{
    FileType type = FileType::Unused;
    uint64_t size = 0;
    uint16_t link_count = 0;
    uint32_t block_ptrs[NUM_BLOCK_PTR] = {0};
    char _pad[42] = {0};
};

/*
    Describes the layout of a entry into a directory
*/
struct DirEntry
{
    uint32_t inode;
    uint16_t entry_size;
    FileType type;
    char name[11];
};

/*
    Creates and initializes filesystem to a default state with root directory created

    Creates blocks, block groups, inodes, superblock, and descriptor table

    fs_size is the total size of the filesystem given in KiB
    block_size is the size of each block, is a power of 2 (1024, 2048, 4096, etc)

    fs_size defaults to 1024 KiB
    block_size defaults to 1024 bytes
    inode_ratio defaults to 1024 bytes / inode as most of these files should be failry small
*/
tl::expected<monostate, std::string> mkfs(int fs_size, int block_size, std::string fs_name, int inode_ratio);

bool is_dir(Inode &inode);

/*
    Returns the address of an block containing an inode address
*/
uint32_t find_block(uint32_t inode_addr, Superblock &sb);