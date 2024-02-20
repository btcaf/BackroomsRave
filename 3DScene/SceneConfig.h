#ifndef SCENE_CONFIG_H
#define SCENE_CONFIG_H

#include <Windows.h>
#include "Rectangle.h"
#include <vector>

class SceneConfig {
public:
	SceneConfig();
	PCWSTR get_texture_path() const;
	std::vector<AxisRectangle> get_rectangles() const;

private:
	PCWSTR texture_path;
	std::vector<AxisRectangle> rectangles;
};

#endif // SCENE_CONFIG_H