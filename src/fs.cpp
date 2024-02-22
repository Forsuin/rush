#define FMT_HEADER_ONLY

#include <fstream>
#include <cmath>

#include "fs.hpp"
#include "fmt/core.h"

unsigned long long operator""_MiB(unsigned long long size_mib)
{
    // return KiB
    return size_mib << 10;
}

tl::expected<monostate, std::string> mkfs(int fs_size, int block_size, std::string fs_name, int inode_ratio)
{
    // convert from KiB to bytes
    fs_size = fs_size * 1024;

    std::ofstream ofile(fs_name, std::ios::binary);
    ofile.seekp((fs_size << 10) - 1);
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

    return monostate{};
}
