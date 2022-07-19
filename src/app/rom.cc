#include "rom.h"

#include <compressions/alttpcompression.h>
#include <rommapping.h>

#include <cstddef>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "app/core/common.h"
#include "app/core/constants.h"
#include "app/gfx/snes_tile.h"

namespace yaze {
namespace app {

void ROM::Close() {
  if (is_loaded_) {
    delete[] current_rom_;
    for (auto i = 0; i < num_sheets_; i++) {
      delete[] decompressed_graphic_sheets_[i];
      delete[] converted_graphic_sheets_[i];
    }
  }
}

void ROM::SetupRenderer(std::shared_ptr<SDL_Renderer> renderer) {
  sdl_renderer_ = renderer;
}

// TODO: check if the rom has a header on load
void ROM::LoadFromFile(const std::string &path) {
  std::ifstream file(path.c_str(), std::ios::binary);
  if (!file.is_open()) {
    std::cout << "Error: Could not open ROM file " << path << std::endl;
    return;
  }
  size_ = std::filesystem::file_size(path.c_str());
  current_rom_ = new uchar[size_];
  for (uint i = 0; i < size_; i++) {
    char byte_read_ = ' ';
    file.read(&byte_read_, sizeof(char));
    current_rom_[i] = byte_read_;
  }
  file.close();
  SDL_memcpy(title, current_rom_ + 32704, 20);
  is_loaded_ = true;
}

void ROM::LoadFromPointer(uchar *data) { current_rom_ = data; }

// 0-112 -> compressed 3bpp bgr -> (decompressed each) 0x600 chars
// 113-114 -> compressed 2bpp -> (decompressed each) 0x800 chars
// 115-126 -> uncompressed 3bpp sprites -> (each) 0x600 chars
// 127-217 -> compressed 3bpp sprites -> (decompressed each) 0x600 chars
// 218-222 -> compressed 2bpp -> (decompressed each) 0x800 chars
void ROM::LoadAllGraphicsData() {
  auto buffer = new uchar[346624];
  auto data = new uchar[2048];
  int buffer_pos = 0;

  for (int i = 0; i < core::constants::NumberOfSheets; i++) {
    // uncompressed sheets
    if (i >= 115 && i <= 126) {
      data = new uchar[core::constants::Uncompressed3BPPSize];
      int startAddress = GetGraphicsAddress(i);
      for (int j = 0; j < core::constants::Uncompressed3BPPSize; j++) {
        data[j] = current_rom_[j + startAddress];
      }
    } else {
      auto gfx_addr = GetGraphicsAddress(i);
      data = Decompress(gfx_addr, core::constants::UncompressedSheetSize);
    }

    gfx::Bitmap tilesheet_bmp(
        core::constants::kTilesheetWidth, core::constants::kTilesheetHeight,
        core::constants::kTilesheetDepth, SNES3bppTo8bppSheet(data));
    tilesheet_bmp.CreateTexture(sdl_renderer_);
    graphics_bin_[i] = tilesheet_bmp;

    for (int j = 0; j < sizeof(data); j++) {
      buffer[j + buffer_pos] = data[j];
    }

    buffer_pos += sizeof(data);
  }

  master_gfx_bin_ = buffer;
}

uchar *ROM::DecompressGraphics(int pos, int size) {
  return Decompress(pos, size, false);
}

uchar *ROM::DecompressOverworld(int pos, int size) {
  return Decompress(pos, size, true);
}

uchar *ROM::Decompress(int pos, int size, bool reversed) {
  auto buffer = new uchar[size];
  uint length = 0;
  uint buffer_pos = 0;
  uchar cmd = 0;

  uchar databyte = current_rom_[pos];
  while (databyte != 0xFF) {  // End of decompression
    databyte = current_rom_[pos];

    // Expanded Command
    if ((databyte & 0xE0) == 0xE0) {
      cmd = (uchar)((databyte >> 2) & 0x07);
      length =
          (ushort)(((current_rom_[pos] << 8) | current_rom_[pos + 1]) & 0x3FF);
      pos += 2;  // Advance 2 bytes in ROM
    } else {     // Normal Command
      cmd = (uchar)((databyte >> 5) & 0x07);
      length = (uchar)(databyte & 0x1F);
      pos += 1;  // Advance 1 byte in ROM
    }
    length += 1;  // each commands is at least of size 1 even if index 00

    switch (cmd) {
      case kCommandDirectCopy:
        for (int i = 0; i < length; i++) {
          buffer[buffer_pos++] = current_rom_[pos++];
        }
        // Do not advance in the ROM
        break;
      case kCommandByteFill:
        for (int i = 0; i < length; i++) {
          buffer[buffer_pos++] = current_rom_[pos];
        }
        pos += 1;  // Advance 1 byte in the ROM
        break;
      case kCommandWordFill:
        for (int i = 0; i < length; i += 2) {
          buffer[buffer_pos++] = current_rom_[pos];
          buffer[buffer_pos++] = current_rom_[pos + 1];
        }
        pos += 2;  // Advance 2 byte in the ROM
        break;
      case kCommandIncreasingFill: {
        uchar inc_byte = current_rom_[pos];
        for (int i = 0; i < length; i++) {
          buffer[buffer_pos++] = inc_byte++;
        }
        pos += 1;  // Advance 1 byte in the ROM
      } break;
      case kCommandRepeatingBytes: {
        ushort s1 = ((current_rom_[pos + 1] & 0xFF) << 8);
        ushort s2 = ((current_rom_[pos] & 0xFF));
        // Reversed byte order for overworld maps
        if (reversed) {
          auto addr = (current_rom_[pos + 2]) | ((current_rom_[pos + 1]) << 8);
          if (addr > buffer_pos) {
            std::cout << "size error" << std::endl;
          }

          if (buffer_pos + length >= size) {
            size *= 2;
            buffer = new uchar[size];
            std::cout << "Reallocate buffer" << std::endl;
          }
          memcpy(buffer + buffer_pos, current_rom_ + pos, length);
          pos += 2;
        } else {
          auto addr = (ushort)(s1 | s2);
          for (int i = 0; i < length; i++) {
            buffer[buffer_pos] = buffer[addr + i];
            buffer_pos++;
          }
          pos += 2;  // Advance 2 bytes in the ROM
        }
      } break;
    }
  }
  num_sheets_++;
  decompressed_graphic_sheets_.push_back(buffer);
  return buffer;
}

// 128x32
uchar *ROM::SNES3bppTo8bppSheet(uchar *buffer_in, int sheet_id, int size) {
  // 8bpp sheet out
  auto sheet_buffer_out = new uchar[size];
  int xx = 0;  // positions where we are at on the sheet
  int yy = 0;
  int pos = 0;
  int ypos = 0;

  if (sheet_id != 0) {
    yy = sheet_id;
  }

  // for each tiles
  // 16 per line
  for (int i = 0; i < 64; i++) {
    // for each line
    for (int y = 0; y < 8; y++) {
      //[0] + [1] + [16]
      for (int x = 0; x < 8; x++) {
        auto b1 = ((buffer_in[(y * 2) + (24 * pos)] & (kGraphicsBitmap[x])));
        auto b2 =
            (buffer_in[((y * 2) + (24 * pos)) + 1] & (kGraphicsBitmap[x]));
        auto b3 = (buffer_in[(16 + y) + (24 * pos)] & (kGraphicsBitmap[x]));
        unsigned char b = 0;
        if (b1 != 0) {
          b |= 1;
        }
        if (b2 != 0) {
          b |= 2;
        }
        if (b3 != 0) {
          b |= 4;
        }
        sheet_buffer_out[x + (xx) + (y * 128) + (yy * 1024)] = b;
      }
    }
    pos++;
    ypos++;
    xx += 8;
    if (ypos >= 16) {
      yy++;
      xx = 0;
      ypos = 0;
    }
  }
  converted_graphic_sheets_.push_back(sheet_buffer_out);
  return sheet_buffer_out;
}

uint ROM::GetGraphicsAddress(uint8_t offset) const {
  uint snes_address = 0;
  uint pc_address = 0;
  snes_address = (uint)((((current_rom_[0x4F80 + offset]) << 16) |
                         ((current_rom_[0x505F + offset]) << 8) |
                         ((current_rom_[0x513E + offset]))));
  pc_address = core::SnesToPc(snes_address);
  return pc_address;
}

SDL_Texture *ROM::DrawGraphicsSheet(int offset) {
  SDL_Surface *surface =
      SDL_CreateRGBSurfaceWithFormat(0, 128, 32, 8, SDL_PIXELFORMAT_INDEX8);
  std::cout << "Drawing surface #" << offset << std::endl;
  uchar *sheet_buffer = nullptr;
  for (int i = 0; i < 8; i++) {
    surface->format->palette->colors[i].r = i * 31;
    surface->format->palette->colors[i].g = i * 31;
    surface->format->palette->colors[i].b = i * 31;
  }

  uint graphics_address = GetGraphicsAddress(offset);
  std::cout << "Decompressing..." << std::endl;
  auto decomp = Decompress(graphics_address);
  std::cout << "Converting to 8bpp sheet..." << std::endl;
  sheet_buffer = SNES3bppTo8bppSheet(decomp);
  std::cout << "Assigning pixel data..." << std::endl;
  surface->pixels = sheet_buffer;
  std::cout << "Creating texture from surface..." << std::endl;
  SDL_Texture *sheet_texture = nullptr;
  sheet_texture = SDL_CreateTextureFromSurface(sdl_renderer_.get(), surface);
  if (sheet_texture == nullptr) {
    std::cout << "Error: " << SDL_GetError() << std::endl;
  }
  return sheet_texture;
}

gfx::SNESPalette ROM::ExtractPalette(uint addr, int bpp) {
  uint filePos = addr;
  uint palette_size = pow(2, bpp);
  auto palette_data = (char *)SDL_malloc(sizeof(char) * (palette_size * 2));
  memcpy(palette_data, current_rom_ + filePos, palette_size * 2);
  return gfx::SNESPalette(palette_data);
}

}  // namespace app
}  // namespace yaze