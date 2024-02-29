#ifndef TYPES_H
#define TYPES_H

#include <Windows.h>
#include <DirectXMath.h>

/*
 * Struct for vertex data
 */
struct vertex_t {
	FLOAT position[3];
	FLOAT color[4];
	FLOAT tex_coord[2];
	FLOAT normal[3];
};


/*
 * Struct for instance data
 */
struct square_instance_t {
	FLOAT tex_coord[2];
	DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT4X4 world;
};

#endif // TYPES_H