#pragma once
#include"EC.hpp"

class Transform :public Component
{
public:
	struct Vector
	{
		Vec3 vec;
		Vec3 pre;
		Vec3 delta;

		Vector();

		void resetVector(const Vec3& v);

		void calculate();

		Vec3 operator =(const Vec3& v);

		Vec3 operator +(const Vec3& v);

		Vec3 operator*(const Vec3& v);

		Vec3 operator *=(const Vec3& v);
	};
	struct Scale
	{
		Vector aspect;
		
		Scale();

		//以下の関数はすべてsetAspectを呼び出す
	
		//代入
		void setScale(double scale);
		void setAspect(const Vec3& aspect);
		//加算
		void addScale(double scale);
		void addAspect(const Vec3& aspect);
		//乗算
		void giveScale(double scale);
		void giveAspect(const Vec3& aspect);

		void operator = (double scale);
		void operator = (const Vec3& aspect);
		void operator +=(double scale);
		void operator +=(const Vec3& aspect);
		void operator *= (double scale);
		void operator *= (const Vec3& aspect);
	};
	struct Direction
	{
		Direction();
		Vec3 vector;
		Vec3 vertical;
		Quaternion q;//回転のための
		Quaternion accum;

		std::pair<Vec3, Vec3> asPair()const;

		//dir方向を向かせる、verRadはdirを軸に時計回りに回転させる
		void setDirection(const Vec3& dir, double verRad=0);
		void rotate(Vec3 axis, double rad);
		void rotate(const Quaternion& qua);
	};
private:
	//位置
	Vector pos;
	//方向
	Direction direction;

	//速度測定用 フレーム毎に更新
	Vec3 measureVel;
	Vector framePos;
	Vec3 measureDirVel;
	Vector frameDir;

	void calculate();

	void affectChildren();

	Transform* m_parent;
public:

	Scale scale;

	void setParent(Transform* parent);

	Transform* getParent();

	//親の影響に関する設定
	bool followParent = true;//親の移動に追従
	bool parentRotationAffectable = true;//親の回転に追従
	bool parentScalingAffectable = true;//親のスケールに追従
	//aspectを回転させるか
	bool rotatableAspect = true;

	Transform();

	virtual ~Transform() {}

	void moveBy(const Vec3& delta);

	Vec3 getAspect()const;
	//絶対座標を返す
	Vec3 getPos()const;
	//親からの相対座標　親なしなら絶対座標 unityと逆になってしまった...
	Vec3 getLocalPos()const;

	Vec3 getVel()const;
	//方向の1軸をゲット
	Vec3 getDirection()const;
	//方向の2軸をゲット
	Direction get2Direction()const;

	Vec3 getLocalDirection()const;

	Vec3 getAngulerVel()const;

	void setDirection(const Vec3& dir, double verRad = 0);

	void setDirAndPreDir(const Vec3& dir, double verRad = 0);

	void setLocalDirection(const Vec3& dir, double verRad = 0);

	void setLocalDirAndPreDir(const Vec3& dir, double verRad = 0);

	void setPos(const Vec3& pos);

	void setPosAndPrePos(const Vec3& pos);

	void setLocalPos(const Vec3& pos);

	void setLocalPosAndPrePos(const Vec3& pos);

	void rotateAt(const Vec3& center,Vec3 axis, double rad);

	void rotateAt(const Vec3& center,const Quaternion& q);

	void rotate(Vec3 axis, double rad);

	void rotate(const Quaternion& q);

	void calUpdate(double dt);
};
