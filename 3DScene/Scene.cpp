#include "Scene.h"

Scene::Scene(SceneConfig config) {
	std::vector<AxisRectangle> rectangles = config.get_rectangles();
	for (AxisRectangle rectangle : rectangles) {
		std::vector<vertex_t> rectangle_vertices = rectangle.get_vertices();
		vertices.insert(vertices.end(), rectangle_vertices.begin(), rectangle_vertices.end());
	}
}

std::vector<vertex_t> Scene::get_vertices() {
	return vertices;
}