#pragma once

//3Dベクトル
class KdVec3 :public DirectX::XMFLOAT3
{
public:

	//追加------------------------------------>
	//指定行列でVectorを変換する
	KdVec3& TransformCoord(const DirectX::XMMATRIX& m)
	{
		*this = XMVector3TransformCoord(*this, m);
		return *this;
	}

	//指定（回転）行列でVectorを変換する
	KdVec3& TransformNormal(const DirectX::XMMATRIX& m)
	{
		*this = XMVector3TransformNormal(*this, m);
		return *this;
	}
	//<------------------------------------追加

	//デフォルトコンストラクタは座標の0クリアを行う
	KdVec3()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}

	//座標指定付きコンストラクタ
	KdVec3(float _x, float _y, float _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}

	//XMVECTORから代入してきた時
	KdVec3(const DirectX::XMVECTOR& v)
	{
		//変換して代入
		DirectX::XMStoreFloat3(this, v);
		//下記と同意だが、上記のSIMD命令を使用した方が高速
		/*x = v.m128_f32[0];
		y = v.m128_f32[1];
		z = v.m128_f32[2];*/
	}

	//XMVECTORへ変換
	operator DirectX::XMVECTOR() const { return DirectX::XMLoadFloat3(this); }

	//Math::Vector3と互換性を持つための関数
	operator Math::Vector3& () { return *(Math::Vector3*)this; }

	//算術演算子　乗算(*)
	KdVec3& operator*=(float s)
	{
		*this = DirectX::XMVectorScale(*this, s);
		return *this;
	}

	//自分を正規化
	void Normalize()
	{
		*this = DirectX::XMVector3Normalize(*this);
	}

	//長さ
	float Length() const
	{
		return DirectX::XMVector3Length(*this).m128_f32[0];
	}

	//長さの2乗（高速なので判定用に使用することが多い
	float LengthSquared() const
	{
		return DirectX::XMVector3LengthSq(*this).m128_f32[0];
	}

	//内積
	static float Dot(const KdVec3& v1, const KdVec3& v2)
	{
		return DirectX::XMVector3Dot(v1, v2).m128_f32[0];
	}

	//外積
	static KdVec3 Cross(const KdVec3& v1, const KdVec3& v2)
	{
		return DirectX::XMVector3Cross(v1, v2);
	}

private:
};

//4*4の行列
class KdMatrix :public DirectX::XMFLOAT4X4
{
public:

	//デフォルトコンストラクタは単位行列化
	KdMatrix()
	{
		*this = DirectX::XMMatrixIdentity();
	}


	//XMMATRIXから代入してきた
	KdMatrix(const DirectX::XMMATRIX& m)
	{
		DirectX::XMStoreFloat4x4(this, m);
	}

	//XMFLOAT4X4.Math;;Matrixから代入してきた
	KdMatrix(const DirectX::XMFLOAT4X4& m)
	{
		memcpy_s(this, sizeof(float) * 16, &m, sizeof(XMFLOAT4X4));
	}

	//XMMATRIXへ変換
	operator DirectX::XMMATRIX() const
	{
		return DirectX::XMLoadFloat4x4(this);
	}

	//Math::Matrixと互換を持つための関数
	operator Math::Matrix& () { return *(Math::Matrix*)this; }

	//代入演算子　乗算
	KdMatrix& operator*=(const KdMatrix& m)
	{
		*this = DirectX::XMMatrixMultiply(*this, m);
		return *this;
	}

	//移動行列作成
	void CreateTranslation(float x, float y, float z)
	{
		*this = DirectX::XMMatrixTranslation(x, y, z);
	}

	//回転行列作成
	void CreateRotationX(float angle)
	{
		*this = DirectX::XMMatrixRotationX(angle);
	}

	//拡縮行列作成
	void CreateScalling(float x, float y, float z)
	{
		*this = DirectX::XMMatrixScaling(x, y, z);
	}

	//指定軸回転行列作成
	void CreateRotationAxis(const KdVec3& axis, float angle)
	{
		*this = DirectX::XMMatrixRotationAxis(axis, angle);
	}

	//透過射影行列の作成
	KdMatrix& CreateProjectionPerspectiveFov(float fovAngleY, float aspecRatio, float nearZ, float farZ)
	{
		*this = DirectX::XMMatrixPerspectiveFovLH(fovAngleY, aspecRatio, nearZ, farZ);
		return *this;
	}

	//操作========================================================


	//Z軸回転
	void RotateZ(float angle)
	{
		*this *= DirectX::XMMatrixRotationZ(angle);
	}

	//Y軸回転
	void RotateY(float angle)
	{
		*this *= DirectX::XMMatrixRotationY(angle);
	}

	//拡縮
	void Scale(float x, float y, float z)
	{
		*this *= DirectX::XMMatrixScaling(x, y, z);
	}

	//逆行列にする
	void Inverse()
	{
		*this = DirectX::XMMatrixInverse(nullptr, *this);
	}

	void Move(const KdVec3 v)
	{
		_41 += v.x;
		_42 += v.y;
		_43 += v.z;
	}

	//プロパティ==================================================

	//X軸取得
	KdVec3 GetAxisX() const { return { _11,_12,_13 }; }
	//Y軸取得
	KdVec3 GetAxisY() const { return { _21,_22,_23 }; }
	//Z軸取得
	KdVec3 GetAxisZ() const { return { _31,_32,_33 }; }


	void SetAxisZ(const KdVec3& v)
	{
		_31 = v.x;
		_32 = v.y;
		_33 = v.z;
	}

	//Z軸取得
	KdVec3 GetTranslation() const { return { _41,_42,_43 }; }


	void SetTranslation(const KdVec3& v)
	{
		_41 = v.x;
		_42 = v.y;
		_43 = v.z;
	}
private:
};

// KdMatrix同士の合成
inline KdMatrix operator* (const KdMatrix& M1, const KdMatrix& M2)
{
	using namespace DirectX;
	return XMMatrixMultiply(M1, M2);
}