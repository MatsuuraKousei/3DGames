#include"ModelComponent.h"

void ModelComponent::Draw()
{
	//有効じゃないときスキップ
	if (m_enable == false) { return; }

	//モデルがないときはスキップ
	if (m_spModel == nullptr) { return; }

	for (UINT i = 0; i < m_spModel->GetOriginalNodes().size(); i++)
	{
		auto& rNode = m_spModel->GetOriginalNodes()[i];
		if (rNode.m_spMesh == nullptr) { continue; }
		SHADER.m_standardShader.SetWorldMatrix(rNode.m_localTransform*m_owner.GetMatrix());
		SHADER.m_standardShader.DrawMesh(rNode.m_spMesh.get(),m_spModel->GetMaterials());
	}
}