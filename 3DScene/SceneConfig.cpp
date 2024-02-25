#include "SceneConfig.h"

const float TILE_SIZE = 1.0f;

SceneConfig::SceneConfig() {
	texture_path = L"assets/full_first.png";
	// floor
	rectangles.push_back(AxisRectangle(
		{ -5.0f, -1.0f, -15.0f },
		{  5.0f, -1.0f,  15.0f },
		{ 0.5f, 0.0f },
		true,
		TILE_SIZE
	));
	// ceiling
	rectangles.push_back(AxisRectangle(
		{ -5.0f,  3.0f, -15.0f },
		{  5.0f,  3.0f,  15.0f },
		{ 0.0f, 0.5f },
		false,
		TILE_SIZE
	));
	// north room north wall
	rectangles.push_back(AxisRectangle(
		{ -5.0f, -1.0f, 15.0f },
		{  5.0f,  3.0f, 15.0f },
		{ 0.0f, 0.0f },
		false,
		TILE_SIZE
	));
	// north room west wall
	rectangles.push_back(AxisRectangle(
		{ -5.0f, -1.0f, 5.0f },
		{ -5.0f,  3.0f, 15.0f },
		{ 0.0f, 0.0f },
		true,
		TILE_SIZE
	));
	// north room east wall
	rectangles.push_back(AxisRectangle(
		{ 5.0f, -1.0f, 5.0f },
		{ 5.0f,  3.0f, 15.0f },
		{ 0.0f, 0.0f },
		false,
		TILE_SIZE
	));
	// north room southwest wall
	rectangles.push_back(AxisRectangle(
		{ -5.0f, -1.0f, 5.0f },
		{ -2.0f,  3.0f, 5.0f },
		{ 0.0f, 0.0f },
		true,
		TILE_SIZE
	));
	// north room southeast wall
	rectangles.push_back(AxisRectangle(
		{ 2.0f, -1.0f, 5.0f },
		{ 5.0f,  3.0f, 5.0f },
		{ 0.0f, 0.0f },
		true,
		TILE_SIZE
	));
	// corridor west wall
	rectangles.push_back(AxisRectangle(
		{ -2.0f, -1.0f, -5.0f },
		{ -2.0f,  3.0f, 5.0f },
		{ 0.0f, 0.0f },
		true,
		TILE_SIZE
	));
	// corridor east wall
	rectangles.push_back(AxisRectangle(
		{ 2.0f, -1.0f, -5.0f },
		{ 2.0f,  3.0f, 5.0f },
		{ 0.0f, 0.0f },
		false,
		TILE_SIZE
	));

	// south room south wall
	rectangles.push_back(AxisRectangle(
		{ -5.0f, -1.0f, -15.0f },
		{  5.0f,  3.0f, -15.0f },
		{ 0.0f, 0.0f },
		true,
		TILE_SIZE
	));
	// south room west wall
	rectangles.push_back(AxisRectangle(
		{ -5.0f, -1.0f, -15.0f },
		{ -5.0f,  3.0f, -5.0f },
		{ 0.0f, 0.0f },
		true,
		TILE_SIZE
	));
	// south room east wall
	rectangles.push_back(AxisRectangle(
		{ 5.0f, -1.0f, -15.0f },
		{ 5.0f,  3.0f, -5.0f },
		{ 0.0f, 0.0f },
		false,
		TILE_SIZE
	));
	// south room northwest wall
	rectangles.push_back(AxisRectangle(
		{ -5.0f, -1.0f, -5.0f },
		{ -2.0f,  3.0f, -5.0f },
		{ 0.0f, 0.0f },
		false,
		TILE_SIZE
	));
	// south room northeast wall
	rectangles.push_back(AxisRectangle(
		{ 2.0f, -1.0f, -5.0f },
		{ 5.0f,  3.0f, -5.0f },
		{ 0.0f, 0.0f },
		false,
		TILE_SIZE
	));

	// corridor lamp
	lamps.push_back(std::make_shared<MovingLamp>(
		DirectX::XMFLOAT3(0.0f, 2.75f, -9.5f),
		DirectX::XMFLOAT3(0.0f, 2.75f, 9.5f),
		0.003f
	));

	// north horizontal lamp
	lamps.push_back(std::make_shared<MovingLamp>(
		DirectX::XMFLOAT3(-3.0f, 2.75f, 10.0f),
		DirectX::XMFLOAT3(3.0f, 2.75f, 10.0f),
		0.002f
	));

	// south horizontal lamp
	lamps.push_back(std::make_shared<MovingLamp>(
		DirectX::XMFLOAT3(-3.0f, 2.75f, -10.0f),
		DirectX::XMFLOAT3(3.0f, 2.75f, -10.0f),
		0.002f
	));

	// northwest vertical lamp
	lamps.push_back(std::make_shared<MovingLamp>(
		DirectX::XMFLOAT3(-3.5f, 2.75f, 6.0f),
		DirectX::XMFLOAT3(-3.5f, 2.75f, 14.0f),
		0.002f
	));

	// northeast vertical lamp
	lamps.push_back(std::make_shared<MovingLamp>(
		DirectX::XMFLOAT3(3.5f, 2.75f, 14.0f),
		DirectX::XMFLOAT3(3.5f, 2.75f, 6.0f),
		0.002f
	));

	// southwest vertical lamp
	lamps.push_back(std::make_shared<MovingLamp>(
		DirectX::XMFLOAT3(-3.5f, 2.75f, -6.0f),
		DirectX::XMFLOAT3(-3.5f, 2.75f, -14.0f),
		0.002f
	));

	// southeast vertical lamp
	lamps.push_back(std::make_shared<MovingLamp>(
		DirectX::XMFLOAT3(3.5f, 2.75f, -14.0f),
		DirectX::XMFLOAT3(3.5f, 2.75f, -6.0f),
		0.002f
	));

}

PCWSTR SceneConfig::get_texture_path() const {
	return texture_path;
}

std::vector<AxisRectangle> SceneConfig::get_rectangles() const {
	return rectangles;
}

std::vector<std::shared_ptr<MovingLamp>> SceneConfig::get_lamps() const {
	return lamps;
}