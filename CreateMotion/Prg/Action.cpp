#include "../stdafx.h"
#include "Action.h"
#include"Condition.h"
#include"Actions.h"

using namespace prg;

IAction::IAction(double t)
	:updatePriority(0)
	, time(0)
	, timeScale(1.0)
	, id(U"")
	, startCondition(new ConditionArray())
	, endCondition(new ConditionArray())
{
	endCondition->add(new FuncCondition(
		[=] {
			return time >= t;
		}
	));
}

IAction::~IAction()
{
	if (startCondition != nullptr) delete startCondition;
	if (endCondition != nullptr) delete endCondition;
}

void IAction::setStartCondition(ConditionArray* condition)
{
	if (startCondition != nullptr)delete startCondition;
	startCondition = condition;
}

void IAction::setEndCondition(ConditionArray* condition)
{
	if (endCondition != nullptr)delete endCondition;
	endCondition = condition;
}

bool IAction::isStarted()
{
	return started;
}

bool IAction::isEnded()
{
	return ended;
}

bool IAction::isActive()
{
	return started and (not ended);
}
//
//void IAction::setTime(double t)
//{
//	if (endCondition != nullptr)delete endCondition;
//	endCondition = new FuncCondition([=]() {return time >= t; });
//}

void IAction::update(double dt)
{
	if (startCondition == nullptr)startCondition = new ConditionArray();
	if (endCondition == nullptr)endCondition = new ConditionArray();

	time += dt * timeScale;
};

MyPrint::MyPrint(const String& text, double t)
	:text(text), IAction(t)
{}

void MyPrint::update(double dt)
{
	IAction::update(dt);
	Print << text;
}
