#include"Missile.h"
#include"Application/main.h"
#include"../../Compornent/ModelComponent.h"

void Missile::Deserialize(const json11::Json& jsonObj)
{
	m_lifeSpan = APP.m_maxFps * 10;

	if (jsonObj.is_null()) { return; }

	GameObject::Deserialize(jsonObj);

	if (!jsonObj["Speed"].is_null())
	{
		m_speed = jsonObj["Speed"].number_value();
	}

	m_lifeSpan = APP.m_maxFps * 10;

	//煙テクスチャ
	m_trailSmoke.SetTexture(KdResFac.GetTexture("Data/Texture/smokeline2.png"));
}

void Missile::Update()
{
	if (m_alive == false) { return; }

	if (--m_lifeSpan <= 0)
	{
		Destroy();
	}

	m_prevPos = m_mWorld.GetTranslation();


	//ターゲットをshared_ptr化
	auto target = m_wpTarget.lock();



	if (target)
	{

		//自身からターゲットへのベクトル
		KdVec3 vTarget = target->GetMatrix().GetTranslation() - m_mWorld.GetTranslation();

		//単位ベクトル化：自身からターゲットへ向かう長さ１のベクトル
		vTarget.Normalize();

		//自分のZ方向（前方向）
		KdVec3 vZ = m_mWorld.GetAxisZ();

		//拡大率が入っていると計算がおかしくなるため単位ベクトル化
		vZ.Normalize();

		//※※※※※回転軸作成（この軸で回転する）※※※※※
		KdVec3 vRotAxis = KdVec3::Cross(vZ, vTarget);

		//0ベクトルなら回転しない
		if (vRotAxis.LengthSquared() != 0)
		{
			//自分のz方向ベクトルと自信からターゲットへ向かうベクトルの内積
			float d = KdVec3::Dot(vZ, vTarget);

			//誤差でー１～１以外になる可能性大なので、クランプする
			if (d > 1.0f)d = 1.0f;
			else if (d < -1.0f)d = -1.0f;

			//自分の前方向ベクトルと自信からターゲットへ向かうベクトル間の角度（radian）
			float radian = acos(d);

			//角度制限	1フレームにつき最大で1度以上回転しない
			if (radian > 1.0f * KdToRadians)
			{
				radian = 1.0f * KdToRadians;
			}

			//※※※※※radian（ここまでで回転角度が求まった）※※※※※

			KdMatrix mRot;
			mRot.CreateRotationAxis(vRotAxis, radian);
			auto pos = m_mWorld.GetTranslation();
			m_mWorld.SetTranslation({ 0,0,0 });
			m_mWorld *= mRot;
			m_mWorld.SetTranslation(pos);
		}
	}

	KdVec3 move = m_mWorld.GetAxisZ();
	move.Normalize();

	move *= m_speed;

	m_mWorld.Move(move);

	UpdateCollision();

	//軌跡の更新
	UpdateTrail();
}

//追加---------------------------------->
#include "../Scene.h"
#include"Aircraft.h"
void Missile::UpdateCollision()
{
	KdVec3 moveVec = m_mWorld.GetTranslation() - m_prevPos;

	float moveDistance = moveVec.Length();

	if (moveDistance == 0.0f) { return; }

	//発射した主人のshared_ptr取得
	auto spOwner = m_wpOwner.lock();


	//球情報の作成
	SphereInfo sInfo;
	sInfo.m_pos = m_mWorld.GetTranslation();
	sInfo.m_radius = 2.0f;

	//レイ情報の作成
	RayInfo rInfo;
	rInfo.m_pos = m_prevPos;
	rInfo.m_dir = m_mWorld.GetTranslation() - m_prevPos;
	rInfo.m_maxRange = rInfo.m_dir.Length();
	rInfo.m_dir.Normalize();

	KdRayResult rayResult;

	//全ての物体と判定を試みる
	for (auto& obj : Scene::GetInstance().GetObjects())
	{
		//自分自身は無視
		if (obj.get() == this) { continue; }
		//発射した主人も無視
		if (obj.get() == spOwner.get()) { continue; }

		bool hit = false;

		if (!(obj->GetTag() & TAG_Character)) { continue; }

		//TAG_Characterとは球判定を行う
		if (obj->GetTag() & TAG_Character)
		{
			hit = obj->HitCheckBySphere(sInfo);

			if (hit)
			{
				//std::dynamic_pointer_cast=基底クラス型をダウンキャストする時に使う、失敗するとnullptrが帰る
				//重たい、多発する場合は設計がミスっています。
				//改善したい人は先生まで相談（自分で一度考えてみよう）
				std::shared_ptr<Aircraft> aircraft = std::dynamic_pointer_cast<Aircraft>(obj);
				if (aircraft)
				{
					aircraft->OnNotify_Damage(m_attackPow);

					//爆発エフェクトを追加する

				}
			}
		}
		//TAG_StageObjectとはレイ判定を行う
		if (obj->GetTag() & TAG_StageObject)
		{
			KdRayResult rr;
			hit = obj->HitCheckByRay(rInfo, rr);
		}

		//当たったら
		if (hit)
		{
			Explosion();
			Destroy();
		}
	}
}

#include "AnimationEffect.h"
void Missile::Explosion()
{
	//アニメーションエフェクトをインスタンス化
	std::shared_ptr<AnimationEffect> effect = std::make_shared<AnimationEffect>();

	//爆発のテクスチャとアニメーション情報を渡す
	effect->SetAnimationInfo(KdResFac.GetTexture("Data/Texture/Explosion00.png"), 10.0f, 5, 5, rand() % 360);

	//場所をミサイル（自分）の位置に合わせる
	effect->SetMatrix(m_mWorld);

	//リストに追加
	Scene::GetInstance().AddObject(effect);
}

void Missile::UpdateTrail()
{

	//軌跡の座標を先頭に追加
	m_trailSmoke.AddPoint(m_mWorld);


	//軌跡の数制限（100以前の軌跡を消去する)
	if (m_trailSmoke.GetNumPoints() > 100)
	{
		m_trailSmoke.DelPoint_Back();
	}
}

void Missile::DrawEffect()
{
	if (!m_alive) { return; }

	SHADER.m_effectShader.SetWorldMatrix(KdMatrix());

	SHADER.m_effectShader.WriteToCB();

	m_trailSmoke.DrawBillboard(0.5f);
}


//<--------------------------------------
