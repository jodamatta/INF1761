
#ifdef _WIN32
#include <glad/glad.h>
#else
#include <OpenGL/gl3.h>
#endif
#include <GLFW/glfw3.h>

#include "arcball.h"
#include "scene.h"
#include "state.h"
#include "camera3d.h"
#include "material.h"
#include "texture.h"
#include "transform.h"
#include "cube.h"
#include "quad.h"
#include "sphere.h"
#include "error.h"
#include "shader.h"
#include "light.h"
#include "light.h"
#include "polyoffset.h"

#include <iostream>
#include <glm/ext/matrix_transform.hpp>
#include "variable.h"
#include "clipplane.h"

static float viewer_pos[3] = { 2.0f, 3.5f, 4.0f };

static ScenePtr scene;
static Camera3DPtr camera;
static ArcballPtr arcball;

static void initialize(void)
{
    // set background color: white 
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    // enable depth test 
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CLIP_DISTANCE0);

    // create objects
    camera = Camera3D::Make(viewer_pos[0], viewer_pos[1], viewer_pos[2]);
    //camera->SetOrtho(true);
    arcball = camera->CreateArcball();

    LightPtr light = Light::Make(0.0f, 0.0f, 0.0f, 1.0f, "camera");

    AppearancePtr white = Material::Make(1.0f, 1.0f, 1.0f);
    AppearancePtr brown = Material::Make(0.5f, 0.3f, 0.25f);
    AppearancePtr terra = Texture::Make("decal", "terra.jpg");
    AppearancePtr sol = Texture::Make("decal", "sol.jpg");
    AppearancePtr madeira = Texture::Make("decal", "madeira.jpg");
    AppearancePtr slide = Texture::Make("slide", "smiley.jpg");

    glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.5f, 1.5f, 1.5f));
    glm::mat4 modelview = camera->GetProjMatrix() * camera->GetViewMatrix();
    glm::mat4 mat = translate * scale * modelview;
    auto mtex = Variable<glm::mat4>::Make("Mtex", mat);

    TransformPtr trf1 = Transform::Make();
    trf1->Scale(3.0f, 0.5f, 3.0f);
    trf1->Translate(0.0f, -1.0f, 0.0f);
    TransformPtr trf2 = Transform::Make();
    trf2->Scale(0.5f, 0.5f, 0.5f);
    trf2->Translate(1.0f, 1.0f, -1.0f);
    TransformPtr trf3 = Transform::Make();
    trf3->Scale(0.25f, 0.25f, 0.25f);
    trf3->Translate(-3.0f, 1.0f, 3.0f);

    Error::Check("before shps");
    ShapePtr cube = Cube::Make();
    Error::Check("before sphere");
    ShapePtr sphere1 = Sphere::Make();
    Error::Check("before sphere");
    ShapePtr sphere2 = Sphere::Make();
    Error::Check("after shps");

    ShaderPtr shader = Shader::Make(light, "world");
    shader->AttachVertexShader("vertex.glsl");
    shader->AttachFragmentShader("fragment.glsl");
    shader->Link();

    ShaderPtr shd_proj = Shader::Make(light, "world");
    shd_proj->AttachVertexShader("proj_vertex.glsl");
    shd_proj->AttachFragmentShader("proj_fragment.glsl");
    shd_proj->Link();

    ShaderPtr shd_tex = Shader::Make(light, "world");
    shd_tex->AttachVertexShader("tex_vertex.glsl");
    shd_tex->AttachFragmentShader("tex_fragment.glsl");
    shd_tex->Link();

    ClipPlanePtr clipplane = ClipPlane::Make("clipplane", 4.0f, -2.0f, 0.0f, 1.0f);

    // build scene
    NodePtr root = Node::Make(shd_proj, {mtex,slide},
        { Node::Make( trf1,{white, madeira},{cube}),
         Node::Make( trf2,{white, sol, clipplane},{sphere1}),
         Node::Make(trf3, {white, terra}, {sphere2}),
        }
    );
    scene = Scene::Make(root);
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

static void cursorpos(GLFWwindow* win, double x, double y)
{
    // convert screen pos (upside down) to framebuffer pos (e.g., retina displays)
    int wn_w, wn_h, fb_w, fb_h;
    glfwGetWindowSize(win, &wn_w, &wn_h);
    glfwGetFramebufferSize(win, &fb_w, &fb_h);
    x = x * fb_w / wn_w;
    y = (wn_h - y) * fb_h / wn_h;
    arcball->AccumulateMouseMotion(x, y);
}
static void cursorinit(GLFWwindow* win, double x, double y)
{
    // convert screen pos (upside down) to framebuffer pos (e.g., retina displays)
    int wn_w, wn_h, fb_w, fb_h;
    glfwGetWindowSize(win, &wn_w, &wn_h);
    glfwGetFramebufferSize(win, &fb_w, &fb_h);
    x = x * fb_w / wn_w;
    y = (wn_h - y) * fb_h / wn_h;
    arcball->InitMouseMotion(x, y);
    glfwSetCursorPosCallback(win, cursorpos);     // cursor position callback
}
static void mousebutton(GLFWwindow* win, int button, int action, int mods)
{
    if (action == GLFW_PRESS) {
        glfwSetCursorPosCallback(win, cursorinit);     // cursor position callback
    }
    else // GLFW_RELEASE 
        glfwSetCursorPosCallback(win, nullptr);      // callback disabled
}

int main()
{
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);       // required for mac os

    glfwSetErrorCallback(error);

    GLFWwindow* win = glfwCreateWindow(600, 400, "Window title", nullptr, nullptr);
    glfwSetFramebufferSizeCallback(win, resize);  // resize callback
    glfwSetKeyCallback(win, keyboard);            // keyboard callback
    glfwSetMouseButtonCallback(win, mousebutton); // mouse button callback

    glfwMakeContextCurrent(win);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to initialize GLAD OpenGL context\n");
        exit(1);
    }

    printf("OpenGL version: %s\n", glGetString(GL_VERSION));

    initialize();

    while (!glfwWindowShouldClose(win)) {
        display(win);
        glfwSwapBuffers(win);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

