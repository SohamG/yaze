#include <memory>

#include "Tile.h"
#include "Graphics/Bitmap.h"
#include "Utils/ROM.h"
#include "imgui/imgui.h"

namespace yaze {
namespace Application {
namespace Data {

using byte = unsigned char;
using ushort = unsigned short;

class OverworldMap {
 public:
  byte parent = 0;
  byte index = 0;
  byte gfx = 0;
  byte palette = 0;
  bool firstLoad = false;
  short messageID = 0;
  bool largeMap = false;

  byte sprgfx[3];
  byte sprpalette[3];
  byte musics[4];

  // 512 * 512
  std::unique_ptr<int> gfxPtr;
  std::unique_ptr<int> mapblockset16;
  // Bitmap gfxBitmap;  // Needs to be removed

  byte staticgfx[16];  // Need to be used to display map and not pre render it!
  ushort** tilesUsed;

  bool needRefresh = false;
  Utils::ROM rom_;

  OverworldMap(Utils::ROM rom, byte index);
  void BuildMap(byte* mapParent, int count, int gameState);
  void CopyTile8bpp16(int x, int y, int tile, int* destbmpPtr,
                      int* sourcebmpPtr);
  void CopyTile8bpp16From8(int xP, int yP, int tileID,
                           int* destbmpPtr,
                           int* sourcebmpPtr);

 private:
  void BuildTiles16Gfx(int count);
  // void ReloadPalettes() { LoadPalette(); }

  void CopyTile(int x, int y, int xx, int yy, int offset, TileInfo tile,
                byte* gfx16Pointer, byte* gfx8Pointer);

  void CopyTileToMap(int x, int y, int xx, int yy, int offset, TileInfo tile,
                     byte* gfx16Pointer, byte* gfx8Pointer);

  /* void LoadPalette();

  void SetColorsPalette(int index, ImVec4 main, ImVec4 animated, ImVec4 aux1,
                        ImVec4 aux2, ImVec4 hud, ImVec4 bgrcolor, ImVec4 spr,
                        ImVec4 spr2); */

  void BuildTileset(int gameState);
};

}  // namespace Data
}  // namespace Application
}  // namespace yaze