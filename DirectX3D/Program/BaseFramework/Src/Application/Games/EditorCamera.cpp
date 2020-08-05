#include "EditorCamera.h"
#include"../main.h"

EditorCamera::EditorCamera()
{
	m_mCam = Math::Matrix::CreateTranslation(0.0f, 0.0f, -5.0f);
	m_mCam *= Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(20));
	m_mCam *= Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(10));

	//射影行列										視野角												アスペクト比	最近接距離、再遠方距離
	m_mProj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(60), D3D.GetBackBuffer()->GetAspectRatio(), 0.01f, 5000.0f);
}

EditorCamera::~EditorCamera()
{
}

void EditorCamera::Update()
{
	POINT nowPos;
	GetCursorPos(&nowPos);

	float raito = 1.0f;
	//Shiftで大きく
	if (GetAsyncKeyState(VK_SHIFT) & 0x8000) { raito = 5.f; }
	//Controlで小さく
	if (GetAsyncKeyState(VK_CONTROL) & 0x8000) { raito = 0.2f; }

	//マウスの右ボタン
	if (GetAsyncKeyState(VK_RBUTTON) & 0x8000)
	{
		//マウスの前回からの移動量
		float deltaX = (float)(nowPos.x - m_prevMousePos.x) * 0.5f*raito;
		float deltaY = (float)(nowPos.y - m_prevMousePos.y) * 0.5f*raito;

		//カメラY軸制御
		Math::Matrix mRotateY;
		mRotateY = DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(deltaX));
		m_mCam *= mRotateY;

		//カメラX軸制御
		Math::Matrix mRotateX;
		mRotateX = DirectX::XMMatrixRotationAxis(m_mCam.Right(),DirectX::XMConvertToRadians(deltaY));
		m_mCam *= mRotateX;
	}

	//ホイールドラッグ
	if (GetAsyncKeyState(VK_MBUTTON) & 0x8000)
	{
		float deltaX = (float)(nowPos.x - m_prevMousePos.x) * 0.05f*raito;
		float deltaY = (float)(nowPos.y - m_prevMousePos.y) * 0.05f*raito;

		m_ViewPos -= m_mCam.Right() * deltaX;
		m_ViewPos += m_mCam.Up() * deltaY;
	}

	//ホイール回転
	if (APP.m_window.GetMouseWheelVal())
	{
		Math::Matrix mDistance;
		mDistance = DirectX::XMMatrixTranslation(0.f, 0.f, (float)APP.m_window.GetMouseWheelVal() * 0.01*raito);

		m_mCam *= mDistance;
	}

	m_prevMousePos = nowPos;
}

void EditorCamera::SetToShader()
{
	//カメラ行列の作成
	Math::Matrix mCam = m_mCam;

	//カメラ行列に中止点の座標を合成する
	mCam *= DirectX::XMMatrixTranslation(m_ViewPos.x, m_ViewPos.y, m_ViewPos.z);

	//カメラの座標をシェーダーにセット
	SHADER.m_cb7_Camera.Work().CamPos = mCam.Translation();

	//カメラのビュー行列をシェーダーにセット
	SHADER.m_cb7_Camera.Work().mV = mCam.Invert();

	//カメラの射影行列をセット
	SHADER.m_cb7_Camera.Work().mP = m_mProj;

	//カメラ情報（ビュー行列、射影行列）を、各シェーダーの定数バッファへセット
	SHADER.m_cb7_Camera.Write();
}