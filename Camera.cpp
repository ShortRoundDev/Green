#include "Camera.h"

#include "GraphicsManager.h"
#include "GTypes.h"
#include "Logger.h"

static constexpr f32 sensitivity = 0.2f;
static Logger logger = CreateLogger("Camera");

Camera::Camera()
{
    m_pos = XMFLOAT3( 0, 512, 0);

    //m_rotation = XMFLOAT3(0, 0, 0);

	auto posVec = XMLoadFloat3(&m_pos);
	XMFLOAT3 look = { 0, -1.0, 0 };
	auto lookVec = XMLoadFloat3(&look);
	XMFLOAT3 up = { 0, 1, 0 };
	auto upVec = XMLoadFloat3(&up);
	m_view = XMMatrixLookAtLH(posVec, lookVec, upVec);
	m_rotation = XMFLOAT3(90, 0, 0);

	//update(); // set view matrix once
}

Camera::~Camera()
{

}

void Camera::update()
{

	///// MOUSE LOOK /////
	f32 dX = (f32)Graphics.diffX() * sensitivity;
	f32 dY = (f32)Graphics.diffY() * sensitivity;

	m_rotation.y += dX;
	m_rotation.x += dY;

	if (m_rotation.x > 89.0f)
	{
		m_rotation.x = 89.0f;
	}
	if (m_rotation.x < -89.0f)
	{
		m_rotation.x = -89.0f;
	}

	XMVECTOR positionVec = XMLoadFloat3(&m_pos);

	XMFLOAT3 look = { 0, 0, 1.0f };
	m_look = XMLoadFloat3(&look);

	XMFLOAT3 up = { 0, 1.0f, 0 };
	XMVECTOR upVec = XMLoadFloat3(&up);

	float
		p = m_rotation.x * TO_RAD,
		y = m_rotation.y * TO_RAD,
		r = m_rotation.z * TO_RAD;

	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(p, y, r);

	m_look = XMVector3TransformCoord(m_look, rotationMatrix);
	XMVECTOR moveVec = XMVectorScale(m_look, 4);

	XMFLOAT3 forward;
	XMFLOAT3 moveF3;

	XMStoreFloat3(&forward, m_look);
	XMStoreFloat3(&moveF3, moveVec);
	///// KEY MOVE /////
	/*if (Graphics.keyDown('S'))
	{
		move({ -moveF3.x, -moveF3.y, -moveF3.z });
	}

	if (Graphics.keyDown('W'))
	{
		move(moveF3);
	}*/

	upVec = XMVector3TransformCoord(upVec, rotationMatrix);
	XMVECTOR _lookVec = XMVectorAdd(positionVec, m_look);

	m_view = XMMatrixLookAtLH(positionVec, _lookVec, upVec);
	//auto space = XMMatrixMultiply(Graphics.getProjection(), m_view);
	//orto = XMMatrixLookAtLH({ 0, 0, 0 }, { 0, 0, 1 }, { 0, 1, 0 });
	//skyViewMatrix = XMMatrixLookAtLH(positionVec, _lookVec, upVec);
}

void Camera::setPosition(float x, float y, float z)
{
	m_pos.x = x;
	m_pos.y = y;
	m_pos.z = z;
}

void Camera::setRotation(float x, float y, float z)
{
	m_rotation.x = x;
	m_rotation.y = y;
	m_rotation.z = z;
}

XMFLOAT3 Camera::getPosition()
{
	return m_pos;
}

void Camera::move(XMFLOAT3 diff)
{
	XMVECTOR pos = XMLoadFloat3(&m_pos);
	XMVECTOR diffVec = XMLoadFloat3(&diff);

	XMVECTOR newPos = XMVectorAdd(pos, diffVec);
	XMStoreFloat3(&m_pos, newPos);

	logger.info("%f, %f, %f", m_pos.x, m_pos.y, m_pos.z);
}

const XMMATRIX& Camera::getView()
{
	return m_view;
}