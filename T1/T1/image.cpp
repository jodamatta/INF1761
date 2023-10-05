#include "image.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>
#include <cstdlib>

Image::Image(const std::string& filename) {
	m_data = stbi_load(filename.c_str(), &m_width, &m_height, &m_nchannels, 0);
	if (!m_data) {
	std::cerr << "Could not load image: " << filename << std::endl;
		exit(1);
	}
}

ImagePtr Image::Make(const std::string& filename) {
    return ImagePtr(new Image(filename));
}

Image::~Image() {

}

int Image::GetWidth() {
	return m_width;
}

int Image::GetHeight() {
	return m_height;
}

int Image::GetNChannels() {
	return m_nchannels;
}
 
const unsigned char* Image::GetData() const {
	return m_data;
}

void Image::ExtractSubimage(int x, int y, int w, int h, unsigned char* data) {
    if (x < 0 || y < 0 || x + w > m_width || y + h > m_height) {
        std::cerr << "Subimage boundaries exceed image dimensions!" << std::endl;
        return;
    }

    int row_stride = m_width * m_nchannels;
    for (int j = 0; j < h; j++) {
        std::memcpy(data + j * w * m_nchannels,
            m_data + (y + j) * row_stride + x * m_nchannels,
            w * m_nchannels);
    }
}
