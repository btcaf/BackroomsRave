#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <DirectXMath.h>
#include <Windows.h>
#include <vector>

#include "types.h"

/**
 * A rectangle in 3D space along some two axes.
 * Constructed with smaller square tiles for better vertex lighting.
 */
class AxisRectangle {
	public:
		AxisRectangle(
			DirectX::XMFLOAT3 pos_lower_left,
			DirectX::XMFLOAT3 pos_upper_right,
			DirectX::XMFLOAT2 tex_lower_left,
			bool change_orientation,
			FLOAT tile_size,
			// use lighting by default
			// change to ignore lighting and set a fixed color (used for lamps)
			DirectX::XMFLOAT4 color = { 0.0f, 0.0f, 0.0f, 0.0f }
		);

	std::vector<square_instance_t> get_instances();

	private:
		std::vector<square_instance_t> instances;
};

#endif // RECTANGLE_H