
struct QuadVertex { float x,y; float u,v; uint32_t colorABGR; };

struct Quad { float x,y,w,h; float u0,v0,u1,v1; uint32_t color; VkImageView tex; };
class QuadBatch {
  void begin();
  void add(const Quad& q);
  void flush(Renderer& r); // records draw calls, texture binds grouped
};

