#pragma once
#include "Action.h"
#include"Condition.h"

namespace prg
{
	class Actions final:public IAction
	{
	public:
		~Actions();

		template<class T,int32 Index=-1,class... Args>
		T* add(Args&&... args)
		{
			auto action = new T(args...);

			endTimeList[action] = none;

			action->startCondition->add(new FuncCondition(
				[=] {
					auto preAction = getAction(getIndex(action) - 1);
					if (preAction == nullptr)return true;
					return preAction->isEnded();
				}
			));

			int32 index = Index == -1 or update_list.size() - 1 < Index ?
				update_list.size() : Index;

			update_list.insert(std::next(update_list.begin(), index), action);

			action->container = this;

			activeNum++;

			return action;
		}

		template<class T, int32 Index = -1, class... Args>
		T* addParallel(Args&&... args)
		{
			auto action = new T(args...);

			endTimeList[action] = none;

			action->startCondition->add(new FuncCondition(
				[=] {
					auto preAction = getAction(getIndex(action) - 1);
					if (preAction == nullptr)return true;
					return preAction->isStarted();
				}
			));

			int32 index = Index == -1 or update_list.size() - 1 < Index ?
				update_list.size() : Index;

			update_list.insert(std::next(update_list.begin(), index), action);

			action->container = this;

			activeNum++;

			return action;
		}

		int32 getIndex(IAction* action);

		IAction* getAction(const String& id);

		IAction* getAction(int32 index);

		int32 getLoopCount()const;
		/// @brief アクションをリセットしてから開始する
		void restart();
		/// @brief アクションを開始する
		void start()override;
		/// @brief アクションをリセット
		void reset()override;

		bool isAllFinished();

		void update(double dt)override;
		//終了したときにリセットを行う
		bool init = false;

		bool loop = false;

		bool stopped = false;
		//ループカウントの上限　別にカンストしてもループは続く
		const int32 maxLoopCount = 10000;
	private:
		void end()override;

		//void end()override;
		//終了していないIActionがいくつあるか addしたときに++ updateでendのとき-- resetのとき=
		int32 activeNum = 0;
		//secondにはアクションが終わってからの経過時間を入れる
		HashTable<IAction*,Optional<double>> endTimeList;
		//HashTableにしてendTimeListの役割も兼ねるようにしたかったけど,addは.backを使うのと,HashTableのソートがうまく機能するのかわからなかったのでやめた。
		Array<IAction*> update_list;
		Array<IAction*> draw_list;

		int32 loopCount = 0;
	};

	
}
