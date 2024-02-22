#define FMT_HEADER_ONLY

#include <cmath>

#include "fs.hpp"
#include "fmt/core.h"

void write_to_fs(std::string fs_name, const IFSWritable &writable, uint32_t block_addr)
{
    writable.write(fs_name, block_addr);
}

tl::expected<monostate, std::string> mkfs(int fs_size, int block_size, std::string fs_name, int inode_ratio)
{
    // convert from KiB to bytes
    fs_size = fs_size * 1024;

    std::ofstream ofile(fs_name, std::ios::binary);
    ofile.seekp((fs_size)-1);
    ofile.write("a", 1);

    if (block_size % 1024 != 0)
    {
        return tl::make_unexpected("Block size is not a power of 1024");
    }

    int mut_block_size = block_size;
    int log2_size = 0;
    while (mut_block_size > 1)
    {
        mut_block_size >>= 1; // Divide by 2
        log2_size += 1;
    }

    // This will lead to a bit of wasted space, but it's easiest to just make
    // full sized blocks and ignore the little bit left over
    int num_blocks = fs_size / block_size;
    int num_inodes = fs_size / inode_ratio;
    int blocks_per_group = block_size * 8;
    int num_groups = std::ceil((double)num_blocks / (double)blocks_per_group);
    int inodes_per_group = std::ceil((double)num_inodes / (double)num_groups);

    fmt::println("Num Blocks: {}, Num Inodes: {}, Blocks Per Group: {}, Inodes Per Group: {}",
                 num_blocks, num_inodes, blocks_per_group, inodes_per_group);

    Superblock sb;

    sb.num_blocks = num_blocks;
    sb.num_inodes = num_inodes;
    sb.num_free_blocks = num_blocks;
    sb.num_free_inodes = num_inodes;
    sb.log_block_size = log2_size;
    sb.blocks_per_group = blocks_per_group;
    sb.inodes_per_group = inodes_per_group;

    write_to_fs(fs_name, sb, 0);

    return monostate{};
}
