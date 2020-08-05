#include "KdCollision.h"

using namespace DirectX;

bool KdRayToMesh(const XMVECTOR& rRayPos, const XMVECTOR& rRayDir, float maxDistance, const KdMesh& rMesh, const KdMatrix& rMatrix,KdRayResult& rResult)
{
	//モデルの逆行列でレイを変換
	//KdMatrix invMat = rMatrix;
	//invMat.Inverse();	//逆行列
	XMMATRIX invMat = XMMatrixInverse(0, rMatrix);

	//レイの判定開始位置を逆変換
	/*KdVec3 rayPos = rRayPos;
	rayPos.TransformCoord(invMat);*/
	XMVECTOR rayPos = XMVector3TransformCoord(rRayPos, invMat);

	//発射方向は正規化されていないと正しく判定できないので正規化
	//KdVec3 rayDir = rRayDir;
	//rayDir.TransformNormal(invMat);

	XMVECTOR rayDir = XMVector3TransformNormal(rRayDir, invMat);



	//逆行列に拡縮が入っていると
	//レイが当たった距離にも拡縮が反映されてしまうので
	//判定用の最大距離にも拡縮を反映させておく
	//float rayCheckRange = maxDistance * XMVector3Length(rayDir).m128_f32[0];
	//<--------------------------------追加
	float dirLength = XMVector3Length(rayDir).m128_f32[0];
	float rayCheckRange = maxDistance * dirLength;

	//rayDir.Normalize();
	rayDir = XMVector3Normalize(rayDir);

	//-----------------------------------------------------
	//ブロードフェイズ
	//	比較的軽量なAABB vs レイな判定で、
	//　あきらかにヒットしない場合は、これ以降の判定を中止
	//-----------------------------------------------------
	{
		//AABB vs レイ
		float AABBdist = FLT_MAX;
		if (rMesh.GetBoundingBox().Intersects(rayPos, rayDir, AABBdist) == false) { return false; }

		//最大距離以降なら範囲外なので中止

		if (AABBdist > rayCheckRange) { return false; }
	}

	//------------------------------------------------------
	//ナローフェイズ
	//	レイ　vs 全ての面
	//------------------------------------------------------
	bool ret = false;				//当たったかどうか
	float closestDist = FLT_MAX;	//当たった面の距離


	//変更----------------------------------------------------------------------->
	//面情報の取得
	//const std::shared_ptr<KdMesh>& mesh = m_spModelComponent->GetMesh();	//モデル（メッシュ）情報の取得
	//const KdMeshFace* pFaces = &mesh->GetFaces()[0];						//面情報の先頭を取得
	//UINT faceNum = mesh->GetFaces().size();									//面の総数を取得
	//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
	const KdMeshFace* pFaces = &rMesh.GetFaces()[0];
	UINT faceNum = rMesh.GetFaces().size();
	//<-----------------------------------------------------------------------変更


	//全ての面（三角形）と当たり判定
	for (UINT faceIdx = 0; faceIdx < faceNum; ++faceIdx)
	{
		//三角形を構成する３つの頂点のIndex
		const UINT* idx = pFaces[faceIdx].Idx;

		//レイと三角形の当たり判定
		float triDist = FLT_MAX;
		bool bHit = DirectX::TriangleTests::Intersects(
			//rInfo.m_pos,		//発射場所
			rayPos,		//変更
			rayDir,				//発射方向

			//判定する３角形の頂点情報
			rMesh.GetVertexPositions()[idx[0]],
			rMesh.GetVertexPositions()[idx[1]],
			rMesh.GetVertexPositions()[idx[2]],

			triDist	//当たった場合の距離
		);

		//ヒットしていなかったらスキップ
		if (bHit == false) { continue; }

		//最大距離以内か
		if (triDist <= rayCheckRange)
		{
			//return true;

			ret = true;		//当たったとする

			//当たり判定でとれる距離は拡縮に影響以内なので、実際の長さを計算する
			triDist /= dirLength;

			if (triDist < closestDist)
			{
				closestDist = triDist;		//距離を更新
			}
		}
	}
	//return false;

	rResult.m_distance = closestDist;
	rResult.m_hit = ret;
	return ret;
}
