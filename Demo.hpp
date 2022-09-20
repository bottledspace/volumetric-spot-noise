#include "Mesh.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "Framebuffer.hpp"
#include <GL/glew.h>
#define NO_SDL_GLEXT
#include <SDL2/SDL.h>
#include <iostream>

class Demo {
public:
    Demo();
    ~Demo();

    bool create();
    bool poll();
    void swap();

    glm::mat4 projection() const;
    glm::mat4 modelview() const;
    float time() const { return m_time; }

private:
    SDL_Window* m_window;
    SDL_GLContext m_context;
    float m_angleX, m_angleY;
    float m_ofsY;
    float m_zoom;
    float m_time;
};

Demo::Demo()
: m_angleX(M_PI_4),
  m_angleY(M_PI_4),
  m_ofsY(0.0f),
  m_zoom(-10.0f),
  m_time(0.0f) {
}

Demo::~Demo() {
}

bool Demo::create() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        std::cerr << "Error: Failed to initialize SDL2." << std::endl;
        return false;
    }
    atexit(SDL_Quit);
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetSwapInterval(1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    m_window = SDL_CreateWindow("Volumetric",
        100, 100, 1024, 1024, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!m_window) {
        std::cerr << "Failed to create window. "
                  << SDL_GetError() << std::endl;
        return false;
    }

    m_context = SDL_GL_CreateContext(m_window);
    if (!m_context) {
        std::cerr << "Error: Failed to create OpenGL context. "
                  << SDL_GetError() << std::endl;
        return false;
    }
    SDL_GL_MakeCurrent(m_window, m_context);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Error: Failed to initialize GLEW." << std::endl;
        return false;
    }

    glClampColor(GL_CLAMP_FRAGMENT_COLOR, GL_FALSE);
    glClampColor(GL_CLAMP_VERTEX_COLOR, GL_FALSE);
    glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);

    SDL_ShowWindow(m_window);
    return true;
}

bool Demo::poll() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            SDL_Quit();
            return false;
        } else if (event.type == SDL_MOUSEMOTION) {
            if (event.motion.state & SDL_BUTTON_LMASK) {
                m_angleX += 0.01f*event.motion.yrel;
                m_angleY += 0.01f*event.motion.xrel;
            } else if (event.motion.state & SDL_BUTTON_RMASK) {
                m_ofsY += 0.01f*event.motion.yrel;
            }
        } else if (event.type == SDL_MOUSEWHEEL) {
            m_zoom += 0.1*event.wheel.y;
        }
    }
    m_time += 0.01f;
    return true;
}

void Demo::swap() {
    SDL_GL_SwapWindow(m_window);
}

glm::mat4 Demo::modelview() const {
    return glm::translate(glm::mat4{1.0f}, glm::vec3{0.0f,m_ofsY-1.0f,m_zoom})
         * glm::rotate(glm::mat4{1.0f}, m_angleX, {1.0f,0.0f,0.0f})
         * glm::rotate(glm::mat4{1.0f}, m_angleY, {0.0f,1.0f,0.0f});
}

glm::mat4 Demo::projection() const {
    return glm::perspective(float(M_PI)/4.0f, 1.0f, 1.0f, 500.0f);
}