#ifndef YAZE_APP_UTILS_ROM_H
#define YAZE_APP_UTILS_ROM_H

#include <compressions/alttpcompression.h>
#include <rommapping.h>
#include <tile.h>

#include <cstddef>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "app/core/constants.h"
#include "app/gfx/tile.h"

namespace yaze {
namespace app {
namespace rom {

class ROM {
 public:
  void Close();

  void SetupRenderer(std::shared_ptr<SDL_Renderer> renderer);
  void LoadFromFile(const std::string& path);
  std::vector<tile8> ExtractTiles(gfx::TilePreset& preset);
  gfx::SNESPalette ExtractPalette(uint addr, int bpp);
  char* Decompress(int pos, int size = 0x800, bool reversed = false);
  uchar* SNES3bppTo8bppSheet(uchar* buffer_in, int sheet_id = 0, int size = 0x1000);
  SDL_Texture* DrawGraphicsSheet(int offset);

  int AddressFromBytes(uint8_t addr1, uint8_t addr2, uint8_t addr3) const;

  int GetPCGfxAddress(uint8_t id);
  char* CreateAllGfxDataRaw();
  void CreateAllGraphicsData(uchar* allGfx16Ptr);

  void LoadBlocksetGraphics(int graphics_id);

  unsigned int SnesToPc(unsigned int addr) const {
    if (addr >= 0x808000) {
      addr -= 0x808000;
    }
    unsigned int temp = (addr & 0x7FFF) + ((addr / 2) & 0xFF8000);
    return (temp + 0x0);
  }
  
  inline uchar* data() { return current_rom_; }
  inline auto Renderer() { return sdl_renderer_; }
  const uchar* getTitle() const { return title; }
  long int getSize() const { return size_; }
  char getVersion() const { return version_; }
  bool isLoaded() const { return loaded; }

 private:
  bool loaded = false;
  bool has_header_ = false;
  long int size_;
  uint compressed_size_;
  uchar* current_rom_;
  uchar version_;
  uchar title[21] = "ROM Not Loaded";
  enum rom_type type_ = LoROM;
  bool isbpp3[core::constants::NumberOfSheets];

  std::shared_ptr<uchar> rom_ptr_;
  std::vector<char*> decompressed_graphic_sheets_;
  std::vector<uchar*> converted_graphic_sheets_;
  std::vector<SDL_Surface> surfaces_;
  std::shared_ptr<SDL_Renderer> sdl_renderer_;
};

}  // namespace rom
}  // namespace app
}  // namespace yaze

#endif