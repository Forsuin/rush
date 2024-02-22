#define FMT_HEADER_ONLY

#include <iostream>
#include <fmt/core.h>

#include "fs.hpp"
#include "args.hpp"

int main(int argc, char **argv)
{
    std::string help = "Usage: rush [OPTION]...\n"
                       "OPTIONS:\n"
                       "\t-f, --filename=file\n"
                       "\t\tSpecifies the name of the file to use as the filesystem, if it doesn't end in .bin, the extension will be added. Defaults to fs.bin\n"
                       "\t-b, --block_size\n"
                       "\t\tSpecifies the size of each block in the filesystem. Defaults to 1024 bytes\n"
                       "\t-s, --fs_size\n"
                       "\t\tSets the total size of the filesystem, defaults to 1024 KiB\n"
                       "\t-i, --inode_ratio\n"
                       "\t\tSets the ratio of bytes per inode, defaults to being 1024 bytes / inode";

    args::ArgParser parser;
    parser.helptext = help;
    parser.option("filename f", "fs.bin");
    parser.option("block_size b", "1024");
    parser.option("fs_size s", "1024");
    parser.option("inode_ratio i", "1024");

    parser.parse(argc, argv);

    std::string fs_name;
    int block_size;
    int fs_size;
    int inode_ratio;

    fs_name = parser.value("filename");

    if (fs_name.rfind(".bin") == std::string::npos)
    {
        fs_name += ".bin";
    }

    block_size = std::stoi(parser.value("block_size"));
    fs_size = std::stoi(parser.value("fs_size"));
    inode_ratio = std::stoi(parser.value("inode_ratio"));

    mkfs(fs_size, block_size, fs_name, inode_ratio)
        .map_error([&](std::string s)
                   { fmt::println("{}", s); });
}