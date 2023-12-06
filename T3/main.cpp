
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
static ScenePtr reflector;
static ScenePtr sombra;
static ScenePtr chao;
static Camera3DPtr camera;
static ArcballPtr arcball;

static glm::mat4 shadowMatrix(const glm::vec4& n, const glm::vec4& l) {
    float ndotl = glm::dot(glm::vec3(n), glm::vec3(l));
    return glm::mat4(
        glm::vec4(ndotl + n.w - l.x * n.x, -l.y * n.x, -l.z * n.x, -n.x),
        glm::vec4(-l.x * n.y, ndotl + n.w - l.y * n.y, -l.z * n.y, -n.y),
        glm::vec4(-l.x * n.z, -l.y * n.z, ndotl + n.w - l.z * n.z, -n.z),
        glm::vec4(-l.x * n.w, -l.y * n.w, -l.z * n.w, ndotl));
}

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

    AppearancePtr green = Material::Make(0.15f, 1.0f, 0.15f, 1.0f);
    AppearancePtr red = Material::Make(1.0f, 0.15f, 0.15f, 1.0f);
    AppearancePtr gray = Material::Make(0.8f, 0.8f, 0.8f, 0.5f);
    AppearancePtr dark_gray = Material::Make(0.8f, 0.2f, 0.1f, 1.0f);
    AppearancePtr brown = Material::Make(0.45f, 0.22f, 0.15f, 1.0f);
    AppearancePtr blue = Material::Make(0.2f, 0.3f, 1.0f, 1.0f);


    TransformPtr trf1 = Transform::Make();
    trf1->Scale(3.0f, 0.001f, 3.0f);
    trf1->Translate(0.5f, -1.0f, 0.0f);
    TransformPtr trf2 = Transform::Make();
    trf2->Scale(0.3f, 0.3f, 0.3f);
    trf2->Translate(2.0f, 1.0f, -1.0f);
    TransformPtr trf3 = Transform::Make();
    trf3->Scale(0.5f, 0.5f, 0.25f);
    trf3->Translate(2.0f, 0.75f, 3.0f);
    TransformPtr trf4 = Transform::Make();
    trf4->Scale(3.0f, 0.001f, 3.0f);
    trf4->Translate(-0.5f, -1.0f, 0.0f);
    TransformPtr trf5 = Transform::Make();
    trf5->Scale(0.4f, 0.4f, 0.4f);
    trf5->Translate(-1.5f, 1.0f, 0.0f);

    Error::Check("before shps");
    ShapePtr cube1 = Cube::Make();
    ShapePtr cube2 = Cube::Make();
    ShapePtr cube3 = Cube::Make();
    ShapePtr sphere = Sphere::Make();
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

    ShapePtr sombra1 = Sphere::Make();
    TransformPtr trf_sombra1 = Transform::Make();
    trf_sombra1->Translate(-1.4f, 0.02f, -1.0f);
    glm::mat4 sm = shadowMatrix(glm::vec4(0.0f, 1.0f, 0.0f, 0.0f), glm::vec4(3.0f, 3.0f, 3.0f, 3.0f));
    trf_sombra1->MultMatrix(sm);

    // build scene
    NodePtr root = Node::Make(shader,
        {
         Node::Make(trf2,{green},{sphere}),
         Node::Make(trf3, {red}, {cube2}),
         Node::Make(trf5, { blue }, { sphere2 }),
         Node::Make(shader, trf4, { brown }, { cube3 })
        }
    );
    scene = Scene::Make(root);
    reflector = Scene::Make(Node::Make(shader, trf1, { gray }, { cube1 }));
    sombra = Scene::Make(Node::Make(shader, trf_sombra1, { dark_gray }, { sombra1 }));
    chao = Scene::Make(Node::Make(shader, trf4, { brown }, { cube3 }));
}

static void display(GLFWwindow* win)
{
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // clear window 
    
    // reflector
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NEVER, 1, 0xFFFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    reflector->Render(camera);

    glStencilFunc(GL_EQUAL, 1, 0xFFFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    NodePtr root = scene->GetRoot();
    TransformPtr trf = Transform::Make();
    trf->Scale(1.0f, -1.0f, 1.0f);
    root->SetTransform(trf);
    glFrontFace(GL_CW);
    scene->Render(camera);
    glDisable(GL_STENCIL_TEST);
    glFrontFace(GL_CCW);
    root->SetTransform(nullptr);
    scene->Render(camera);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    reflector->Render(camera);
    glDisable(GL_BLEND);

    
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NEVER, 1, 0xFFFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    sombra->Render(camera);
    glStencilFunc(GL_EQUAL, 0, 0xFFFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glBlendFunc(GL_ONE, GL_ONE);
    glEnable(GL_BLEND);
    glDepthFunc(GL_EQUAL);
    chao->Render(camera);
    glDisable(GL_STENCIL_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND); 
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
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

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

