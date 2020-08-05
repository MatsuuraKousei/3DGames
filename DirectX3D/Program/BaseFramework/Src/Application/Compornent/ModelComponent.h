#pragma once

#include"../Games/GameObject.h"

//===========================================
//モデルコンポーネント
//===========================================
class ModelComponent
{
public:
	ModelComponent(GameObject& owner):m_owner(owner){}

	//有効フラグ
	bool IsEnable()const { return m_enable; }
	void SetEnable(bool enable) { m_enable = enable; }

	//モデル取得
	inline const std::shared_ptr<KdModel>& GetModel()const { return m_spModel; }

	//メッシュ取得
	inline const std::shared_ptr<KdMesh> GetMesh(UINT index)const
	{
		if (m_spModel == nullptr) { return nullptr; }
		return m_spModel->GetMesh(index);
	}

	//モデルセット
	inline void SetModel(const std::shared_ptr<KdModel>& model) { m_spModel = model; }

	//StandardShaderで描画
	void Draw();
private:

	//有効
	bool m_enable = true;

	//モデルデータの参照
	std::shared_ptr<KdModel> m_spModel;

	GameObject& m_owner;
};