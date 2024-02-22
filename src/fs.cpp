#include <fstream>

#include "fs.hpp"

unsigned long long operator""_MiB(unsigned long long size_mib)
{
    // return KiB
    return size_mib << 10;
}

tl::expected<monostate, std::string> init_fs(int fs_size, int block_size, std::string fs_name, int inode_ratio)
{
    std::ofstream ofile(fs_name, std::ios::binary);
    ofile.seekp(fs_size << 10 - 1);
    ofile.write("a", 1);

    // Check if block_size is power of two, Gemini wrote this
    // The right side of && is the important part
    // The - 1 flips all bits to the right of leading 1, while leading becomes 0
    // And the original number and that flipped number together and that will
    // always result in 0 for a power of 2
    if (!(block_size && (block_size & (block_size - 1))))
    {
        return tl::make_unexpected("Block size is not a power of 2");
    }

    int log2_size = 0;
    while (block_size > 1)
    {
        block_size >>= 1; // Divide by 2
        log2_size += 1;
    }

    // This will lead to a bit of wasted space, but it's easiest to just make
    // full sized blocks and ignore the little bit left over
    int num_blocks = fs_size / block_size;
    int num_inodes = fs_size / inode_ratio;
    int blocks_per_group = block_size * 8;
    int inodes_per_block = block_size / sizeof(Inode);
    int inodes_per_group = num_inodes / (num_blocks / blocks_per_group);
}
