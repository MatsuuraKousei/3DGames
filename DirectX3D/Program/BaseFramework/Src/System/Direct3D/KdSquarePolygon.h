#pragma once

class KdSquarePolygon
{
public:
	//四角ポリゴンの設定
	//w:幅 h:高さ _color:色
	void Init(float w, float h, const Math::Vector4& _color);

	//ポリゴンの描画
	void Draw(int setTextureNo);

	//使用するテクスチャの設定
	inline void SetTexture(const std::shared_ptr<KdTexture>& tex)
	{
		m_texture = tex;
	}

	inline void SetAnimationInfo(int splitX, int splitY)
	{
		m_animSplitX = splitX;
		m_animSplitY = splitY;
	}

	//分割設定と渡されたコマ数を元にUV座標の計算
	//no : 設定したいコマ番号
	void SetAnimationPos(float no);
	//speed : 進行速度(1.0で一コマ進む)
	//loop : ループ再生するかどうか
	void Animation(float speed, bool loop);

	//アニメーションが終わったかどうか
	bool IsAnimationEnd();

private:

	//アニメーション関連
	int m_animSplitX = 1;	//横の分割数
	int m_animSplitY = 1;	//縦の分割数

	float	m_animPos = 0;	//現在のコマ位置

	//頂点の情報
	struct Vertex
	{
		Math::Vector3 pos;		//3D空間上の座標
		Math::Vector2 UV;		//テクスチャ座標(0-1)
		Math::Vector4 color;	//頂点の色(2点間は補充される)
	};

	Vertex m_vertex[4];	//四角形ｎ情報

	std::shared_ptr<KdTexture> m_texture;	//貼り付けるテクスチャ情報
};