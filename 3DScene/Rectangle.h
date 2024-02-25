#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <DirectXMath.h>
#include <Windows.h>
#include <vector>

#include "types.h"

class AxisRectangle {
	public:
		AxisRectangle(
			DirectX::XMFLOAT3 pos_lower_left,
			DirectX::XMFLOAT3 pos_upper_right,
			DirectX::XMFLOAT2 tex_lower_left,
			bool change_orientation,
			FLOAT tile_size,
			DirectX::XMFLOAT4 color = { 0.0f, 0.0f, 0.0f, 0.0f } // use lighting
		);

	std::vector<square_instance_t> get_instances();

	private:
		std::vector<square_instance_t> instances;
};

#endif // RECTANGLE_H