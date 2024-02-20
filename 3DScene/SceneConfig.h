#ifndef SCENE_CONFIG_H
#define SCENE_CONFIG_H

#include <Windows.h>
#include <vector>
#include <memory>
#include "Rectangle.h"
#include "MovingLamp.h"

class SceneConfig {
public:
	SceneConfig();
	PCWSTR get_texture_path() const;
	std::vector<AxisRectangle> get_rectangles() const;
	std::vector<std::shared_ptr<MovingLamp>> get_lamps() const;

private:
	PCWSTR texture_path;
	std::vector<AxisRectangle> rectangles;
	std::vector<std::shared_ptr<MovingLamp>> lamps;
};

#endif // SCENE_CONFIG_H