#ifndef TYPES_H
#define TYPES_H

#include <Windows.h>

struct vertex_t {
	FLOAT position[3];
	FLOAT color[4];
	FLOAT tex_coord[2];
	FLOAT normal[3];
};

#endif // TYPES_H