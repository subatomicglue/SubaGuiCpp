#pragma once
#include <vector>
#include <map>

enum class EventType {
    Quit,
    Resize,
    MouseDown,
    MouseUp,
    MouseMove,
    KeyDown,
    KeyUp,
    Unknown
};

enum class MouseButton {
    Left,
    Right,
    Middle,
    Unknown
};

struct Event {
    EventType type{};
    int x{0}, y{0};        // for mouse
    int width{0}, height{0}; // for resize
    int key{0};            // for keyboard
    char character{0};           // normalized ASCII/UTF-8 character for the key
    bool keyRepeat{false}; // true if KeyDown is a repeat
    MouseButton button{MouseButton::Unknown}; // which mouse button
    bool isPrintableKey() const { return character >= 32 && character <= 126; }
};

class IEventListener {
public:
    virtual ~IEventListener() = default;
    virtual void onEvent(const Event& e) = 0;
};


class AppEvents : public IEventListener {
public:
    AppEvents() {
        addHandler(EventType::MouseDown,  [this](const Event& e) { verbose && printf("App saw mouse down at %d,%d which:%d\n", e.x, e.y, e.button); });
        addHandler(EventType::MouseUp,    [this](const Event& e) { verbose && printf("App saw mouse up at %d,%d which:%d\n", e.x, e.y, e.button); });
        addHandler(EventType::MouseMove,  [this](const Event& e) { verbose && printf("App saw mouse move at %d,%d which:%d\n", e.x, e.y, e.button); });
        addHandler(EventType::KeyDown,    [this](const Event& e) { verbose && printf("App saw key down ch:%c key:%d repeat:%d\n", e.character, e.key, e.keyRepeat); });
        addHandler(EventType::KeyUp,      [this](const Event& e) { verbose && printf("App saw key up ch:%c key:%d repeat:%d\n", e.character, e.key, e.keyRepeat); });
        addHandler(EventType::Resize,     [this](const Event& e) { verbose && printf("App saw window resize to %dx%d\n", e.width, e.height); });
        addHandler(EventType::Quit,       [this](const Event& e) { verbose && printf("App saw quit event\n"); running = false; });
    }

    // Add a lambda/callback for a specific event type
    template<typename F>
    void addHandler(EventType type, F&& callback) {
        handlers[type].emplace_back(std::forward<F>(callback));
    }

    void onEvent(const Event& e) override {
        auto it = handlers.find(e.type);
        if (it != handlers.end()) {
            for (auto& f : it->second) {
                f(e); // call each registered handler
            }
        }
    }

    bool running{true};
    bool verbose{true};

private:
    std::unordered_map<EventType, std::vector<std::function<void(const Event&)>>> handlers;
};

class EventPubSubMixin {
public:
  void addListener(IEventListener* listener) {
    listeners.push_back(listener);
  }

  void dispatch(const Event& e) {
    for (auto* l : listeners) l->onEvent(e);
  }

private:
  std::vector<IEventListener*> listeners;
};

