#include "texture.h"
#include <algorithm>

Color Texture::getPixel(size_t u, size_t v) const{
    return Color(
        data[3 * (v * width + u)], data[3 * (v * width + u) + 1], data[3 * (v * width + u) + 2]
    );
}

Color Texture::getPixelSafe(size_t u, size_t v, RepeatMode repeat_mode) const{

    switch (repeat_mode){
        case RepeatMode::TILE:
            return getPixel(
                u % width,
                v % height
            );
            break;

        case RepeatMode::EXTEND:
            return getPixel(
                std::clamp(u, 0ul, width-1ul),
                std::clamp(v, 0ul, height-1ul)
            );
            break;
        
        case RepeatMode::MIRROR:

            u = u % (width * 2 );
            v = v % (height * 2 );

            if (u<0) u = -u;
            if (v<0) v = -v;

            if (u >= width) u = 2*width - u - 1;
            if (v >= height) v = 2*height - v - 1;

            return getPixel(
                u,
                v
            );
            break;
    }
    return getPixel(
                u,
                v
            );
}