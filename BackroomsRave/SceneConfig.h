#ifndef SCENE_CONFIG_H
#define SCENE_CONFIG_H

#include <Windows.h>
#include <vector>
#include <memory>
#include "Rectangle.h"
#include "MovingLamp.h"
#include "types.h"

/*
 * Class that holds the configuration of the scene 
 * (geometric data, texture path, etc.)
 */
class SceneConfig {
public:
	SceneConfig();
	std::vector<vertex_t> get_base_square() const;
	PCWSTR get_texture_path() const;
	std::vector<AxisRectangle> get_rectangles() const;
	std::vector<std::shared_ptr<MovingLamp>> get_lamps() const;

private:
	PCWSTR texture_path;
	std::vector<vertex_t> base_square;
	std::vector<AxisRectangle> rectangles;
	std::vector<std::shared_ptr<MovingLamp>> lamps;
};

#endif // SCENE_CONFIG_H