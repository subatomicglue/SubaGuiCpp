#include "window_cocoa.h"
#include "renderer.h"

int main() {
    PlatformWindow win(800,600,"gfxkit");
    Renderer renderer(win.nativeParent(),800,600);

    while(true) {
        win.poll();
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
