#include "Scene.h"

Scene::Scene(SceneConfig config) {
	lamps = config.get_lamps();
	auto rectangles = config.get_rectangles();
	for (AxisRectangle rectangle : rectangles) {
		auto square_instances = rectangle.get_instances();
		const_instances.insert(const_instances.end(), square_instances.begin(),
			square_instances.end());
	}
	update_instances();
}

void Scene::update_instances() {
	instances = const_instances;
	for (auto lamp : lamps) {
		lamp->update();
		auto lamp_instances = lamp->get_instances();
		instances.insert(instances.end(), lamp_instances.begin(),
			lamp_instances.end());
	}
}

std::vector<square_instance_t> Scene::get_instances() {
	return instances;
}

std::vector<DirectX::XMFLOAT4> Scene::get_lamp_positions() const {
	std::vector<DirectX::XMFLOAT4> positions;
	for (auto lamp : lamps) {
		auto position = lamp->get_position();
		positions.push_back({
			position.x,
			position.y,
			position.z,
			0.0f
		});
	}
	return positions;
}

std::vector<DirectX::XMFLOAT4> Scene::get_lamp_colors() const {
	std::vector<DirectX::XMFLOAT4> colors;
	for (auto lamp : lamps) {
		auto color = lamp->get_color();
		colors.push_back(color);
	}
	return colors;
}