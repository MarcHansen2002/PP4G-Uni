#ifndef SMOKE_HELPERS_MIN_SHADER
#define SMOKE_HELPERS_MIN_SHADER

int GetCellIndex(int x, int y)
{
	return x + (y * GridSizeX);
}

int GetVelocityXIndex(int x, int y)
{
	return x + y * (GridSizeX + 1);
}

int GetVelocityYIndex(int x, int y)
{
	return x + y * GridSizeX;
}

bool IsSolid(int x, int y)
{
	return x <= 0 || x >= GridSizeX - 1 || y <= 0 || y >= GridSizeY - 1;
}

bool IsSolid(ivec2 pos)
{
    return IsSolid(pos.x, pos.y);
}

float SampleBilinear(float x0, float x1, float y0, float y1, vec2 t)
{
	float bottom = mix(x0, x1, t.x);
	float top = mix(y0, y1, t.x);

	return mix(bottom, top, t.y);
}

#endif