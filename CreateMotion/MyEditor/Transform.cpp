#include "../stdafx.h"
#include "Transform.h"

namespace {
	/// @brief 親->子
	HashTable<Transform*, Array<Transform*>>ParentChildrenRelationship;
	/// @brief 子->親
	HashTable<Transform*, Transform*>ChildParentRelationship;
}

Transform::Vector::Vector() {
	resetVector({ 0,0,0 });
}

void Transform::Vector::calculate()
{
	delta = vec - pre;
	pre = vec;
}

void Transform::Vector::resetVector(const Vec3& v)
{
	vec = pre = v;
	delta = { 0,0,0 };
}

Vec3 Transform::Vector::operator=(const Vec3& v)
{
	vec = v;
	return vec;
}

Vec3 Transform::Vector::operator+(const Vec3& v)
{
	return vec + v;
}

Vec3 Transform::Vector::operator*(const Vec3& v)
{
	return vec* v;
}

Vec3 Transform::Vector::operator*=(const Vec3& v)
{
	vec = vec * v;
	return vec;
}

Transform::Scale::Scale()
{
	aspect = { 1,1,1 };
}

void Transform::Scale::setScale(double scale)
{
	setAspect({ scale,scale,scale });
}

void Transform::Scale::setAspect(const Vec3& aspect)
{
	Scale::aspect = aspect;
}

void Transform::Scale::addScale(double scale)
{
	addAspect({ scale,scale,scale });
}

void Transform::Scale::addAspect(const Vec3& aspect)
{
	setAspect(Scale::aspect + aspect);
}

void Transform::Scale::giveScale(double scale)
{
	giveAspect({ scale,scale,scale });
}

void Transform::Scale::giveAspect(const Vec3& aspect)
{
	setAspect(Scale::aspect * aspect);
}

void Transform::Scale::operator=(double scale)
{
	setScale(scale);
}

void Transform::Scale::operator=(const Vec3& aspect)
{
	setAspect(aspect);
}

void Transform::Scale::operator+=(double scale)
{
	addScale(scale);
}

void Transform::Scale::operator+=(const Vec3& aspect)
{
	addAspect(aspect);
}

void Transform::Scale::operator*=(double scale)
{
	giveScale(scale);
}

void Transform::Scale::operator*=(const Vec3& aspect)
{
	giveAspect(aspect);
}

Transform::Direction::Direction()
	:vector({ 1,0,0 })
	, vertical({ 0,1,0 })
	,accum(Quaternion(0,0,0,1))
{
}

std::pair<Vec3, Vec3> Transform::Direction::asPair()const
{
	return std::make_pair(vector, vertical);
}

void Transform::Direction::setDirection(const Vec3& dir, double rad)
{
	rotate(Quaternion::FromUnitVectors(vector, dir));
	if (rad != 0)rotate(vector, rad);
}

void Transform::Direction::rotate(Vec3 axis, double rad)
{
	rotate(Quaternion{ axis.x * sin(rad / 2), axis.y * sin(rad / 2), axis.z * sin(rad / 2), cos(rad / 2) });
}

void Transform::Direction::rotate(const Quaternion& qua)
{
	q = qua;
	accum *= qua;
	vector = q * vector;
	vertical = q * vertical;
}

void Transform::setParent(Transform* parent)
{
	//親を取得
	auto preParent = ChildParentRelationship[this];
	if (preParent != nullptr) {
		//preParentの子の関係から自分を消す
		ParentChildrenRelationship[preParent].remove(this);
	}
	//親を更新;
	ChildParentRelationship[this] = parent;
	//parentの子に自分を追加
	ParentChildrenRelationship[parent] << this;

	m_parent = parent;
}

Transform* Transform::getParent() {
	return m_parent;
}

Transform::Transform()
{
	ParentChildrenRelationship[this] = {};
	ChildParentRelationship[this] = nullptr;
	measureVel = measureDirVel = { 0,0,0 };
	m_parent = nullptr;
}

void Transform::setDirection(const Vec3& dir, double verRad)
{
	direction.setDirection(dir, verRad);
	affectChildren();
}

void Transform::setDirAndPreDir(const Vec3& dir, double verRad)
{
	direction.setDirection(dir, verRad);
	frameDir.vec = frameDir.pre = dir;
}

void Transform::setLocalDirection(const Vec3& dir, double verRad)
{
	auto parent=ChildParentRelationship[this];
	if (parent == nullptr) {
		setDirection(dir,verRad);
		return;
	}

	auto q = Quaternion::FromUnitVectors({ 1,0,0 }, dir);
	direction.setDirection(q * parent->getDirection(),verRad);
	affectChildren();
}

