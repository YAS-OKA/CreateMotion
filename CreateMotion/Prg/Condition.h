#pragma once

namespace prg
{
	class ICondition
	{
	public:
		ICondition();
		virtual ~ICondition() {};
		void fire();

		virtual bool check();

	protected:
		bool flag;
	};

	enum class Type {
		Every,
		Any
	};

	class ConditionArray:public ICondition
	{
	public:
		virtual ~ConditionArray() {}

		virtual bool check();

		void add(ICondition* condition);

		Array<ICondition*> conditionArray;
		
		//Every すべてのconditionがtrueならtrue
		//Any いずれかのconditionがtrueならtrue
		Type checkType=Type::Any;
	};

	//関数でチェック
	template <class Fty, std::enable_if_t<std::is_invocable_r_v<bool, Fty>>* = nullptr>
	class FuncCondition :public ICondition
	{
	public:
		//ラムダ式などで関数を渡す　関数の戻り値はbool
		FuncCondition(Fty _function) :m_function{ _function } {}

		bool check()
		{
			return flag or m_function();
		};
	private:
		const Fty m_function;
	};

}
