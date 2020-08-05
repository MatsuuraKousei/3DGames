#include "Scene.h"
#include"GameObject.h"
#include"../Compornent/CameraCompornent.h"

#include"Shooting//Aircraft.h"
#include"Shooting/Missile.h"

#include"Shooting/AnimationEffect.h"

//コンストラクタ
Scene::Scene()
{

}

//デストラクタ
Scene::~Scene()
{

}

//初期化
void Scene::Init()
{
	// jsonファイルを開く
	std::ifstream ifs("Data/test.json");
	if (ifs.fail()) { assert(0 && "jsonファイルのパスが間違っています"); }

	// 文字列として全読み込み
	std::string strjson((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

	// 文字列のJSONを解析(パース)する
	std::string err;
	json11::Json jsonObj = json11::Json::parse(strjson, err);
	if (err.size() > 0) { assert(0 && "読み込んだファイルのJson変換に失敗"); }

	// 値アクセス
	{
		OutputDebugStringA(jsonObj["Name"].string_value().append("\n").c_str());
		//auto name = jsonObj["Name"].string_value(); 本来値を取得するならこれだけでよい
		OutputDebugStringA(std::to_string(jsonObj["Hp"].int_value()).append("\n").c_str());
	}

	//配列アクセス
	{
		//配列全アクセス
		auto& pos = jsonObj["Position"].array_items();
		for (auto&& p : pos)
		{
			OutputDebugStringA(std::to_string(p.int_value()).append("\n").c_str());
		}
		//配列添字アクセス
		OutputDebugStringA(std::to_string(pos[0].int_value()).append("\n").c_str());
		OutputDebugStringA(std::to_string(pos[1].int_value()).append("\n").c_str());
		OutputDebugStringA(std::to_string(pos[2].int_value()).append("\n").c_str());
	}

	//Object取得
	{
		auto& object = jsonObj["monster"].object_items();
		OutputDebugStringA(object["name"].string_value().append("\n").c_str());
		OutputDebugStringA(std::to_string(object["pos"][0].number_value()).append("\n").c_str());
		OutputDebugStringA(std::to_string(object["pos"][0].number_value()).append("\n").c_str());
		OutputDebugStringA(std::to_string(object["pos"][1].number_value()).append("\n").c_str());
		OutputDebugStringA(std::to_string(object["pos"][2].number_value()).append("\n").c_str());
	}

	//Object配列取得
	{
		auto& objects = jsonObj["techniques"].array_items();
		for (auto&& object : objects)
		{
			//共通の要素はチェック無しでアクセス
			OutputDebugStringA(object["name"].string_value().append("\n").c_str());
			OutputDebugStringA(std::to_string(object["atk"].int_value()).append("\n").c_str());
			OutputDebugStringA(std::to_string(object["hitrate"].number_value()).append("\n").c_str());

			//固有パラメータはチェックしてからアクセス
			if (object["effect"].is_string())
			{
				OutputDebugStringA(object["effect"].string_value().append("\n").c_str());
			}
		}
	}

	m_spSky = KdResouceFactory::GetInstance().getMode("Data/Sky/Sky.gltf");

	LoadScene("Data/Scene/ShootingGame.json");

	std::shared_ptr<AnimationEffect> spExp = std::make_shared<AnimationEffect>();
	spExp->SetAnimationInfo(KdResFac.GetTexture("Data/Texture/Explosion00.png"), 10.0f, 5, 5, 0.0f);
	AddObject(spExp);
}

void Scene::Deserialize()
{
	LoadScene("Data/Scene/ShootingGame.json");
}

//開放
void Scene::Release()
{
	m_spObjects.clear();
}

//更新
void Scene::Update()
{
	if (m_editorCaeraEnable)
	{
		m_camera.Update();
	}
	
	for (auto pObject : m_spObjects)
	{
		pObject->Update();
	}

	for (auto spObjectItr = m_spObjects.begin();spObjectItr!=m_spObjects.end();)
	{
		//寿命が尽きていたらリストから除外
		if ((*spObjectItr)->IsAlive()==false)
		{
			spObjectItr = m_spObjects.erase(spObjectItr);
		}
		else
		{
			++spObjectItr;
		}
	}

	//m_poly.Animation(0.5f, true);
}

//描画
void Scene::Draw()
{
	//エディターカメラをシェーダーにセット
	//m_camera.SetToShader();

	if (m_editorCaeraEnable)
	{
		m_camera.SetToShader();
	}
	else
	{
		std::shared_ptr<CameraComponent> spCamera = m_wpTargetCamera.lock();
		if (spCamera)
		{
			spCamera->SetToShader();
		}
	}


	//ライト情報をセット
	SHADER.m_cb8_Light.Write();

	//エフェクトシェーダーを描画デバイスにセット
	SHADER.m_effectShader.SetToDevice();

	Math::Matrix skyScale = DirectX::XMMatrixScaling(100.0f, 100.0f, 100.0f);

	SHADER.m_effectShader.SetWorldMatrix(skyScale);

	//モデルの描画（メッシュ情報とマテリアルの情報を渡す）
	if (m_spSky)
	{
		SHADER.m_effectShader.DrawMesh(m_spSky->GetMesh(0).get(), m_spSky->GetMaterials());
	}

	//不透明物描画
	SHADER.m_standardShader.SetToDevice();

	for (auto spObject : m_spObjects)
	{
		spObject->Draw();
	}

	//半透明物描画
	SHADER.m_effectShader.SetToDevice();
	SHADER.m_effectShader.SetTexture(D3D.GetWhiteTex()->GetSRView());

	{//ポリゴンの描画

		D3D.GetDevContext()->OMSetDepthStencilState(SHADER.m_ds_ZEnable_ZWriteDisable, 0);
		D3D.GetDevContext()->RSSetState(SHADER.m_rs_cullNone);

		for (auto spObj : m_spObjects)
		{
			spObj->DrawEffect();
		}

		D3D.GetDevContext()->OMSetDepthStencilState(SHADER.m_ds_ZEnable_ZWriteEnable, 0);
		D3D.GetDevContext()->RSSetState(SHADER.m_rs_cullBack);
	}

	//デバックライン描画
	SHADER.m_effectShader.SetToDevice();
	SHADER.m_effectShader.SetTexture(D3D.GetWhiteTex()->GetSRView());
	{
		AddDebugLine(Math::Vector3(), Math::Vector3(0.0f, 10.0f, 0.0f));

		AddDebugSphereLine(Math::Vector3(5.0f, 5.0f, 0.0f), 2.0f);

		AddDebugCoordinateAxisLine(Math::Vector3(0.0f, 5.0f, 5.0f), 3.0f);

		//Zバッファ仕様OFF・書き込みOFF
		D3D.GetDevContext()->OMSetDepthStencilState(SHADER.m_ds_ZDisable_ZWriteDisable, 0);

		if (m_debugLines.size() >= 1)
		{
			SHADER.m_effectShader.SetWorldMatrix(Math::Matrix());

			SHADER.m_effectShader.DrawVertices(m_debugLines, D3D_PRIMITIVE_TOPOLOGY_LINELIST);

			m_debugLines.clear();
		}

		//Zバッファ仕様ON・書き込みON
		D3D.GetDevContext()->OMSetDepthStencilState(SHADER.m_ds_ZEnable_ZWriteEnable, 0);
	}
}

void Scene::AddObject(std::shared_ptr<GameObject> spObject)
{
	if (spObject == nullptr) { return; }
	m_spObjects.push_back(spObject);
}

void Scene::LoadScene(const std::string& sceneFilename)
{
	//GameObjectリストを空にする
	m_spObjects.clear();

	//JSON読み込み
	json11::Json json = KdResFac.GetJSON(sceneFilename);
	if (json.is_null())
	{
		assert(0 && "[LoadScene]jsonファイル読み込み失敗");
		return;
	}

	//オブジェクトリスト取得
	auto& objectDataList = json["GameObjects"].array_items();

	//オブジェクト生成ループ
	for (auto&& objJsonData : objectDataList)
	{
		//オブジェクト作成
		auto newGameObj = CreateGameObject(objJsonData["ClassName"].string_value());

		//プレハブ指定ありの場合は、プレハブ側のものをベースにこのJSONをマージする
		KdMergePrefab(objJsonData);

		//オブジェクトのデシリアライズ
		newGameObj->Deserialize(objJsonData);

		//リストへの追加
		AddObject(newGameObj);
	}
}

void Scene::ImGuiUpdate()
{
	if (ImGui::Begin("Scene"))
	{
		//ImGui::Text(u8"今日はいい天気だから\n飛行機の座標でも表示しようかな");
		ImGui::Checkbox("EditorCamera", &m_editorCaeraEnable);
	}
	ImGui::End();
}

//デバックライン描画
void Scene::AddDebugLine(const Math::Vector3& p1, const Math::Vector3& p2, const Math::Color& color)
{
	//ラインの開始頂点
	KdEffectShader::Vertex ver;
	ver.Color = color;
	ver.UV = { 0.0f,0.0f };
	ver.Pos = p1;
	m_debugLines.push_back(ver);

	//ラインの終端頂点
	ver.Pos = p2;
	m_debugLines.push_back(ver);
}

void Scene::AddDebugSphereLine(const Math::Vector3& pos, float radius, const Math::Color& color)
{
	KdEffectShader::Vertex ver;
	ver.Color = color;
	ver.UV = { 0.0f,0.0f };

	static constexpr int kDetail = 32;
	for (UINT i = 0; i < kDetail + 1; ++i)
	{
		//XZ平面
		ver.Pos = pos;
		ver.Pos.x += cos((float)i * (360 / kDetail) * KdToRadians) * radius;
		ver.Pos.z += sin((float)i * (360 / kDetail) * KdToRadians) * radius;
		m_debugLines.push_back(ver);

		ver.Pos = pos;
		ver.Pos.x += cos((float)(i + 1) * (360 / kDetail) * KdToRadians) * radius;
		ver.Pos.z += sin((float)(i + 1) * (360 / kDetail) * KdToRadians) * radius;
		m_debugLines.push_back(ver);

		//XY平面
		ver.Pos = pos;
		ver.Pos.x += cos((float)i * (360 / kDetail) * KdToRadians) * radius;
		ver.Pos.y += sin((float)i * (360 / kDetail) * KdToRadians) * radius;
		m_debugLines.push_back(ver);

		ver.Pos = pos;
		ver.Pos.x += cos((float)(i + 1) * (360 / kDetail) * KdToRadians) * radius;
		ver.Pos.y += sin((float)(i + 1) * (360 / kDetail) * KdToRadians) * radius;
		m_debugLines.push_back(ver);

		//YZ平面
		ver.Pos = pos;
		ver.Pos.y += cos((float)i * (360 / kDetail) * KdToRadians) * radius;
		ver.Pos.z += sin((float)i * (360 / kDetail) * KdToRadians) * radius;
		m_debugLines.push_back(ver);

		ver.Pos = pos;
		ver.Pos.y += cos((float)(i + 1) * (360 / kDetail) * KdToRadians) * radius;
		ver.Pos.z += sin((float)(i + 1) * (360 / kDetail) * KdToRadians) * radius;
		m_debugLines.push_back(ver);
	}
}

void Scene::AddDebugCoordinateAxisLine(const Math::Vector3& pos, float scale)
{

	KdEffectShader::Vertex ver;
	ver.Color = { 1.0f,1.0f,1.0f,1.0f };
	ver.UV = { 0.0f,0.0f };

	//X軸・赤
	ver.Color = { 1.0f,0.0f,0.0f,1.0f };
	ver.Pos = pos;
	m_debugLines.push_back(ver);

	ver.Pos.x += 1.0f * scale;
	m_debugLines.push_back(ver);

	//Y軸・緑
	ver.Color = { 0.0f,1.0f,0.0f,1.0f };
	ver.Pos = pos;
	m_debugLines.push_back(ver);

	ver.Pos.y += 1.0f * scale;
	m_debugLines.push_back(ver);

	//Z軸・青
	ver.Color = { 0.0f,0.0f,1.0f,1.0f };
	ver.Pos = pos;
	m_debugLines.push_back(ver);

	ver.Pos.z += 1.0f * scale;
	m_debugLines.push_back(ver);
}