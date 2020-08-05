#pragma once

class KdTrailPlygon {
public:
	//先頭のポイントを取得
	inline Math::Matrix* GetTopPoint() {
		if (m_pointList.size() == 0) { return nullptr; }
		return &m_pointList.front();
	}

	//ポイントを追加
	inline void AddPoint(const Math::Matrix& mat) {
		m_pointList.push_front(mat);
	}

	//最後尾のポイントを削除
	inline void DelPoint_Back()
	{
		m_pointList.pop_back();
	}

	//リストの数を取得
	inline int GetNumPoints() const
	{
		return (int)m_pointList.size();
	}

	//画像をセット
	inline void SetTexture(const std::shared_ptr<KdTexture>& tex)
	{
		m_texture = tex;
	}

	//通常描画	行列のX軸方向に頂点が作られ描画される
	//・width	…ポリゴンの数
	void Draw(float width);

	//ビルボード描画
	void DrawBillboard(float width);

	//頂点情報をそのまま続けてポリゴン描画
	void DrawStrip();

private:

	
	//１頂点の形式
	struct Vertex
	{
		Math::Vector3 Pos;
		Math::Vector2 UV;
		Math::Vector4 Color = { 1,1,1,1 };
	};

	//軌跡の位置などを記憶する行列配列
	std::deque<Math::Matrix>	m_pointList;

	//テクスチャ
	std::shared_ptr<KdTexture>	m_texture;
};