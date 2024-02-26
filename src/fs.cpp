#define FMT_HEADER_ONLY

#include <cmath>
#include <vector>

#include "fs.hpp"
#include "fmt/core.h"

// Have this helper that just calls the writable's write function since I don't want to
// have to write it myself multiple times
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
    ofile.write("\0", 1);

    if (block_size % 1024 != 0)
    {
        return tl::make_unexpected("Block size is not a power of 1024");
    }

    int mut_block_size = block_size;
    int log2_size = 0;
    while (mut_block_size > 1024)
    {
        mut_block_size >>= 1; // Divide by 2
        log2_size += 1;
    }

    // This will lead to a bit of wasted space, but it's easiest to just make
    // full sized blocks and ignore the little bit left over
    int num_blocks = fs_size / block_size;
    int num_inodes = (num_blocks * block_size) / inode_ratio;
    int blocks_per_group = block_size * 8;
    int num_groups = std::ceil((double)num_blocks / (double)blocks_per_group);
    int inodes_per_group = std::ceil((double)num_inodes / (double)num_groups);

    // fmt::println("Log Block Size: {}, Num Blocks: {}, Num Inodes: {}, Blocks Per Group: {}, Inodes Per Group: {}",
    //              log2_size, num_blocks, num_inodes, blocks_per_group, inodes_per_group);

    Superblock sb;

    sb.num_blocks = num_blocks;
    sb.num_inodes = num_inodes;
    sb.num_free_blocks = num_blocks;
    sb.num_free_inodes = num_inodes;
    sb.log_block_size = log2_size;
    sb.blocks_per_group = blocks_per_group;
    sb.inodes_per_group = inodes_per_group;
    sb.blocks_reserved = 1; // reserve superblock

    // ===========Block Group Descriptor Table===================

    write_to_fs(fs_name, sb, 0);

    int gdt_blocks = (num_groups * sizeof(BlockGroupDescriptor)) / block_size;
    // acount for any partial block needed
    if ((num_groups * sizeof(BlockGroupDescriptor)) % block_size)
        gdt_blocks++;

    sb.blocks_reserved += gdt_blocks;

    std::ofstream(fs_name, std::ios::binary | std::ios::app);

    // skip over superblock and the blocks reserved for table
    int first_free_block = sb.blocks_reserved + gdt_blocks;

    std::vector<BlockGroupDescriptor> descriptors;
    std::vector<Inode> inodes;

    for (int i = 0; i < gdt_blocks; i++)
    {
        BlockGroupDescriptor bgd;
        // byte address of start of group
        int group_start = block_size * (i + 1);

        ofile.seekp(group_start + (i * 32), std::ios::beg);

        bgd.block_bitmap_addr = first_free_block;
        bgd.inode_bitmap_addr = first_free_block + 1;
        bgd.inode_table = first_free_block + 2;
        bgd.num_dirs = 0;
        bgd.free_blocks = blocks_per_group - 3;
        bgd.free_inodes = inodes_per_group;

        // fmt::println("group_start: {}, seek_pos: {}, bitmap_addr: {}, inode_addr: {}, inode_table: {}, num_dirs: {}, free_blocks: {}, free_inodes: {}",
        //              group_start, group_start + (i * 32), bgd.block_bitmap_addr, bgd.inode_bitmap_addr, bgd.inode_table, bgd.num_dirs, bgd.free_blocks, bgd.free_inodes);

        WRITE(ofile, bgd.block_bitmap_addr);
        WRITE(ofile, bgd.inode_bitmap_addr);
        WRITE(ofile, bgd.inode_table);
        WRITE(ofile, bgd.num_dirs);
        WRITE(ofile, bgd.free_blocks);
        WRITE(ofile, bgd.free_inodes);
        WRITE(ofile, bgd._pad);

        descriptors.push_back(bgd);

        // write inodes for this group
        int addr = bgd.inode_table * block_size;
        ofile.seekp(addr, std::ios::beg);

        for (int i = 0; i < inodes_per_group; i++)
        {
            Inode inode;

            WRITE(ofile, inode.type);
            WRITE(ofile, inode.size);
            WRITE(ofile, inode.link_count);
            WRITE(ofile, inode.block_ptrs);
            WRITE(ofile, inode._pad);

            inodes.push_back(inode);
        }
    }

    // ============ Root Directory =============

    BlockGroupDescriptor &first_group = descriptors[0];
    int inode_table = first_group.inode_table * block_size;

    Inode

        return monostate{};
}

bool is_dir(Inode &inode)
{
    return inode.type == FileType::Directory;
}

uint32_t find_block(uint32_t inode_addr, Superblock &sb)
{
    uint32_t group = (inode_addr - 1) / sb.inodes_per_group;
    uint32_t index = (inode_addr - 1) % sb.inodes_per_group;
    return index * sizeof(Inode) / (1024 << sb.log_block_size);
}