#ifndef NOISEUTILS_H
#define NOISEUTILS_H

#include <QtGlobal>
#include <QtMath>

namespace NoiseUtils {
inline float getHeight(float worldX, float worldZ) {
    return sin(worldX * 0.1f) * cos(worldZ * 0.1f) * 5.0f;
    }
}

#endif // NOISEUTILS_H
