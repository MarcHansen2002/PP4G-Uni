#ifndef SMOKE_HELPERS_SHADER
#define SMOKE_HELPERS_SHADER

// Should already have access to push constants as they are global in a shader
#include "SmokeHelpersMin.glsl"

float SampleVelocityX(vec2 pos)
{
    int width = GridSizeX + 1;
    int height = GridSizeY;

    pos -= vec2(0.0, 0.5);

    pos.x = clamp(pos.x, 0.0, float(width - 1));
    pos.y = clamp(pos.y, 0.0, float(height - 1));

    int x0 = int(floor(pos.x));
    int y0 = int(floor(pos.y));

    int x1 = min(x0 + 1, width - 1);
    int y1 = min(y0 + 1, height - 1);

    return SampleBilinear(
        VelocityX[x0 + y0 * width],
        VelocityX[x1 + y0 * width],
        VelocityX[x0 + y1 * width],
        VelocityX[x1 + y1 * width],
        fract(pos)
    );
}

float SampleVelocityY(vec2 pos)
{
	int width = GridSizeX;
    int height = GridSizeY + 1;

    pos -= vec2(0.5, 0.0);

    pos.x = clamp(pos.x, 0.0, float(width - 1));
    pos.y = clamp(pos.y, 0.0, float(height - 1));

    int x0 = int(floor(pos.x));
    int y0 = int(floor(pos.y));

    int x1 = min(x0 + 1, width - 1);
    int y1 = min(y0 + 1, height - 1);

    return SampleBilinear(
        VelocityY[x0 + y0 * width],
        VelocityY[x1 + y0 * width],
        VelocityY[x0 + y1 * width],
        VelocityY[x1 + y1 * width],
        fract(pos)
    );
}

vec2 GetVelocityAtGridPos(vec2 pos)
{
	return vec2(
        SampleVelocityX(pos),
        SampleVelocityY(pos)
    );
}


#endif