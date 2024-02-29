#ifndef MOVING_LAMP_H
#define MOVING_LAMP_H

#include <DirectXMath.h>
#include <vector>
#include <chrono>
#include <random>
#include "types.h"

/*
 * A lamp that moves back and forth between two points, 
 * changing color randomly.
 */
class MovingLamp {
public:
	MovingLamp(DirectX::XMFLOAT3 start, DirectX::XMFLOAT3 end, float speed);

	void update();
	DirectX::XMFLOAT3 get_position() const;
	DirectX::XMFLOAT4 get_color() const;
	std::vector<square_instance_t> get_instances() const;

private:
	void update_instances();

	DirectX::XMFLOAT3 start;
	DirectX::XMFLOAT3 end;
	DirectX::XMFLOAT4 color;
	float speed;
	float t;
	bool forward;
	std::chrono::time_point<std::chrono::steady_clock> last_color_change;
	const int64_t color_change_interval_ms = 
		static_cast<int64_t>(60000.f / 165.0f); // 165 BPM, same as the music
	std::vector<square_instance_t> instances;
	std::mt19937 gen;
};

#endif // MOVING_LAMP_H