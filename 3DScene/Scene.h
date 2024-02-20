#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include "SceneConfig.h"
#include "types.h"

class Scene {
	public:
		Scene(SceneConfig config);

		std::vector<vertex_t> get_vertices();

	private:
		std::vector<vertex_t> vertices;
};

#endif // SCENE_H