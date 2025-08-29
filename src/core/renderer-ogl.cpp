#include "renderer.h"
#ifdef __APPLE__
#include <OpenGL/gl3.h>   // Desktop GL Core profile
#else
#include <GLES2/gl2.h>    // Everywhere else
#endif
#include <vector>
#include <stdexcept>
#include <iostream>
#include "NativeParent_gl.h"


struct QuadData {
    Quad quad;
    GLuint texId;
};

struct Impl {
    uint64_t ctx; // gl context

    GLuint program = 0;
    GLuint vbo = 0;
    GLuint vao = 0;

    GLint posLoc = -1;
    GLint uvLoc  = -1;
    GLint samplerLoc = -1;
    GLint screenSizeLoc = -1;

    int screenW = 0;
    int screenH = 0;

    std::vector<QuadData> drawList;

    void checkCompile(GLuint shader, const char* type) {
        GLint status = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if (!status) {
            GLint logLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
            std::string log(logLen, ' ');
            glGetShaderInfoLog(shader, logLen, nullptr, log.data());
            throw std::runtime_error(std::string(type) + " compile error: " + log);
        }
    }

    GLuint makeShaderProgram(const char* vsSrc, const char* fsSrc) {
        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &vsSrc, nullptr);
        glCompileShader(vs);
        checkCompile(vs, "vertex");

        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &fsSrc, nullptr);
        glCompileShader(fs);
        checkCompile(fs, "fragment");

        GLuint prog = glCreateProgram();
        glAttachShader(prog, vs);
        glAttachShader(prog, fs);
        glBindAttribLocation(prog, 0, "aPos");
        glBindAttribLocation(prog, 1, "aUV");
        glLinkProgram(prog);

        GLint linked = 0;
        glGetProgramiv(prog, GL_LINK_STATUS, &linked);
        if (!linked) {
            GLint logLen = 0;
            glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLen);
            std::string log(logLen, ' ');
            glGetProgramInfoLog(prog, logLen, nullptr, log.data());
            throw std::runtime_error("link error: " + log);
        }
        glDeleteShader(vs);
        glDeleteShader(fs);
        return prog;
    }
};

// GLSL ES 2.0 shaders in string literals
static const char* vertexShaderSrc = R"(#version 100
attribute vec2 aPos;
attribute vec2 aUV;
varying vec2 vUV;
uniform vec2 uScreenSize; // width, height

void main() {
    // Convert screen pixels to NDC
    vec2 ndc;
    ndc.x = aPos.x / uScreenSize.x * 2.0 - 1.0;
    ndc.y = 1.0 - aPos.y / uScreenSize.y * 2.0; // top-left origin
    gl_Position = vec4(ndc, 0.0, 1.0);
    vUV = aUV;
}
)";


static const char* fragmentShaderSrc = R"(#version 100
precision mediump float;
varying vec2 vUV;
uniform sampler2D uTex;
void main() {
    gl_FragColor = texture2D(uTex, vUV);
}
)";

Renderer::Renderer() : impl(std::make_unique<Impl>()) {}
Renderer::~Renderer() {}

Renderer::Renderer(NativeParent& np, int width, int height) : Renderer() {
    this->init( np, width, height );
}

void Renderer::init(NativeParent& np, int width, int height) {
    impl->ctx = makeSurface(np, 640, 480);
    makeCurrent(impl->ctx);
    // now you can init shaders, buffers, etc.

    impl->screenW = width;
    impl->screenH = height;
    glViewport(0, 0, impl->screenW, impl->screenH);

    impl->program = impl->makeShaderProgram(vertexShaderSrc, fragmentShaderSrc);

    impl->posLoc = glGetAttribLocation(impl->program, "aPos");
    impl->uvLoc  = glGetAttribLocation(impl->program, "aUV");
    impl->samplerLoc = glGetUniformLocation(impl->program, "uTex");
    impl->screenSizeLoc = glGetUniformLocation(impl->program, "uScreenSize");

    glGenBuffers(1, &impl->vbo);
    glGenVertexArrays(1, &impl->vao);
    glBindVertexArray(impl->vao);
}

void Renderer::resize(int width, int height) {
    impl->screenW = width;
    impl->screenH = height;
}

void Renderer::addQuad(const Quad& quad, unsigned int textureId) {
    impl->drawList.push_back({quad, textureId});
}

void Renderer::drawFrame() {
    makeCurrent(impl->ctx);

    glViewport(0, 0, impl->screenW, impl->screenH);
    glDisable(GL_CULL_FACE);
    glClearColor(0.5f, 0.0f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(impl->program);
    glUniform2f(impl->screenSizeLoc, (float)impl->screenW, (float)impl->screenH);

    if (impl->drawList.size() == 0)
        printf( "nothing to draw\n" );

    for (auto& qd : impl->drawList) {
        float verts[16];

        // interleave pos + uv
        for (int i = 0; i < 4; i++) {
            verts[i*4 + 0] = qd.quad.verts[i*2 + 0];
            verts[i*4 + 1] = qd.quad.verts[i*2 + 1];
            verts[i*4 + 2] = qd.quad.uvs[i*2 + 0];
            verts[i*4 + 3] = qd.quad.uvs[i*2 + 1];
        }

        glBindBuffer(GL_ARRAY_BUFFER, impl->vbo);
        glBindVertexArray(impl->vao);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(impl->posLoc);
        glVertexAttribPointer(impl->posLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(impl->uvLoc);
        glVertexAttribPointer(impl->uvLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, qd.texId);
        glUniform1i(impl->samplerLoc, 0);

        // draw quad as triangle strip
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // check
        // printf("posLoc=%d uvLoc=%d samplerLoc=%d screenSizeLoc=%d\n",
        //     impl->posLoc, impl->uvLoc, impl->samplerLoc, impl->screenSizeLoc);
    }

    impl->drawList.clear();

    swapBuffers(impl->ctx);
}

unsigned int Renderer::createSolidTexture(unsigned char r, unsigned char g,
                                          unsigned char b, unsigned char a) {
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    unsigned char pixel[4] = {r, g, b, a};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixel);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return tex;
}

unsigned int Renderer::createTexture(const Texture& tex) {
    makeCurrent(impl->ctx);  // ensure GL context is active

    GLuint texId;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);

    glTexImage2D(GL_TEXTURE_2D,
                 0,                  // mip level
                 GL_RGBA,            // internal format
                 tex.width,
                 tex.height,
                 0,                  // border
                 GL_RGBA,            // format of pixel data
                 GL_UNSIGNED_BYTE,   // type of pixel data
                 tex.data);

    // nearest for pixel-perfect rendering; you can change to GL_LINEAR if desired
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // clamp to edge to avoid bleeding
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return texId;
}
