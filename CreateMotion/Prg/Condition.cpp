#include "../stdafx.h"
#include "Condition.h"

using namespace prg;

ICondition::ICondition()
	:flag(false)
{
}

void ICondition::fire()
{
	flag = true;
}

bool ICondition::check()
{
	return flag;
}

bool ConditionArray::check()
{
	if (flag)return true;
	if (conditionArray.isEmpty())return false;

	bool tmp = checkType == Type::Any;

	for (auto& condition : conditionArray)
	{
		if (tmp == condition->check())return tmp;
	}

	return not tmp;
}

void ConditionArray::add(ICondition* condition)
{
	conditionArray << condition;
}
