#include <png.h>
#include "PlatformWindow_cocoa.h"
#include "renderer.h"

#include <fstream>
Texture loadPNG(const char* filename) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) throw std::runtime_error("Failed to open PNG");

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    png_infop info = png_create_info_struct(png);
    if (setjmp(png_jmpbuf(png))) throw std::runtime_error("PNG read error");

    png_init_io(png, fp);
    png_read_info(png, info);

    int width      = png_get_image_width(png, info);
    int height     = png_get_image_height(png, info);
    png_byte color_type = png_get_color_type(png, info);
    png_byte bit_depth  = png_get_bit_depth(png, info);

    // Convert to 8-bit RGBA if needed
    if (bit_depth == 16)
        png_set_strip_16(png);
    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png);
    if (png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);
    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY)
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

    png_read_update_info(png, info);

    std::vector<char> pixels(width * height * 4);
    std::vector<png_bytep> rows(height);
    for (int y = 0; y < height; y++)
        rows[y] = reinterpret_cast<png_bytep>(&pixels[y * width * 4]);

    png_read_image(png, rows.data());
    fclose(fp);
    png_destroy_read_struct(&png, &info, nullptr);

    char* data = new char[width*height*4];
    memcpy(data, pixels.data(), width*height*4);
    return Texture(width, height, data);
}

struct Widget {
    Widget( Renderer& renderer, std::string png, float x, float y ) { init( renderer, png, x, y ); }
    void init( Renderer& renderer, std::string png, float x, float y ) {
        tex = loadPNG(png.c_str());
        texId = renderer.createTexture(tex);
        quad.init( x, y, tex.width, tex.height );
    }
    Texture tex;
    unsigned int texId;
    Quad quad;
};

#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

std::vector<Widget*> widgets;

void loadGUI(Renderer& renderer, const std::string& filename) {
    std::ifstream f(filename);
    if (!f.is_open()) {
        printf("ERROR: could not open file '%s'\n", filename.c_str());
        return;
    }

    json j;
    try {
        f >> j;
    } catch (const nlohmann::json::parse_error& e) {
        // e.byte gives the position in the input where the error occurred
        std::string line_context;
        try {
            f.clear(); f.seekg(0);
            std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            size_t pos = e.byte;
            size_t start = (pos > 20) ? pos - 20 : 0;
            size_t end = std::min(content.size(), pos + 20);
            line_context = content.substr(start, end - start);
        } catch (...) {
            line_context = "(could not extract context)";
        }

        printf("JSON PARSE ERROR: %s\n at byte %zu, context: '%s'\n", e.what(), e.byte, line_context.c_str());
        exit(-1);
        return;
    } catch (const std::exception& e) {
        printf("UNKNOWN ERROR parsing JSON: %s\n", e.what());
        exit(-1);
        return;
    }

    for (size_t i = 0; i < j.at("controls").size(); ++i) {
        auto& ctrl = j.at("controls")[i];

        try {
            std::string type = ctrl.at("type");
            int x = ctrl.at("pos").at(0);
            int y = ctrl.at("pos").at(1);
            std::string texture = ctrl.at("texture"); // this might throw
            std::string label = ctrl.value("label", "");

            if (type == "background") {
                widgets.push_back(new Widget(renderer, texture, x, y));
            } else if (type == "knob") {
                widgets.push_back(new Widget(renderer, texture, x, y));
            } else if (type == "button") {
                widgets.push_back(new Widget(renderer, texture, x, y));
            } else if (type == "display") {
                widgets.push_back(new Widget(renderer, texture, x, y));
            } else if (type == "splash") {
                widgets.push_back(new Widget(renderer, texture, x, y));
            } else if (type == "kickButton") {
                widgets.push_back(new Widget(renderer, texture, x, y));
            } else {
                printf("ERROR: unknown widget type: %s\n", type.c_str());
                widgets.push_back(new Widget(renderer, texture, x, y));
            }
        } catch (const nlohmann::json::exception& e) {
            std::string typeInfo = ctrl.value("type", "unknown");
            std::string labelInfo = ctrl.value("label", "");
            printf("JSON ERROR processing widget #%zu (type='%s', label='%s'): %s\n",
                i, typeInfo.c_str(), labelInfo.c_str(), e.what());
        }
    }
}



int main() {
    PlatformWindow win(800,600,"gfxkit");
    Renderer renderer(win.nativeParent(),800,600);

    // Create a solid white texture
    // unsigned int texId = renderer.createSolidTexture(255,0,0,255);

    // Widget widgets[] = {
    //     { renderer, "bmp00128.png", 100, 100 },
    //     { renderer, "bmp00147.png", 200, 400 },
    //     { renderer, "bmp00158.png", 300, 500 },
    // };

    loadGUI( renderer, "def.json" );

    while(true) {
        win.poll();

        for (auto* w : widgets) {
            renderer.addQuad(w->quad, w->texId);
        }
        renderer.drawFrame();
    }
}



// int main(){
//   PlatformWindow win(1280, 720, "gfxkit");
//   VkInstance instance = createInstance(/* enable surface extensions per platform */);
//   VkSurfaceKHR surface = makeSurface(instance, win.nativeParent());

//   Renderer r;
//   r.init(instance, surface, win.size());

//   QuadBatch batch;
//   Texture tex = r.createTextureFromFile("assets/ui.png");

//   while(win.pump()){
//     r.beginFrame();

//     batch.begin();
//     batch.add({100,100, 256,256,  0,0,1,1, 0xFFFFFFFF, tex.view});
//     batch.flush(r);

//     r.endFrame();
//   }
// }
