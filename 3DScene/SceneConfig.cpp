#include "SceneConfig.h"

SceneConfig::SceneConfig() {
	texture_path = L"assets/backrooms.png";
	// floor
	rectangles.push_back(AxisRectangle(
		{ -5.0f, -1.0f, -15.0f },
		{  5.0f, -1.0f,  15.0f },
		{ 0.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ 0.0f, 1.0f, 0.0f }
	));
	// ceiling
	rectangles.push_back(AxisRectangle(
		{ -5.0f,  3.0f, -15.0f },
		{  5.0f,  3.0f,  15.0f },
		{ 0.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ 0.0f, -1.0f, 0.0f }
	));
	// north room north wall
	rectangles.push_back(AxisRectangle(
		{ -5.0f, -1.0f, 15.0f },
		{  5.0f,  3.0f, 15.0f },
		{ 0.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ 0.0f, 0.0f, -1.0f }
	));
	// north room west wall
	rectangles.push_back(AxisRectangle(
		{ -5.0f, -1.0f, 5.0f },
		{ -5.0f,  3.0f, 15.0f },
		{ 0.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ 1.0f, 0.0f, 0.0f }
	));
	// north room east wall
	rectangles.push_back(AxisRectangle(
		{ 5.0f, -1.0f, 5.0f },
		{ 5.0f,  3.0f, 15.0f },
		{ 0.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ -1.0f, 0.0f, 0.0f }
	));
	// north room southwest wall
	rectangles.push_back(AxisRectangle(
		{ -5.0f, -1.0f, 5.0f },
		{ -2.0f,  3.0f, 5.0f },
		{ 0.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ 0.0f, 0.0f, 1.0f }
	));
	// north room southeast wall
	rectangles.push_back(AxisRectangle(
		{ 2.0f, -1.0f, 5.0f },
		{ 5.0f,  3.0f, 5.0f },
		{ 0.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ 0.0f, 0.0f, 1.0f }
	));
	// corridor west wall
	rectangles.push_back(AxisRectangle(
		{ -2.0f, -1.0f, -5.0f },
		{ -2.0f,  3.0f, 5.0f },
		{ 0.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ 1.0f, 0.0f, 0.0f }
	));
	// corridor east wall
	rectangles.push_back(AxisRectangle(
		{ 2.0f, -1.0f, -5.0f },
		{ 2.0f,  3.0f, 5.0f },
		{ 0.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ -1.0f, 0.0f, 0.0f }
	));

	// south room south wall
	rectangles.push_back(AxisRectangle(
		{ -5.0f, -1.0f, -15.0f },
		{  5.0f,  3.0f, -15.0f },
		{ 0.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ 0.0f, 0.0f, 1.0f }
	));
	// south room west wall
	rectangles.push_back(AxisRectangle(
		{ -5.0f, -1.0f, -15.0f },
		{ -5.0f,  3.0f, -5.0f },
		{ 0.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ 1.0f, 0.0f, 0.0f }
	));
	// south room east wall
	rectangles.push_back(AxisRectangle(
		{ 5.0f, -1.0f, -15.0f },
		{ 5.0f,  3.0f, -5.0f },
		{ 0.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ -1.0f, 0.0f, 0.0f }
	));
	// south room northwest wall
	rectangles.push_back(AxisRectangle(
		{ -5.0f, -1.0f, -5.0f },
		{ -2.0f,  3.0f, -5.0f },
		{ 0.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ 0.0f, 0.0f, -1.0f }
	));
	// south room northeast wall
	rectangles.push_back(AxisRectangle(
		{ 2.0f, -1.0f, -5.0f },
		{ 5.0f,  3.0f, -5.0f },
		{ 0.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ 0.0f, 0.0f, -1.0f }
	));

}

PCWSTR SceneConfig::get_texture_path() const {
	return texture_path;
}

std::vector<AxisRectangle> SceneConfig::get_rectangles() const {
	return rectangles;
}