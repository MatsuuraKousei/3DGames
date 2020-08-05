#pragma once

//==============================
//リソース管理クラス
//・デザインパターンのFlyweightパターンを採用
//==============================
class KdResouceFactory
{
public:
	//モデルデータ取得
	std::shared_ptr<KdModel> getMode(const std::string& filename);

	//テクスチャデータ取得
	std::shared_ptr<KdTexture> GetTexture(const std::string& filename);

	//JSON取得
	json11::Json GetJSON(const std::string& filename);

	//管理を破棄する
	void Clear()
	{
		m_modelMap.clear();
		m_jsonMap.clear();
	}

private:

	//JSON読み込み
	json11::Json LoadJSON(const std::string& filename);

	//テクスチャー管理マップ
	std::unordered_map<std::string, std::shared_ptr<KdTexture>> m_texMap;

	//モデルデータ管理マップ
	std::unordered_map<std::string, std::shared_ptr<KdModel>> m_modelMap;

	//JSON管理マップ
	std::unordered_map<std::string, json11::Json> m_jsonMap;

	//===========================
	//シングルトン
	//===========================
private:
	KdResouceFactory() {}
public:
	static KdResouceFactory& GetInstance()
	{
		static::KdResouceFactory instance;
		return instance;
	}
};

#define KdResFac KdResouceFactory::GetInstance()