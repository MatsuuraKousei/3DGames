#include"EffectObject.h"

void EffectObject::Update()
{
	if (m_alive == false) { return; }	//表示フラグが落ちていたら更新しない

	//寿命の更新
	if (--m_lifeSpan <= 0)
	{
		Destroy();

		return;
	}

	//徐々に大きくなっていく更新
	{
		m_scale += m_lifeSpan * 0.001f;

		//色を爆発っぽい色（黄色、オレンジ）
		m_color.R(m_lifeSpan / 30.0f);
		m_color.G(m_lifeSpan / 60.0f);
	}
}

#include"../Scene.h"
void EffectObject::Draw()
{
	if (m_alive == false) { return; }

	//デバック表示で爆発の球を描く
	Scene::GetInstance().AddDebugSphereLine(m_mWorld.GetTranslation(), m_scale, m_color);
}