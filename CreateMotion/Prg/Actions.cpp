#include "../stdafx.h"
#include "Actions.h"

using namespace prg;

Actions::~Actions()
{
	for (auto& action : update_list)delete action;
}

int32 Actions::getIndex(IAction* action)
{
	int32 count = 0;
	for (auto& a : update_list)
	{
		if (a == action)return count;
		++count;
	}
	return -1;
}

IAction* Actions::getAction(const String& id)
{
	for (auto& action : update_list)if (action->id == id)return action;
	//見つからなければ
	return nullptr;
}

IAction* Actions::getAction(int32 index)
{
	if (0 <= index and index <= update_list.size() - 1) return update_list[index];

	return nullptr;
}

int32 Actions::getLoopCount()const
{
	return loopCount;
}

void Actions::restart()
{
	reset();
	start();
}

void Actions::start()
{
	IAction::start();
	//最初のアクションをスタートさせる
	for (auto& action : update_list)
	{
		if ((not action->started) and (not action->ended) and action->startCondition->check())
		{
			action->start();
		}
	}
}

void Actions::reset()
{
	IAction::reset();
	activeNum = update_list.size();
	loopCount = 0;
	for (auto& action : update_list)
	{
		action->reset();
		endTimeList[action] = none;
	}
}

bool Actions::isAllFinished()
{
	return activeNum == 0;
}

void Actions::end()
{
	IAction::end();
	//実行中のアクションは終了させる
	for (auto& action : update_list)
	{
		if (action->isStarted() and not action->isEnded())action->end();
	}

	if (loop)
	{
		int32 temp = ++loopCount;
		reset();
		start();
		loopCount = Min(temp, maxLoopCount);
		return;
	}
	if (init)reset();
}

void Actions::update(double dt)
{
	if (stopped)return;

	dt *= timeScale;
	IAction::update(dt);
	//優先度でソート
	std::stable_sort(update_list.begin(), update_list.end(), [this](const IAction* ac1, const IAction* ac2) {
		return ac1->updatePriority < ac2->updatePriority;
	});

	for (auto& endTime : endTimeList)if (endTime.second.has_value()) *endTime.second += dt;
	
	for (auto& action : update_list)
	{
		if ((not action->started) and (not action->ended) and action->startCondition->check())
		{
			action->start();
		}

		if ((not action->started) or action->ended)continue;

		action->update(dt);

		if (action->endCondition->check())
		{
			action->end();//actionがループアクションの場合ここでisEndedがfalseにならないので下の処理は行わない。
			if (action->isEnded())
			{
				endTimeList[action] = 0;
				activeNum--;
			}
		}
	}
}

