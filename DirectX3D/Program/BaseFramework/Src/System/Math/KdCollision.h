#pragma once
//レイ判定をしたときの結果情報
struct KdRayResult
{
	float	m_distance = -1;	//当たったところまでの距離
	bool	m_hit = false;		//当たったかどうか
};


//レイによる当たり判定
bool KdRayToMesh(const DirectX::XMVECTOR& rRayPos, const DirectX::XMVECTOR& rRayDir, float maxDistance, const KdMesh& rMesh, const KdMatrix& rMatrix,KdRayResult& rResult);