#pragma once
#include <stdint.h>
#include <stddef.h>
#include <iostream>

struct Metadata{
    uint64_t height = 0;
    uint64_t width = 0;
    size_t element_size = 0;
};