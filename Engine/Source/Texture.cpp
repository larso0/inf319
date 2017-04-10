#include <Engine/Texture.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stdexcept>

using namespace std;

namespace Engine {
	Texture::Texture(const std::string& file) :
	deleter(stbi_image_free)
	{
		int w, h, n;
		pixelData = stbi_load(file.c_str(), &w, &h, &n, 0);
		if (pixelData == nullptr) {
			throw runtime_error("Failed to load image \"" + file + "\".");
		}
		width = w;
		height = h;
		format = static_cast<Format>(n);
	}
	
	static void pixelDeleter(void* data) {
		delete[] (uint8_t*)data;
	}
	
	Texture::Texture(Format format, uint32_t w, uint32_t h) :
	format(format),
	width(w),
	height(h),
	deleter(pixelDeleter)
	{
		uint32_t size = w * h * static_cast<int>(format);
		if (size == 0) {
			throw runtime_error("Can't allocate 0 memory for image.");
		}
		pixelData = new uint8_t[size];
	}
	
	Texture::~Texture() {
		deleter(pixelData);
	}
}
