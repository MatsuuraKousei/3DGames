#include"Aircraft.h"
#include"Missile.h"
#include"../Scene.h"
#include"../../Compornent/CameraCompornent.h"
#include"../../Compornent/InputCompornent.h"
#include"../../Compornent/ModelComponent.h"

void Aircraft::Deserialize(const json11::Json& jsonObj)
{
	GameObject::Deserialize(jsonObj);

	if (m_spCameraComponent)
	{
		m_spCameraComponent->OffsetMatrix().CreateTranslation(0.0f, 1.5f, -10.0f);
	}

	if ((GetTag() & OBJECT_TAG::TAG_Player) != 0)
	{
		Scene::GetInstance().SetTargetCamera(m_spCameraComponent);

		//プレイヤー入力
		m_spInputComponent = std::make_shared<PlayerInputComponent>(*this);
	}
	else
	{
		//敵飛行機入力	
		m_spInputComponent = std::make_shared<EnemyInputComponent>(*this);
	}

	m_spActionState = std::make_shared<ActionFly>();

	m_propRotSpeed = 0.3f;

	//軌跡ポリゴン設定
	m_propTrail.SetTexture(KdResFac.GetTexture("Data/Texture/sabelline.png"));
}

void Aircraft::Update()
{
	if (m_spInputComponent)
	{
		m_spInputComponent->Update();
	}

	m_prevPos = m_mWorld.GetTranslation();

	if (m_spActionState)
	{
		m_spActionState->Update(*this);
	}

	if (m_spCameraComponent)
	{
		m_spCameraComponent->SetCameraMatrix(m_mWorld);
	}

	UpdatePropeller();
}

void Aircraft::UpdateMove()
{

	if (m_spInputComponent == nullptr) { return; }

	const Math::Vector2& inputMove = m_spInputComponent->GetAxis(Input::Axes::L);

	//移動ベクトル
	KdVec3 move = { inputMove.x,0.0f,inputMove.y };
	move.Normalize();

	//移動速度補正
	move *= m_speed;

	//m_mWorld._41 += move.x;
	//m_mWorld._42 += move.y;
	//m_mWorld._43 += move.z;

	//移動行列作成
	//Math::Matrix moveMat = DirectX::XMMatrixTranslation(move.x, move.y, move.z);
	KdMatrix moveMat;
	moveMat.CreateTranslation(move.x, move.y, move.z);

	//ワールド行列に合成
	//m_mWorld = DirectX::XMMatrixMultiply(moveMat, m_mWorld);
	m_mWorld = moveMat * m_mWorld;

	const Math::Vector2& inputRot = m_spInputComponent->GetAxis(Input::Axes::R);

	//回転ベクトル
	//Math::Vector3 rotate = { 0.0f,0.0f,0.0f };
	KdVec3 rotate = { inputRot.x,0.0f,inputRot.y };

	/*if (GetAsyncKeyState('W') & 0x8000) { rotate.x = 1.0f; }
	if (GetAsyncKeyState('A') & 0x8000) { rotate.z = 1.0f; }
	if (GetAsyncKeyState('S') & 0x8000) { rotate.x = -1.0f; }
	if (GetAsyncKeyState('D') & 0x8000) { rotate.z = -1.0f; }*/

	//回転行列作成
	/*Math::Matrix rotateMat = DirectX::XMMatrixRotationX(rotate.x * KdToRadians);
	rotateMat = DirectX::XMMatrixMultiply(rotateMat, DirectX::XMMatrixRotationZ(rotate.z * KdToRadians));*/
	KdMatrix rotateMat;
	rotateMat.CreateRotationX(rotate.x * KdToRadians);
	rotateMat.RotateZ(rotate.z * KdToRadians);

	//ワールド行列に合成
	//m_mWorld = DirectX::XMMatrixMultiply(rotateMat, m_mWorld);
	m_mWorld = rotateMat * m_mWorld;
}

void Aircraft::ImGuiUpdate()
{
	if (ImGui::TreeNodeEx("Aircraft", ImGuiTreeNodeFlags_DefaultOpen))
	{
		KdVec3 pos;

		pos = m_mWorld.GetTranslation();

		//ImGui::Text("Position [x:%.2f] [y:%.2f] [z:%.2f]", pos.x, pos.y, pos.z);

		if (ImGui::DragFloat3("Position", &pos.x, 0.01f))
		{
			KdMatrix mTrans;
			mTrans.CreateTranslation(pos.x, pos.y, pos.z);

			m_mWorld = mTrans;
		}

		ImGui::TreePop();
	}
}

