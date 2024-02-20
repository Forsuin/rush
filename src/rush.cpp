#define FMT_HEADER_ONLY

#include <iostream>
#include <fmt/core.h>

#include "fs.h"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fmt::println("Usage: rush [filename]");
        return 1;
    }
}