#ifndef TEXTUREATLAS_H
#define TEXTUREATLAS_H

#include "Texture.h"
#include <unordered_map>
#include <string>

namespace Engine {
	struct TextureRect {
		float x;
		float y;
		float w;
		float h;
	};

	class TextureAtlas {
	public:
		TextureAtlas(const std::string& image, const std::string& meta);
		~TextureAtlas();

		const Texture* getTexture() const {
			return texture;
		}

		const TextureRect& getRegion(const std::string& name) const {
			auto found = regions.find(name);
			return found->second;
		}
		
	private:
		Texture* texture;
		std::unordered_map<std::string, TextureRect> regions;
	};
}

#endif