void Aircraft::UpdateShoot()
{

	if (m_spInputComponent == nullptr) { return; }

	if (m_spInputComponent->GetButton(Input::Buttons::A))
	{
		if (m_canShoot)
		{
			std::shared_ptr<Missile> spMissile = std::make_shared<Missile>();

			if (spMissile)
			{
				spMissile->Deserialize(KdResFac.GetJSON("Data/Scene/Missile.json"));

				KdMatrix mLaunch;
				mLaunch.CreateRotationX((rand() % 120 - 60.0f) * KdToRadians);
				mLaunch.RotateY((rand() % 120 - 60.0f) * KdToRadians);
				mLaunch *= m_mWorld;

				spMissile->SetMatrix(mLaunch);

				//追加
				spMissile->SetOwner(shared_from_this());

				Scene::GetInstance().AddObject(spMissile);

				//一番近いオブジェクトとの距離を格納する変数：初期値はfloatで最も大きな値を入れておく
				float minDistance = FLT_MAX;
				//誘導する予定のターゲットGameObject
				std::shared_ptr<GameObject> spTarget = nullptr;

				//全ゲームオブジェクトのリストから敵を探す
				for (auto&& object : Scene::GetInstance().GetObjects())
				{
					//発射した飛行機自信は無視
					if (object.get() == this) { continue; }

					if ((object->GetTag() & TAG_AttackHit))
					{
						//（ターゲットの座標ー自身の座標）の長さの２乗
						float distance = KdVec3(object->GetMatrix().GetTranslation() - m_mWorld.GetTranslation()).LengthSquared();

						//一番近いオブジェクトとの距離よりも近ければ
						if (distance < minDistance)
						{
							//誘導する予定のターゲットを今チェックしたGameObjectに置き換え
							spTarget = object;

							//一番近いオブジェクトとの距離を今のものに更新
							minDistance = distance;
						}
					}
				}

				//誘導するターゲットのセット
				spMissile->SetTarget(spTarget);

				////全ゲームオブジェクトのリストからミサイルが当たる対象を探す
				//for (auto object : Scene::GetInstance().GetObjects())
				//{
				//	//発射した飛行機自身は無視
				//	if (object.get() == this) { continue; }

				//	if ((object->GetTag() & TAG_AttackHit))
				//	{
				//		spMissile->SetTarget(object);

				//		break;
				//	}
				//}
			}
			m_canShoot = false;
		}
	}
	else
	{
		m_canShoot = true;
	}

	m_laser = (m_spInputComponent->GetButton(Input::Buttons::B) != InputComponent::FREE);
}

