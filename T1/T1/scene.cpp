#include "scene.h"
#include "state.h"

#ifdef _WIN32
#include <glad/glad.h>
#else
#include <OpenGL/gl3.h>
#endif

Scene::Scene(NodePtr root)
    : m_root(root)
{
}

ScenePtr Scene::Make(NodePtr root)
{
    return ScenePtr(new Scene(root));
}
Scene::~Scene()
{
}

NodePtr Scene::GetRoot() const
{
    return m_root;
}

void Scene::AddEngine(EnginePtr engine)
{
    m_engines.push_back(engine);
}

void Scene::Update(float dt) const
{
    for (auto e : m_engines)
        e->Update(dt);
}

void Scene::Render(CameraPtr camera)
{
    StatePtr st = State::Make(camera);
    m_root->Render(st);
}
