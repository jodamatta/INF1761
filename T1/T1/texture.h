#pragma once
#include <memory>
#include "appearance.h"
class Texture;
using TexturePtr = std::shared_ptr<Texture>;

class Texture : public Appearance {
	unsigned int m_tex;
	std::string m_varname;
protected:
	Texture(const std::string& varname, const std::string& filename);
	Texture(const std::string& varname, int width, int height);
	Texture(const std::string& varname, const glm::vec3& texel);
public:
	static TexturePtr Make(const std::string& varname, const std::string& filename);
	static TexturePtr Make(const std::string& varname, int width, int height);
	static TexturePtr Make(const std::string& varname, const glm::vec3& texel);
	virtual ~Texture();
	virtual void Load(StatePtr st);
	virtual void Unload(StatePtr st);
};