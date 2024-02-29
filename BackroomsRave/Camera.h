#ifndef CAMERA_H
#define CAMERA_H

#include <DirectXMath.h>
#include <windows.h>


/*
 * Class storing the position of the camera and responsible for
 * updating it based on user input.
 */
class Camera {
	public:
		Camera(HWND hwnd);
		void update();
		DirectX::XMMATRIX get_view_matrix();
	private:
		int center_x;
		int center_y;
		DirectX::XMFLOAT3 position;
		float pitch = 0.0f;
		float yaw = 0.0f;
		const float speed = 0.1f;
		const float rotation_speed = 0.01f;
};

#endif /* CAMERA_H */