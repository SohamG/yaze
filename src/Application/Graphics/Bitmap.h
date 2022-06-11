#ifndef YAZE_APPLICATION_UTILS_BITMAP_H
#define YAZE_APPLICATION_UTILS_BITMAP_H

#include <SDL2/SDL.h>
// #include <SDL2/SDL_opengl.h>

#include <memory>

#include "GL/glew.h"
#include "Utils/ROM.h"

namespace yaze {
namespace Application {
namespace Graphics {

using byte = unsigned char;

class Bitmap {
 public:
  Bitmap()=default;
  Bitmap(int width, int height, byte* data);

  void Create(GLuint* out_texture);
  int GetWidth();
  int GetHeight();

  bool LoadBitmapFromROM(unsigned char* texture_data, GLuint* out_texture,
                         int* out_width, int* out_height);

 private:
  int width_;
  int height_;
  byte* pixel_data_;
  SDL_PixelFormat pixel_format_;
};
}  // namespace Graphics
}  // namespace Application
}  // namespace yaze

#endif