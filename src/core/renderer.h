#pragma once
#include <memory>
#include <array>

#include <array>

struct Quad {
  Quad() {}
  Quad(float x, float y, float width, float height) { init( x, y, width, height ); }
  void init(float x, float y, float width, float height) {
      verts = { x, y,               // top-left
                x + width, y,       // top-right
                x, y + height,      // bottom-left
                x + width, y + height }; // bottom-right

      uvs = { 0.0f, 0.0f,          // top-left
              1.0f, 0.0f,          // top-right
              0.0f, 1.0f,          // bottom-left
              1.0f, 1.0f };        // bottom-right
  }

  std::array<float, 8> verts;   // x,y for 4 corners (screen space or NDC)
  std::array<float, 8> uvs;     // u,v for 4 corners
};


struct Impl;
struct NativeParent;

class Texture {
public:
    int width;
    int height;
    char* data;  // RGBA8, row-major, 4 bytes per pixel

    Texture() {}
    Texture(int w, int h, char* d) { init( w, h, d ); }
    void init(int w, int h, char* d) {
      width = w;
      height = h;
      data = d;
    }
};


class Renderer {
public:
    Renderer();
    Renderer(NativeParent& np, int width, int height);
    ~Renderer();

    void init(NativeParent& np, int width, int height);
    void resize(int width, int height);
    void addQuad(const Quad& quad, unsigned int textureId);
    void drawFrame();
    unsigned int createSolidTexture(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
    unsigned int createTexture(const Texture& tex);

private:
    std::unique_ptr<Impl> impl;
};
