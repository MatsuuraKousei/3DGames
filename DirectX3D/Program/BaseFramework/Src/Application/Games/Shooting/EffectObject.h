#pragma once

#include"../GameObject.h"

class EffectObject :public GameObject
{
public:

	void Update() override;
	void Draw() override;

private:

	float		m_scale = 0.1f;		//少しずつ拡大する
	int			m_lifeSpan = 60;	//エフェクトが表示されるフレーム数
	Math::Color m_color;			//爆発の色
};