void Transform::setLocalDirAndPreDir(const Vec3& dir, double verRad)
{
	auto parent = ChildParentRelationship[this];
	if (parent == nullptr) {
		setDirAndPreDir(dir);
		return;
	}

	auto q = Quaternion::FromUnitVectors({ 1,0,0 }, dir);
	direction.setDirection(q * parent->getDirection(), verRad);
	frameDir.vec = frameDir.pre = getDirection();;
}

void Transform::setPosAndPrePos(const Vec3& p)
{
	pos.vec = pos.pre = p;
	framePos.vec = framePos.pre = p;
}

void Transform::setPos(const Vec3& p)
{
	pos.vec = p;
	affectChildren();
}

void Transform::setLocalPos(const Vec3& p)
{
	auto parent = ChildParentRelationship[this];
	if (parent == nullptr) {
		setPos(p);
		return;
	}

	pos.vec = p + parent->getPos();
	affectChildren();
}

void Transform::setLocalPosAndPrePos(const Vec3& p)
{
	auto parent = ChildParentRelationship[this];
	if (parent == nullptr) {
		setPosAndPrePos(p);
		return;
	}

	pos.vec
		= pos.pre
		= framePos.vec
		= framePos.pre
		= p + parent->getPos();
}

Vec3 Transform::getAspect()const
{
	auto parent = ChildParentRelationship.at(this);
	return parentScalingAffectable and parent != nullptr ? parent->getAspect()*scale.aspect.vec : scale.aspect.vec;
}

Vec3 Transform::getPos()const
{
	auto parent = ChildParentRelationship.at(this);
	//親がいなければ座標をそのまま返す
	if (parent == nullptr)return pos.vec;
	
	if (parent->rotatableAspect)
		//親のアスペクトが回転する場合->相対座標を最初の場所に戻してからスケーリングし、元の場所に戻す。
		return parent->getPos()+parent->direction.accum * ((parent->direction.accum.inverse() * (pos.vec - parent->pos.vec)) * parent->getAspect());
	else
		//親のアスペクトが回転しない場合->そのままスケーリングする
		return parent->getPos() + parent->getAspect() * (pos.vec - parent->pos.vec);
}

Vec3 Transform::getLocalPos()const
{
	auto parent = ChildParentRelationship.at(this);
	return parent != nullptr ? getPos() - parent->getPos() : getPos();
}

Vec3 Transform::getVel()const
{
	return measureVel;
}

Vec3 Transform::getDirection()const
{
	return direction.vector;
}

Transform::Direction Transform::get2Direction()const
{
	return direction;
}

Vec3 Transform::getLocalDirection()const
{
	auto parent = ChildParentRelationship.at(this);
	if (parent == nullptr)return getDirection();
	//親がいる場合
	auto q = Quaternion::FromUnitVectors(parent->getDirection(), getDirection());
	return q*Vec3{ 1,0,0 };
}

Vec3 Transform::getAngulerVel()const
{
	return measureDirVel;
}

void Transform::moveBy(const Vec3& delta)
{
	pos.vec += delta;
	affectChildren();
}

void Transform::affectChildren()
{
	calculate();
	for (auto& child : ParentChildrenRelationship[this])
	{
		if (child->followParent)child->moveBy(pos.delta);
		if (child->parentRotationAffectable)child->rotateAt(pos.vec, direction.q);
	}
	direction.q.set(0, 0, 0, 1);
}

void Transform::calculate()
{
	pos.calculate();
}

void Transform::rotateAt(const Vec3& center, Vec3 axis, double rad)
{
	axis = axis.withLength(1);
	rotateAt(center, Quaternion{ axis.x * sin(rad / 2), axis.y * sin(rad / 2), axis.z * sin(rad / 2), cos(rad / 2) });
}

void Transform::rotateAt(const Vec3& center,const Quaternion& qua)
{
	direction.rotate(qua);
	pos.vec = qua * (pos.vec - center) + center;
	affectChildren();
}

void Transform::rotate(Vec3 axis, double rad)
{
	axis = axis.withLength(1);
	rotateAt(pos.vec, Quaternion{ axis.x * sin(rad / 2), axis.y * sin(rad / 2), axis.z * sin(rad / 2), cos(rad / 2) });
}

void Transform::rotate(const Quaternion& q)
{
	rotateAt(pos.vec, q);
}

void Transform::calUpdate(double dt)
{
	if (dt == 0)return;
	//速度を求める
	measureVel = (getPos() - framePos.vec) / dt;
	measureDirVel = (getDirection() - frameDir.vec) / dt;
	//framePos,frameDirの更新
	framePos.vec = getPos();
	frameDir.vec = getDirection();
}
