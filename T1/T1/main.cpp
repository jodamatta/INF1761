
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "scene.h"
#include "state.h"
#include "camera.h"
#include "color.h"
#include "transform.h"
#include "error.h"
#include "shader.h"
#include "disk.h"
#include "triangle.h"
#include "texture.h"
#include "rectangle.h"

#include <iostream>


static ScenePtr scene;
static CameraPtr camera;

class RotateEarthAxis;
using RotateEarthAxisPtr = std::shared_ptr<RotateEarthAxis>;
class RotateEarthAxis : public Engine
{
    TransformPtr m_trf;
protected:
    RotateEarthAxis(TransformPtr trf)
        : m_trf(trf)
    {
    }
public:
    static RotateEarthAxisPtr Make(TransformPtr trf)
    {
        return RotateEarthAxisPtr(new RotateEarthAxis(trf));
    }
    virtual void Update(float dt)
    {
        m_trf->Rotate(0.00000005f, 0, 0, 1);
    }
};

class MoonOrbit;
using MoonOrbitPtr = std::shared_ptr<MoonOrbit>;
class MoonOrbit : public Engine
{
    TransformPtr m_trf;
protected:
    MoonOrbit(TransformPtr trf)
        : m_trf(trf)
    {
    }
public:
    static MoonOrbitPtr Make(TransformPtr trf)
    {
        return MoonOrbitPtr(new MoonOrbit(trf));
    }
    virtual void Update(float dt)
    {
        m_trf->Translate(-1.25f, 0, 0);
        m_trf->Rotate(80.0f * dt, 0, 0, 1);
        m_trf->Translate(1.25f, 0, 0);
    }
};

class EarthOrbit;
using EarthOrbitPtr = std::shared_ptr<EarthOrbit>;
class EarthOrbit : public Engine
{
    TransformPtr m_trf;

protected:
    EarthOrbit(TransformPtr trf)
        : m_trf(trf)
    {
    }
public:
    static EarthOrbitPtr Make(TransformPtr trf)
    {
        return EarthOrbitPtr(new EarthOrbit(trf));
    }
    virtual void Update(float dt)
    {
        m_trf->Translate(-4.5, 0, 0);
        m_trf->Rotate(30.0f * dt, 0, 0, 1);
        m_trf->Translate(4.5, 0, 0);
    }
};

class MercuryOrbit;
using MercuryOrbitPtr = std::shared_ptr<MercuryOrbit>;
class MercuryOrbit : public Engine
{
    TransformPtr m_trf;

protected:
    MercuryOrbit(TransformPtr trf)
        : m_trf(trf)
    {
    }
public:
    static MercuryOrbitPtr Make(TransformPtr trf)
    {
        return MercuryOrbitPtr(new MercuryOrbit(trf));
    }
    virtual void Update(float dt)
    {
        m_trf->Translate(-2, 0, 0);
        m_trf->Rotate(90.0f * dt, 0, 0, 1);
        m_trf->Translate(2, 0, 0);
    }
};

static void initialize(void)
{
    // set background color: almost black
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    // enable depth test 
    glEnable(GL_DEPTH_TEST);

    // create objects
    camera = Camera::Make(0, 10, 0, 10);

    // FUNDO
    AppearancePtr textura_fundo = Texture::Make("sampler", "galaxia2.jpg");
    auto trf_fundo = Transform::Make();
    trf_fundo->Scale(10, 10, 0.9f);
    trf_fundo->Translate(0.5f, 0, 0);
    
    auto fundo = Node::Make(trf_fundo, { Color::Make(1,1,1), textura_fundo }, { Rectangle::Make() });

    // SOL
    
    auto trf_sol = Transform::Make();
    trf_sol->Translate(5, 5, 0);
    trf_sol->Scale(0.8f, 0.8f, 1);

    AppearancePtr textura_sol = Texture::Make("sampler", "sol.jpg");
    auto sol = Node::Make(trf_sol, { Color::Make(1.0f,1.0f,1.0f), textura_sol }, { Disk::Make(128) });
    
    
    // MERCURIO
    auto trf_mercurio_pos = Transform::Make();
    trf_mercurio_pos->Translate(7, 5, 0);
    
    auto mercurio_pos = Node::Make(trf_mercurio_pos);

    auto trf_mercurio_escala = Transform::Make();
    trf_mercurio_escala->Scale(0.3f, 0.3f, 1);
    auto textura_mercurio = Texture::Make("sampler", "mercurio.jpg");
    auto mercurio_escala = Node::Make(trf_mercurio_escala, { Color::Make(1.0f,1.0f,1.0f), textura_mercurio }, { Disk::Make(128) });

    mercurio_pos->AddNode(mercurio_escala);

    // TERRA
    auto trf_terra_pos = Transform::Make();
    trf_terra_pos->Translate(9.5f, 5, 0);

    auto terra_pos = Node::Make(trf_terra_pos);
    
    auto trf_terra_escala = Transform::Make();
    trf_terra_escala->Scale(0.5f, 0.5f, 0);

    auto textura_terra = Texture::Make("sampler", "terra.jpg");
    auto terra_escala = Node::Make(trf_terra_escala, { Color::Make(1.0f,1.0f,1.0f), textura_terra }, { Disk::Make(128) });

    terra_pos->AddNode(terra_escala);


    // LUA
    auto trf_lua_pos = Transform::Make();
    trf_lua_pos->Translate(1.25f, 0, 0);

    auto lua_pos = Node::Make(trf_lua_pos);

    auto trf_lua_escala = Transform::Make();
    trf_lua_escala->Scale(0.2f, 0.2f, 0);

    auto textura_lua = Texture::Make("sampler", "lua.jpg");
    auto lua_escala = Node::Make(trf_lua_escala, { Color::Make(1.0f,1.0f,1.0f), textura_lua }, { Disk::Make(128) });

    lua_pos->AddNode(lua_escala);
    terra_pos->AddNode(lua_pos);

    auto shader = Shader::Make();
    shader->AttachVertexShader("vertex.glsl");
    shader->AttachFragmentShader("fragment.glsl");
    shader->Link();

    // build scene
    auto root = Node::Make(shader, {fundo, sol, terra_pos, mercurio_pos});
    scene = Scene::Make(root);
    scene->AddEngine(MercuryOrbit::Make(trf_mercurio_pos));
    scene->AddEngine(EarthOrbit::Make(trf_terra_pos));
    scene->AddEngine(RotateEarthAxis::Make(trf_terra_escala));
    scene->AddEngine(MoonOrbit::Make(trf_lua_pos));
}

static void display(GLFWwindow* win)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear window 
    Error::Check("before render");
    scene->Render(camera);
    Error::Check("after render");
}

static void error(int code, const char* msg)
{
    printf("GLFW error %d: %s\n", code, msg);
    glfwTerminate();
    exit(0);
}

static void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void resize(GLFWwindow* win, int width, int height)
{
    glViewport(0, 0, width, height);
}

static void update(float dt)
{
    scene->Update(dt);
}

int main()
{
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwSetErrorCallback(error);

    GLFWwindow* win = glfwCreateWindow(600, 400, "Window title", nullptr, nullptr);
    glfwSetFramebufferSizeCallback(win, resize);  // resize callback
    glfwSetKeyCallback(win, keyboard);            // keyboard callback

    glfwMakeContextCurrent(win);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to initialize GLAD OpenGL context\n");
        exit(1);
    }

    initialize();

    float t0 = glfwGetTime();
    while (!glfwWindowShouldClose(win)) {
        float t = glfwGetTime();
        update(t - t0);
        t0 = t;
        display(win);
        glfwSwapBuffers(win);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

