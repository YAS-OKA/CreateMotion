#pragma once

namespace prg
{
	class ConditionArray;

	class Actions;

	class IAction
	{
	public:		
		IAction(double time=0);

		virtual ~IAction();

		void setStartCondition(ConditionArray* condition);

		void setEndCondition(ConditionArray* condition);

		bool isStarted();

		bool isEnded();

		bool isActive();

		double updatePriority;

		double time;

		double timeScale;

		ConditionArray* startCondition = nullptr;
		ConditionArray* endCondition = nullptr;
	protected:
		String id;
		bool started = false;
		bool ended = false;
		friend Actions;

		Actions* container;

		//draw以外のスーパーは呼び出す

		virtual void reset()
		{
			time = 0;
			started = false;
			ended = false;
		}
		virtual void start()
		{
			started = true;
		};
		virtual void end()
		{
			ended = true;
		};

		virtual void update(double dt);
	};

	//class Wait :public IAction
	//{
	//public:
	//	Wait(double time = 0, double timeScale = 1.0, double updatePriority = 1.0, const String& id = U"");
	//	/// @brief waitTargetアクションが終了するまで待つ
	//	Wait(IAction* waitTarget);
	//};

	class MyPrint final :public IAction
	{
	public:
		MyPrint(const String& text,double time=0);

		String text;

	private:
		void update(double dt);
	};
}
