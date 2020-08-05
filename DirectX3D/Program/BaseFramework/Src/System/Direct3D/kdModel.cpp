#include "kdModel.h"
#include "KdGLTFLoader.h"

//コンストラクタ
KdModel::KdModel()
{

}

//デストラクタ
KdModel::~KdModel()
{
	Release();
}

void KdModel::Release()
{
	m_materials.clear();
	m_originalNodes.clear();
}

//ロード関数
//bool KdModel::Load(const std::string& filename)
//{
//	std::string fileDir = KdGetDirFromPath(filename);
//
//	auto spModel = KdLoadGLTFModel(filename);
//	if (spModel == nullptr) { return false; }
//
//	//メッシュの受け取り
//	for (UINT i = 0; i < spModel->Nodes.size(); ++i)
//	{
//		const KdGLTFNode& rNode = spModel->Nodes[i];
//
//		if (rNode.IsMesh)
//		{
//			m_spMesh = std::make_shared<KdMesh>();
//
//			if (m_spMesh)
//			{
//				//				頂点情報配列		面情報配列		サブセット情報配列
//				m_spMesh->Create(rNode.Mesh.Vertices,rNode.Mesh.Faces,rNode.Mesh.Subsets);
//				break;
//			}
//		}
//	}
//
//	//マテリアル配列を受け取れるサイズのメモリ確保
//	m_materials.resize(spModel->Materials.size());
//
//	for (UINT i = 0; i < m_materials.size(); ++i)
//	{
//		//src=sourceの略
//		//dst=destinationの略
//		const KdGLTFMaterial& rSrcMaterial = spModel->Materials[i];
//		KdMaterial& rDstMatrial = m_materials[i];
//
//		//名前
//		rDstMatrial.Name = rSrcMaterial.Name;
//
//		//基本色
//		rDstMatrial.BaseColor = rSrcMaterial.BaseColor;
//		rDstMatrial.BaseColorTex = std::make_shared<KdTexture>();
//
//		if (rDstMatrial.BaseColorTex->Load(fileDir + rSrcMaterial.BaseColorTexture) == false)
//		{
//			rDstMatrial.BaseColorTex = D3D.GetWhiteTex();
//		}
//
//		//テクスチャを読み込み
//		/*if (rDstMatrial.BaseColorTex->Load(fileDir + rSrcMaterial.BaseColorTexture) == false)
//		{
//			rDstMatrial.BaseColorTex = D3D.GetWhiteTex();
//		}*/
//		rDstMatrial.BaseColorTex = KdResFac.GetTexture(fileDir + rSrcMaterial.BaseColorTexture);
//		if (rDstMatrial.BaseColorTex == nullptr)
//		{
//			rDstMatrial.BaseColorTex = D3D.GetWhiteTex();
//		}
//	}
//}

bool KdModel::Load(const std::string& filename)
{
	//ファイルの完全パスを取得
	std::string fileDir = KdGetDirFromPath(filename);

	//GLTFの読み込み
	std::shared_ptr<KdGLTFModel> spGltfModel = KdLoadGLTFModel(filename);
	if (spGltfModel == nullptr) { return false; }

	//ノード格納場所のメモリ解放
	m_originalNodes.resize(spGltfModel->Nodes.size());

	//メッシュの受け取り
	for (UINT i = 0; i < spGltfModel->Nodes.size(); ++i)
	{
		//入力元ノード
		const KdGLTFNode& rSrcNode = spGltfModel->Nodes[i];

		//出力先ノード
		Node& rDstNode = m_originalNodes[i];

		//ノード情報のセット
		rDstNode.m_name = rSrcNode.Name;
		rDstNode.m_localTransform = rSrcNode.LocalTransform;

		//ノードの内容がメッシュだったら
		if (rSrcNode.IsMesh)
		{
			rDstNode.m_spMesh = std::make_shared<KdMesh>();

			//メッシュ情報の作成
			if (rDstNode.m_spMesh)
			{
				rDstNode.m_spMesh->Create(rSrcNode.Mesh.Vertices, rSrcNode.Mesh.Faces, rSrcNode.Mesh.Subsets);
			}
		}
	}
	m_materials.resize(spGltfModel->Materials.size());
	
		for (UINT i = 0; i < m_materials.size(); ++i)
		{
			//src=sourceの略
			//dst=destinationの略
			const KdGLTFMaterial& rSrcMaterial = spGltfModel->Materials[i];
			KdMaterial& rDstMatrial = m_materials[i];
	
			//名前
			rDstMatrial.Name = rSrcMaterial.Name;
	
			//基本色
			rDstMatrial.BaseColor = rSrcMaterial.BaseColor;
			rDstMatrial.BaseColorTex = std::make_shared<KdTexture>();
	
			if (rDstMatrial.BaseColorTex->Load(fileDir + rSrcMaterial.BaseColorTexture) == false)
			{
				rDstMatrial.BaseColorTex = D3D.GetWhiteTex();
			}
	
			//テクスチャを読み込み
			/*if (rDstMatrial.BaseColorTex->Load(fileDir + rSrcMaterial.BaseColorTexture) == false)
			{
				rDstMatrial.BaseColorTex = D3D.GetWhiteTex();
			}*/
			rDstMatrial.BaseColorTex = KdResFac.GetTexture(fileDir + rSrcMaterial.BaseColorTexture);
			if (rDstMatrial.BaseColorTex == nullptr)
			{
				rDstMatrial.BaseColorTex = D3D.GetWhiteTex();
			}
		}
		return true;
}