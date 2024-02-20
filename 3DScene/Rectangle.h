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
		DirectX::XMFLOAT2 tex_upper_right,
		DirectX::XMFLOAT3 normal
	);

	std::vector<vertex_t> get_vertices();

	private:
		std::vector<vertex_t> vertices;
};

#endif // RECTANGLE_H