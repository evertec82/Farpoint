#include <cstdio>
#include <cstdlib>
#include "video_core/amdgpu/resource.h"
#include "video_core/texture_cache/image_info.h"
using VideoCore::ImageInfo;
static void check(bool ok, const char* name) { if (!ok) { std::fprintf(stderr, "FAIL: %s\n", name); std::exit(1); } }
int main() {
    ImageInfo parent{};
    parent.type = AmdGpu::ImageType::Color2D;
    parent.pixel_format = vk::Format::eR16G16B16A16Sfloat;
    parent.num_bits = 64;
    parent.size = {128, 128, 1}; parent.pitch = 128;
    parent.resources = {8, 8}; parent.guest_address = 0x100000;
    parent.array_mode = AmdGpu::ArrayMode::Array1DTiledThin1;
    unsigned offset = 0;
    for (unsigned mip=0; mip<8; ++mip) {
        unsigned dim = 128u >> mip, pitch = dim < 8 ? 8 : dim;
        unsigned bytes = pitch * pitch * 8 * 8;
        parent.mips_layout[mip] = {bytes, pitch, pitch, offset}; offset += bytes;
    }
    parent.guest_size = offset;
    auto child = [&](unsigned mip, unsigned layer, unsigned count) {
        ImageInfo c = parent;
        c.resources = {1, count};
        c.size = {128u >> mip, 128u >> mip, 1};
        c.pitch = parent.mips_layout[mip].pitch;
        auto slice = parent.mips_layout[mip].size / 8;
        c.guest_address += parent.mips_layout[mip].offset + slice * layer;
        c.guest_size = slice * count;
        return c;
    };
    for (unsigned mip=0; mip<8; ++mip) {
        for (unsigned layer=0; layer<8; ++layer) {
            auto c=child(mip, layer, 1);
            check(c.MipOf(parent)==mip, "single face mip including padded mips");
            check(c.SliceOf(parent,mip)==layer, "single face layer");
        }
    }
    auto pair=child(1, 2, 2);
    check(pair.SliceOf(parent,1)==2, "two-layer view begins at layer 2, not layer 1");
    pair=child(1,1,2);
    check(pair.SliceOf(parent,1)==1, "multi-layer view need not align to its total size");
    auto invalid=child(1,7,2);
    check(invalid.SliceOf(parent,1)==-1, "reject range crossing end of mip");
    invalid=child(1,1,1); ++invalid.guest_address;
    check(invalid.SliceOf(parent,1)==-1, "reject misaligned address");
    check(invalid.SliceOf(parent,-1)==-1, "reject invalid mip");
    std::puts("Image subresource tests passed (all cube faces/mips, multi-layer aliases, bounds)");
}
