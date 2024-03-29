#include "Camera.h"

#include <windows.h>
#include <algorithm>

Camera::Camera(HWND hwnd) {
	RECT rect;
	GetClientRect(hwnd, &rect);

	center_x = (rect.right - rect.left) / 2;
	center_y = (rect.bottom - rect.top) / 2;

	SetCursorPos(center_x, center_y);
	position = { 0.0f, 0.0f, 0.0f };
}

void Camera::update() {
	auto translate_vector = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	if (GetAsyncKeyState(0x57) & 0x8000) { // W
		translate_vector = DirectX::XMVectorAdd(
			translate_vector,
			DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f)
		);
	}
	if (GetAsyncKeyState(0x41) & 0x8000) { // A
		translate_vector = DirectX::XMVectorAdd(
			translate_vector,
			DirectX::XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f)
		);
	}
	if (GetAsyncKeyState(0x53) & 0x8000) { // S
		translate_vector = DirectX::XMVectorAdd(
			translate_vector,
			DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f)
		);
	}
	if (GetAsyncKeyState(0x44) & 0x8000) { // D
		translate_vector = DirectX::XMVectorAdd(
			translate_vector,
			DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f)
		);
	}

	translate_vector = DirectX::XMVector3Normalize(translate_vector);
	translate_vector = DirectX::XMVectorScale(translate_vector, speed);

	auto rotation_matrix = DirectX::XMMatrixRotationRollPitchYaw(0.0f, yaw, 0.0f);
	auto translate_vector_rotated = DirectX::XMVector3Transform(translate_vector, rotation_matrix);

	DirectX::XMStoreFloat3(
		&position,
		DirectX::XMVectorAdd(
			DirectX::XMLoadFloat3(&position),
			translate_vector_rotated
		)
	);

	// each frame we look for the change in cursor position
	// and reset it to the center of the window
	POINT cursor_pos;
	GetCursorPos(&cursor_pos);
	SetCursorPos(center_x, center_y);

	float dx = static_cast<float>(cursor_pos.x - center_x);
	float dy = static_cast<float>(cursor_pos.y - center_y);
	// clamp to prevent the camera from flipping
	pitch = std::clamp(pitch + dy * rotation_speed, -1.5f, 1.5f);
	yaw = fmodf(yaw + dx * rotation_speed, DirectX::XM_2PI);
}

DirectX::XMMATRIX Camera::get_view_matrix() {
	auto look_direction = DirectX::XMVector3Transform(
		DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
		DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, 0.0f)
	);

	auto target = DirectX::XMVectorAdd(
		DirectX::XMLoadFloat3(&position),
		look_direction
	);

	return DirectX::XMMatrixLookAtLH(
		DirectX::XMLoadFloat3(&position),
		target,
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
	);
}