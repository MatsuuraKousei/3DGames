#include "System/KdSystem.h"

#include "KdMesh.h"

#include "KdGLTFLoader.h"

//=============================================================
//
// Mesh
//
//=============================================================

void KdMesh::SetToDevice() const
{
	// 頂点バッファセット
	UINT stride = sizeof(KdMeshVertex);	// 1頂点のサイズ
	UINT offset = 0;					// オフセット
	D3D.GetDevContext()->IASetVertexBuffers(0, 1, m_vb.GetAddress(), &stride, &offset);

	// インデックスバッファセット
	D3D.GetDevContext()->IASetIndexBuffer(m_ib.GetBuffer(), DXGI_FORMAT_R32_UINT, 0);

	//プリミティブ・トポロジーをセット
	D3D.GetDevContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


bool KdMesh::Create(const std::vector<KdMeshVertex>& vertices, const std::vector<KdMeshFace>& faces, const std::vector<KdMeshSubset>& subsets)
{
	Release();

	//------------------------------
	// サブセット情報
	//------------------------------
	m_subsets = subsets;

	//------------------------------
	// 頂点バッファ作成
	//------------------------------
	if(vertices.size() > 0)
	{
		// 書き込むデータ
		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = &vertices[0];				// バッファに書き込む頂点配列の先頭アドレス
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		// バッファ作成
		if (FAILED(m_vb.Create(D3D11_BIND_VERTEX_BUFFER, sizeof(KdMeshVertex) * vertices.size(), D3D11_USAGE_DEFAULT, &initData)))
		{
			Release();
			return false;
		}

		// 座標のみの配列
		m_positions.resize(vertices.size());
		for (UINT i = 0; i < m_positions.size(); i++)
		{
			m_positions[i] = vertices[i].Pos;
		}

		// AA境界データ作成
		DirectX::BoundingBox::CreateFromPoints(m_aabb, m_positions.size(), &m_positions[0], sizeof(Math::Vector3));
		// 境界球データ作成
		DirectX::BoundingSphere::CreateFromPoints(m_bs, m_positions.size(), &m_positions[0], sizeof(Math::Vector3));
	}	

	//------------------------------
	// インデックスバッファ作成
	//------------------------------
	if(faces.size() > 0)
	{
		// 書き込むデータ
		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = &faces[0];				// バッファに書き込む頂点配列の先頭アドレス
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		// バッファ作成
		if (FAILED(m_ib.Create(D3D11_BIND_INDEX_BUFFER, faces.size() * sizeof(KdMeshFace), D3D11_USAGE_DEFAULT, &initData)))
		{
			Release();
			return false;
		}

		// 面情報コピー
		m_faces = faces;
	}

	return true;
}


void KdMesh::DrawSubset(int subsetNo) const
{
	// 範囲外のサブセットはスキップ
	if (subsetNo >= (int)m_subsets.size())return;
	// 面数が0なら描画スキップ
	if (m_subsets[subsetNo].FaceCount == 0)return;

	// 描画
	D3D.GetDevContext()->DrawIndexed(m_subsets[subsetNo].FaceCount * 3, m_subsets[subsetNo].FaceStart * 3, 0);
}
