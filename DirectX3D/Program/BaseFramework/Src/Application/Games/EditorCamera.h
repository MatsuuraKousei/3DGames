#pragma once

class EditorCamera
{
public:
	EditorCamera();			//コンストラクタ
	~EditorCamera();		//デストラクタ

	void Update();			//更新
	void SetToShader();		//ビュー行列・射影行列をシェーダーへセット
private:

	Math::Vector3	m_ViewPos;	//注視点
	Math::Matrix	m_mCam;		//注視点からの行列
	Math::Matrix	m_mProj;	//射影行列

	POINT			m_prevMousePos = { 0,0 };//前フレームのマウス座標
};