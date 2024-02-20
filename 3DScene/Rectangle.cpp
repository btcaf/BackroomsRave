#include "Rectangle.h"

AxisRectangle::AxisRectangle(
	DirectX::XMFLOAT3 pos_lower_left,
	DirectX::XMFLOAT3 pos_upper_right,
	DirectX::XMFLOAT2 tex_lower_left,
	DirectX::XMFLOAT2 tex_upper_right,
	bool change_orientation,
	FLOAT tile_size,
	DirectX::XMFLOAT4 color
) {
	int axis = 0;
	DirectX::XMFLOAT2 planar_lower_left;
	DirectX::XMFLOAT2 planar_upper_right;
	if (pos_lower_left.x == pos_upper_right.x) {
		planar_lower_left = { pos_lower_left.y, pos_lower_left.z };
		planar_upper_right = { pos_upper_right.y, pos_upper_right.z };
		axis = 0;
	}
	else if (pos_lower_left.y == pos_upper_right.y) {
		planar_lower_left = { pos_lower_left.x, pos_lower_left.z };
		planar_upper_right = { pos_upper_right.x, pos_upper_right.z };
		axis = 1;
	}
	else if (pos_lower_left.z == pos_upper_right.z) {
		planar_lower_left = { pos_lower_left.x, pos_lower_left.y };
		planar_upper_right = { pos_upper_right.x, pos_upper_right.y };
		axis = 2;
	}
	else {
		throw "Invalid rectangle axis";
	}

	// Rotate the initial square
	DirectX::XMMATRIX base_world;
	switch (axis) {
	case 0:
		base_world = DirectX::XMMatrixRotationY(DirectX::XM_PIDIV2);
		break;
	case 1:
		base_world = DirectX::XMMatrixRotationX(-DirectX::XM_PIDIV2);
		break;
	default:
		base_world = DirectX::XMMatrixIdentity();
	}

	// Scale the square to the correct size
	base_world = DirectX::XMMatrixMultiply(
		base_world,
		DirectX::XMMatrixScaling(
			tile_size / 2.0f,
			tile_size / 2.0f,
			tile_size / 2.0f
		)
	);

	// orient the square correctly
	if (change_orientation) {
		if (axis != 1) {
			base_world = DirectX::XMMatrixMultiply(
				base_world,
				DirectX::XMMatrixRotationY(DirectX::XM_PI)
			);
		}
		else {
			base_world = DirectX::XMMatrixMultiply(
				base_world,
				DirectX::XMMatrixRotationX(DirectX::XM_PI)
			);
		}
	}

	// Get number of tiles
	int tiles_x = round(std::abs(planar_upper_right.x - planar_lower_left.x) / tile_size);
	int tiles_y = round(std::abs(planar_upper_right.y - planar_lower_left.y) / tile_size);

	// Create the instance data
	for (int i = 0; i < tiles_x; i++) {
		for (int j = 0; j < tiles_y; j++) {
			DirectX::XMMATRIX world;
			switch (axis) {
			case 0:
				world = DirectX::XMMatrixMultiply(
					base_world,
					DirectX::XMMatrixTranslation(
						pos_lower_left.x,
						pos_lower_left.y + i * tile_size + tile_size / 2.0f,
						pos_lower_left.z + j * tile_size + tile_size / 2.0f
					)
				);
				break;
			case 1:
				world = DirectX::XMMatrixMultiply(
					base_world,
					DirectX::XMMatrixTranslation(
						pos_lower_left.x + i * tile_size + tile_size / 2.0f,
						pos_lower_left.y,
						pos_lower_left.z + j * tile_size + tile_size / 2.0f
					)
				);
				break;
			default:
				world = DirectX::XMMatrixMultiply(
					base_world,
					DirectX::XMMatrixTranslation(
						pos_lower_left.x + i * tile_size + tile_size / 2.0f,
						pos_lower_left.y + j * tile_size + tile_size / 2.0f,
						pos_lower_left.z
					)
				);
				break;
			}

			square_instance_t instance;
			/*instance.tex_coord[0] = tex_lower_left.x;
			instance.tex_coord[1] = tex_upper_right.y;*/
			DirectX::XMStoreFloat4x4(&instance.world, world);
			instance.color = color;
			instances.push_back(instance);
		}
	}
}

std::vector<square_instance_t> AxisRectangle::get_instances() {
	return instances;
}