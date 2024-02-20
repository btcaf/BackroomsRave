#include "Rectangle.h"

AxisRectangle::AxisRectangle(
	DirectX::XMFLOAT3 pos_lower_left,
	DirectX::XMFLOAT3 pos_upper_right,
	DirectX::XMFLOAT2 tex_lower_left,
	DirectX::XMFLOAT2 tex_upper_right,
	DirectX::XMFLOAT3 normal
) {
	vertex_t v1 = {
		{ pos_lower_left.x, pos_lower_left.y, pos_lower_left.z },
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ tex_lower_left.x, tex_lower_left.y },
		{ normal.x, normal.y, normal.z }
	};

	vertex_t v2;
	vertex_t v3;
	if (normal.x != 0.0f) {
		v2 = {
			{ pos_lower_left.x, pos_lower_left.y, pos_upper_right.z },
			{ 1.0f, 1.0f, 1.0f, 1.0f },
			{ tex_lower_left.x, tex_upper_right.y },
			{ normal.x, normal.y, normal.z }
		};
		v3 = {
			{ pos_lower_left.x, pos_upper_right.y, pos_lower_left.z },
			{ 1.0f, 1.0f, 1.0f, 1.0f },
			{ tex_upper_right.x, tex_lower_left.y },
			{ normal.x, normal.y, normal.z }
		};
		if (normal.x > 0.0f) {
			std::swap(v2, v3);
		}
	}
	else if (normal.y != 0.0f) {
		v2 = {
			{ pos_upper_right.x, pos_lower_left.y, pos_lower_left.z },
			{ 1.0f, 1.0f, 1.0f, 1.0f },
			{ tex_lower_left.x, tex_upper_right.y },
			{ normal.x, normal.y, normal.z }
		};
		v3 = {
			{ pos_lower_left.x, pos_lower_left.y, pos_upper_right.z },
			{ 1.0f, 1.0f, 1.0f, 1.0f },
			{ tex_upper_right.x, tex_lower_left.y },
			{ normal.x, normal.y, normal.z }
		};
		if (normal.y > 0.0f) {
			std::swap(v2, v3);
		}
	}
	else {
		v2 = {
			{ pos_lower_left.x, pos_upper_right.y, pos_lower_left.z },
			{ 1.0f, 1.0f, 1.0f, 1.0f },
			{ tex_lower_left.x, tex_upper_right.y },
			{ normal.x, normal.y, normal.z }
		};
		v3 = {
			{ pos_upper_right.x, pos_lower_left.y, pos_lower_left.z },
			{ 1.0f, 1.0f, 1.0f, 1.0f },
			{ tex_upper_right.x, tex_lower_left.y },
			{ normal.x, normal.y, normal.z }
		};
		if (normal.z > 0.0f) {
			std::swap(v2, v3);
		}
	}

	vertex_t v4 = {
		{ pos_upper_right.x, pos_upper_right.y, pos_upper_right.z },
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ tex_upper_right.x, tex_upper_right.y },
		{ normal.x, normal.y, normal.z }
	};

	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v3);
	vertices.push_back(v2);
	vertices.push_back(v4);
}

std::vector<vertex_t> AxisRectangle::get_vertices() {
	return vertices;
}