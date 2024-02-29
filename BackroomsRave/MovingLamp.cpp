#include "MovingLamp.h"
#include "Rectangle.h"

MovingLamp::MovingLamp(DirectX::XMFLOAT3 start, DirectX::XMFLOAT3 end, float speed) :
	start(start),
	end(end),
	speed(speed),
	t(0.0f),
	forward(true)
{
	color = { 1.0f, 1.0f, 1.0f, 1.0f };
	last_color_change = std::chrono::steady_clock::now();

	std::random_device rd;
	gen = std::mt19937(rd());
	update_instances();
}

void MovingLamp::update() {
	t += forward ? speed : -speed;
	if (t > 1.0f) {
		t = 1.0f;
		forward = false;
	}
	else if (t < 0.0f) {
		t = 0.0f;
		forward = true;
	}

	auto now = std::chrono::steady_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_color_change);
	if (duration.count() > color_change_interval_ms) {
		color = {
			std::uniform_real_distribution<float>(0.0f, 1.0f)(gen),
			std::uniform_real_distribution<float>(0.0f, 1.0f)(gen),
			std::uniform_real_distribution<float>(0.0f, 1.0f)(gen),
			1.0f
		};
		last_color_change = now;
	}
	update_instances();
}

DirectX::XMFLOAT3 MovingLamp::get_position() const {
	return {
		start.x + t * (end.x - start.x),
		start.y + t * (end.y - start.y),
		start.z + t * (end.z - start.z)
	};
}

DirectX::XMFLOAT4 MovingLamp::get_color() const {
	return color;
}

std::vector<square_instance_t> MovingLamp::get_instances() const {
	return instances;
}

void MovingLamp::update_instances() {
	instances.clear();
	auto pos = get_position();
	// top face
	auto top_rect = AxisRectangle(
		{ pos.x - 0.25f, pos.y + 0.25f, pos.z - 0.25f },
		{ pos.x + 0.25f, pos.y + 0.25f, pos.z + 0.25f },
		{ 0.5f, 0.5f },
		true,
		0.5f,
		color
	);

	// bottom face
	auto bottom_rect = AxisRectangle(
		{ pos.x - 0.25f, pos.y - 0.25f, pos.z - 0.25f },
		{ pos.x + 0.25f, pos.y - 0.25f, pos.z + 0.25f },
		{ 0.5f, 0.0f },
		false,
		0.5f,
		color
	);
	
	// north face
	auto north_rect = AxisRectangle(
		{ pos.x - 0.25f, pos.y - 0.25f, pos.z + 0.25f },
		{ pos.x + 0.25f, pos.y + 0.25f, pos.z + 0.25f },
		{ 0.5f, 0.0f },
		true,
		0.5f,
		color
	);
	
	// south face
	auto south_rect = AxisRectangle(
		{ pos.x - 0.25f, pos.y - 0.25f, pos.z - 0.25f },
		{ pos.x + 0.25f, pos.y + 0.25f, pos.z - 0.25f },
		{ 0.5f, 0.0f },
		false,
		0.5f,
		color
	);
	
	// west face
	auto west_rect = AxisRectangle(
		{ pos.x - 0.25f, pos.y - 0.25f, pos.z - 0.25f },
		{ pos.x - 0.25f, pos.y + 0.25f, pos.z + 0.25f },
		{ 0.5f, 0.0f },
		false,
		0.5f,
		color
	);
	
	// east face
	auto east_rect = AxisRectangle(
		{ pos.x + 0.25f, pos.y - 0.25f, pos.z - 0.25f },
		{ pos.x + 0.25f, pos.y + 0.25f, pos.z + 0.25f },
		{ 0.5f, 0.0f },
		true,
		0.5f,
		color
	);

	// insert instances
	auto top_instances = top_rect.get_instances();
	auto bottom_instances = bottom_rect.get_instances();
	auto north_instances = north_rect.get_instances();
	auto south_instances = south_rect.get_instances();
	auto east_instances = east_rect.get_instances();
	auto west_instances = west_rect.get_instances();

	instances.insert(instances.end(), top_instances.begin(), top_instances.end());
	instances.insert(instances.end(), bottom_instances.begin(), bottom_instances.end());
	instances.insert(instances.end(), north_instances.begin(), north_instances.end());
	instances.insert(instances.end(), south_instances.begin(), south_instances.end());
	instances.insert(instances.end(), west_instances.begin(), west_instances.end());
	instances.insert(instances.end(), east_instances.begin(), east_instances.end());
}