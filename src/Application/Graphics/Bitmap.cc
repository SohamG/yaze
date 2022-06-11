#include "Bitmap.h"

namespace yaze {
namespace Application {
namespace Graphics {

Bitmap::Bitmap(int width, int height, byte* data)
    : width_(width), height_(height), pixel_data_(data) {}

void Bitmap::Create(GLuint* out_texture) { 
  // // Read the pixel data from the ROM 
  // SDL_RWops * src = SDL_RWFromMem(pixel_data_, 0);
  // // Create the surface from that RW stream 
  // SDL_Surface* surface = SDL_LoadBMP_RW(src, SDL_FALSE);
  // GLenum mode = 0;
  // Uint8 bpp = surface->format->BytesPerPixel;
  // Uint32 rm = surface->format->Rmask;
  // if (bpp == 3 && rm == 0x000000ff) mode = GL_RGB;
  // if (bpp == 3 && rm == 0x00ff0000) mode = GL_BGR;
  // if (bpp == 4 && rm == 0x000000ff) mode = GL_RGBA;
  // if (bpp == 4 && rm == 0xff000000) mode = GL_BGRA;

  // GLsizei width = surface->w;
  // GLsizei height = surface->h;
  // GLenum format = mode;
  // GLvoid* pixels = surface->pixels;

  // Create a OpenGL texture identifier
  GLuint image_texture;
  glGenTextures(1, &image_texture);
  glBindTexture(GL_TEXTURE_2D, image_texture);

  // Setup filtering parameters for display
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, pixel_data_);

  *out_texture = image_texture;
}

int Bitmap::GetWidth() {
  return width_;
}
int Bitmap::GetHeight() {
  return height_;
}

// Simple helper function to load an image into a OpenGL texture with common
// settings
bool Bitmap::LoadBitmapFromROM(unsigned char* texture_data, GLuint* out_texture,
                               int* out_width, int* out_height) {
  // Load from file
  int image_width = 0;
  int image_height = 0;
  if (texture_data == NULL) return false;

  // Create a OpenGL texture identifier
  GLuint image_texture;
  glGenTextures(1, &image_texture);
  glBindTexture(GL_TEXTURE_2D, image_texture);

  // Setup filtering parameters for display
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                  GL_CLAMP_TO_EDGE);  // This is required on WebGL for non
                                      // power-of-two textures
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  // Same

  // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, texture_data);

  *out_texture = image_texture;
  *out_width = image_width;
  *out_height = image_height;

  return true;
}

}  // namespace Graphics
}  // namespace Application
}  // namespace yaze
