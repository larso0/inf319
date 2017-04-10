#ifndef ENGINE_TEXTURE_H
#define ENGINE_TEXTURE_H

#include <cstdint>
#include <string>

namespace Engine {
	class Texture {
	public:
		enum class Format : int {
			Grey = 1,
			GreyAlpha = 2,
			RGB = 3,
			RGBA = 4
		};
		
		Texture(const std::string& file);
		Texture(Format format, uint32_t w, uint32_t h);
		virtual ~Texture();
		
		Format getFormat() const {
			return format;
		}
		
		uint32_t getWidth() const {
			return width;
		}
		
		uint32_t getHeight() const {
			return height;
		}
		
		uint8_t* getPixelData() {
			return pixelData;
		}
		
		const uint8_t* getPixelData() const {
			return pixelData;
		}
		
	private:
		Format format;
		uint32_t width, height;
		uint8_t* pixelData;
		void (*deleter)(void*);
	};
}

#endif
