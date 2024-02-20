#include <cstdint>

const int NUM_BLOCKS = 15;

enum class State : uint16_t
{
    OK,
    ERR,
};

struct Superblock
{
    uint32_t num_inodes;
    uint32_t num_blocks;
    uint32_t num_free_blocks;
    uint32_t num_free_inodes;
    uint32_t log_block_size;
    uint32_t blocks_per_group;
    uint32_t inodes_per_group;
    State state;
};

struct BlockGroupDescriptor
{
    uint32_t block_bitmap;
    uint32_t inode_bitmap;
    uint32_t inode_table;
    uint16_t free_blocks;
    uint16_t free_inodes;
    uint16_t num_dirs;
};

struct Inode
{
    uint16_t type;
    uint64_t size;
    uint16_t link_count;
    uint32_t block_ptrs[NUM_BLOCKS];
};