#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include "SceneConfig.h"
#include "types.h"

class Scene {
	public:
		Scene(SceneConfig config);

		std::vector<square_instance_t> get_instances();
		void update_instances();
		std::vector<DirectX::XMFLOAT4> get_lamp_positions() const;
		std::vector<DirectX::XMFLOAT4> get_lamp_colors() const;

	private:
		std::vector<square_instance_t> const_instances;
		std::vector<std::shared_ptr<MovingLamp>> lamps;
		std::vector<square_instance_t> instances;
};

#endif // SCENE_H