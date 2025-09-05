
#include <guikit.h>

int main() {
    printf( "[SubaAudioDevice] standalone demo\n" );
    PlatformWindow win(800,600,"gfxkit");
    Renderer renderer(win.nativeParent(),800,600);

    // Create a solid white texture
    // unsigned int texId = renderer.createSolidTexture(255,0,0,255);

    // simple
    // Widget widgets[] = {
    //     { renderer, "bmp00128.png", 100, 100 },
    //     { renderer, "bmp00147.png", 200, 400 },
    //     { renderer, "bmp00158.png", 300, 500 },
    // };

    // use the json widget layout loader
    std::vector<Widget*> widgets = loadGUI( renderer, "def.json" );

    AppEvents appEvents;
    win.pubsub.addListener(&appEvents);
    appEvents.addHandler(EventType::KeyDown, [&widgets, &renderer](const Event& e){
        if (e.character == 'r' && !e.keyRepeat) {
            printf( "reload\n" );
            widgets = loadGUI( renderer, "def.json" );
        }
    });

    while(appEvents.running) {
        win.poll();

        for (auto* w : widgets) {
            renderer.addQuad(w->quad, w->texId);
        }
        renderer.drawFrame();
    }
}