#include "EffectObject.h"
//
void Aircraft::UpdateCollision()
{
	//レーザーが有効であれば当たり判定を行う
	if (m_laser)
	{
		//レイの発射情報
		RayInfo rayInfo;
		rayInfo.m_pos = m_prevPos;				//移動する前の地点から
		rayInfo.m_dir = m_mWorld.GetAxisZ();	//自分の向いている方向に
		rayInfo.m_dir.Normalize();
		rayInfo.m_maxRange = m_laserRange;		//レーザーの射程分判定

		//レイの判定結果
		KdRayResult rayResult;

		for (auto& obj : Scene::GetInstance().GetObjects())
		{
			//自分自身を無視
			if (obj.get() == this) { continue; }

			//キャラクターと当たり判定をするのでそれ以外は無視
			if (!(obj->GetTag() & (TAG_StageObject | TAG_Character))) { continue; }

			//判定実行
			if (obj->HitCheckByRay(rayInfo, rayResult))
			{
				//当たったのであれば爆発をインスタンス化
				std::shared_ptr<EffectObject> effectObj = std::make_shared<EffectObject>();

				//相手の飛行機へのダメージ通知
				//ミサイルやレーザーの攻撃力はJsonに入れておく


				if (effectObj)
				{
					//キャラクターのリストに爆発の追加
					Scene::GetInstance().AddObject(effectObj);

					//レーザーのヒット位置＝レイの発射位置＋（レイの発射方向ベクトル＋レイが当たった地点までの距離）
					KdVec3 hitPos(rayInfo.m_pos);
					hitPos = hitPos + (rayInfo.m_dir * rayResult.m_distance);

					//爆発エフェクトの行列を計算
					KdMatrix mMat;
					mMat.CreateTranslation(hitPos.x, hitPos.y, hitPos.z);
					effectObj->SetMatrix(mMat);
				}
			}
		}
	}



	//一回の移動量と移動方向を計算
	KdVec3 moveVec = m_mWorld.GetTranslation() - m_prevPos;	//動く前→今の場所のベクトル
	float moveDistance = moveVec.Length();	//一回の移動量

	//動いていないなら判定しない
	if (moveDistance == 0.0f) { return; }

	//球情報の作成
	SphereInfo info;
	info.m_pos = m_mWorld.GetTranslation();
	info.m_radius = m_colRadius;


	for (auto& obj : Scene::GetInstance().GetObjects())
	{
		//自分自身を無視
		if (obj.get() == this) { continue; }

		//キャラクターと当たり判定をするのでそれ以外は無視
		if (!(obj->GetTag() & TAG_Character)) { continue; }

		//当たり判定
		if (obj->HitCheckBySphere(info))
		{
			Scene::GetInstance().AddDebugSphereLine(
				m_mWorld.GetTranslation(), 2.0f, { 1.0f,0.0f,0.0f,1.0f }
			);

			//移動する前の位置に戻る
			m_mWorld.SetTranslation(m_prevPos);
		}
	}

	//レイによる当たり判定
	//レイ情報の作成
	RayInfo rayInfo;
	rayInfo.m_pos = m_prevPos;			//一つ前の場所から
	rayInfo.m_dir = moveVec;			//動いた方向に向かって
	rayInfo.m_maxRange = moveDistance;	//動いた分だけ判定を行う

	rayInfo.m_dir.Normalize();

	KdRayResult rayResult;

	for (auto& obj : Scene::GetInstance().GetObjects())
	{
		//自分自身を無視
		if (obj.get() == this) { continue; }

		//背景タグは無視
		if (!(obj->GetTag() & TAG_StageObject)) { continue; }

		//判定実行
		if (obj->HitCheckByRay(rayInfo, rayResult))
		{
			//移動する前の１フレーム前に戻る
			m_mWorld.SetTranslation(m_prevPos);
		}
	}
}

void Aircraft::OnNotify_Damage(int damage)
{
	m_hp -= damage;	//相手の攻撃力分、HPをへらす

	//HPが０になったら消える
	if (m_hp <= 0)
	{
		m_spActionState = std::make_shared<ActionCrash>();
	}
}

void Aircraft::ActionFly::Update(Aircraft& owner)
{
	owner.UpdateMove();

	owner.UpdateCollision();

	owner.UpdateShoot();
}

void Aircraft::ActionCrash::Update(Aircraft& owner)
{
	if (!(--m_timer))
	{
		owner.Destroy();
	}

	KdMatrix rotation;
	rotation.CreateRotationX(0.08f);
	rotation.RotateY(0.065f);
	rotation.RotateZ(0.03f);

	owner.m_mWorld = rotation * owner.m_mWorld;

	owner.m_mWorld.Move(KdVec3(0.0f, -0.2f, 0.0f));
}

void Aircraft::UpdatePropeller()
{
	
}


void Aircraft::Draw()
{
	GameObject::Draw();		//基底クラスのDrawを呼び出す

	/*if (m_spPropeller)
	{
		m_spPropeller->Draw();
	}*/

	//レーザー距離
	if (m_laser)
	{
		//レーザーの終点を求める
		KdVec3 laserStart(m_prevPos);
		KdVec3 LaserEnd;
		KdVec3 laserDir(m_mWorld.GetAxisZ());

		laserDir.Normalize();	//拡大が入っていると1以上になるので正規化

		laserDir *= m_laserRange;	//レーザーの射程分方向ベクトル伸ばす

		LaserEnd = LaserEnd * laserDir;	//レーザーの終点は発射位置ベクトル＋レーザーの長さ分

		Scene::GetInstance().AddDebugLine(m_prevPos, LaserEnd, { 0.0f,1.0f,1.0f,1.0f });
	}
}
