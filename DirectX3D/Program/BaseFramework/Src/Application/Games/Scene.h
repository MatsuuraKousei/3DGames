#pragma once

#include"EditorCamera.h"

//前方宣言
class GameObject;
class CameraComponent;

class Scene
{
public:

	static Scene& GetInstance()
	{
		static Scene instance;
		return instance;
	}

	~Scene();	//デストラクタ

	void Init();	//初期化
	void Deserialize();
	void Release();	//開放
	void Update();	//更新
	void Draw();	//描画

	void LoadScene(const std::string& scneFilename);

	//GameObjectの値を返す
	const std::list<std::shared_ptr<GameObject>> GetObjects() const { return m_spObjects; }

	void AddObject(std::shared_ptr<GameObject> pObject);

	inline void SetTargetCamera(std::shared_ptr<CameraComponent> spCamera) { m_wpTargetCamera = spCamera; }

	void ImGuiUpdate();	//ImGuiの更新

	//デバックライン描画
	void AddDebugLine(const Math::Vector3& p1, const Math::Vector3& p2, const Math::Color& color = { 1, 1, 1, 1 });

	//デバックスフィア描画
	void AddDebugSphereLine(const Math::Vector3& pos, float radius, const Math::Color& color = { 1,1,1,1 });

	//デバッグ座標軸描画
	void AddDebugCoordinateAxisLine(const Math::Vector3& pos, float scale = 1.0f);
private:
	Scene();	//コンストラクタ


	std::shared_ptr<KdModel> m_spSky = nullptr;					//スカイスフィア
	EditorCamera	m_camera;
	bool			m_editorCaeraEnable = true;

	std::list<std::shared_ptr<GameObject>> m_spObjects;

	//ターゲットのカメラ
	std::weak_ptr<CameraComponent> m_wpTargetCamera;

	//デバックライン描画用の頂点配列
	std::vector<KdEffectShader::Vertex> m_debugLines;

	//KdSquarePolygon m_poly;
};