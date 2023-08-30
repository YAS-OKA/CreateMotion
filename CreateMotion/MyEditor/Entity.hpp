#pragma once
#include<Siv3D.hpp>

namespace component
{
	class Entity;

	class Priority
	{
		//値が大きい方が先に更新される
		double priority_update= 0;
		//値が大きい方が後に描画される(前に描画される)
		double priority_draw = 0;
	public:
		double getUpdate() const { return priority_update; };
		double getDraw() const { return -priority_draw; };
		void setUpdate(double priority) { priority_update = priority; };
		void setDraw(double priority) { priority_draw = -priority; };
	};

	//コンポーネントの基底クラス
	class Component
	{
	public:
		Component(){}
		virtual ~Component() {}
		virtual void start() {}
		virtual void update(double dt) {}
		virtual void draw()const {}

		//自身を排除する
		template <class T>
		void removeSelf();

		// 親にコンポーネントを追加する
		template<class T>
		T* AddComponent(const String& id, T* component = new T());
		template<class T>
		T* AddComponent(T* component = new T());

		//親のコンポーネントを取得する。
		template<class T>
		T* GetComponent();
		template<class T>
		T* GetComponent(const String& id);
		template<class T>
		Array<T*> GetComponentArr();

		Priority priority;

		Entity* Parent;
		//追加番号　priorityが等しい場合、これをもとにソートする
		int32 index;
		
	};

	//コンポーネントを適用した基底クラス
	//HashTableを使っているので計算量は基本O(1)？
	//同一の型のコンポーネントを追加することは可能　その場合idを指定しないとGetComponentはできない（バグ防止）
	//型が重複してるならGetComponentArrを使うことを推奨
	class Entity
	{
	public:
		Entity(){}

		virtual ~Entity()
		{
			for (auto& multi_comp : components)
			{
				for (auto& comp : multi_comp.second)
				{
					if(comp.second!=nullptr)delete comp.second;
				}
			}

			components.clear();
		};

		//コンポーネントをアップデート
		virtual void update_components(double dt)
		{
			//キャッシュからコンポーネントを追加　メモリリークはないはず
			for (auto& com_ls : Cash)
			{
				for (auto& com : com_ls.second)
				{
					components[com_ls.first][com.first] = com.second;
				}
				com_ls.second.clear();
			}
			Cash.clear();

			//allComponentをセット
			allComponentForUpdate.clear();
			allComponentForDraw.clear();
			for (const auto& com_ls : components)
			{
				for (const auto& com : com_ls.second)
				{
					allComponentForUpdate << com.second;
					allComponentForDraw << com.second;
				}
			}
			std::sort(allComponentForUpdate.begin(), allComponentForUpdate.end(), [this](const Component* com1, const Component* com2) {return _replace_flag(true, com1, com2);});
			std::sort(allComponentForDraw.begin(), allComponentForDraw.end(), [this](const Component* com1, const Component* com2) {return _replace_flag(false, com1, com2);});

			for (auto& com : allComponentForUpdate)
				com->update(dt);
		};

		//コンポーネントをdraw
		virtual void draw_components() const
		{
			for (const auto& com : allComponentForDraw)
				com->draw();
		};

		const String& get_id() const { return id; }

		const String& get_name() const { return name; }
		//一致するコンポーネントを削除
		template<class T>
		void remove(Component* com)
		{
			if (not components.contains(typeid(T).hash_code()))return;
			Optional<String> id=none;
			for (const auto& component : components[typeid(T).hash_code()])
			{
				if (component.second == com)id = component.first;
			}
			if (not id.has_value())return;
			remove<T>(*id);
		}
		//コンポーネントの削除 コンポーネントの型が重複している場合下のif文でid指定で消す
		template<class T>
		void remove(String id = U"")
		{
			if (components[typeid(T).hash_code()].size() <= 1)
			{
				components.erase(typeid(T).hash_code());
				if (ids.contains(typeid(T).hash_code()))ids[typeid(T).hash_code()] -= 1;
				return;
			}
			if (components[typeid(T).hash_code()].contains(id))
			{
				components[typeid(T).hash_code()].erase(id);
				if (ids.contains(typeid(T).hash_code()))ids[typeid(T).hash_code()] -= 1;
			}
		}

		//コンポーネントの追加　idがかぶったら上書き
		template<class T>
		T* AddComponent(const String& id, T* component = new T())
		{
			if (component->Parent == NULL)component->Parent = this;
			component->start();
			component->index = 0;
			for (const auto& id : ids)
			{
				component->index += id.second;
			}

			components[typeid(T).hash_code()][id] = component;
			return component;
		}
		//コンポーネントの追加　hash_codeなので異なるクラスならidはたぶんかぶらない
		template<class T>
		T* AddComponent(T* component = new T())
		{
			if (component->Parent == NULL)component->Parent = this;
			component->start();
			component->index = 0;
			for (const auto& id : ids)
			{
				component->index += id.second;
			}
			if (not ids.contains(typeid(T).hash_code()))
			{
				ids[typeid(T).hash_code()] = 0;
			}
			else
			{
				ids[typeid(T).hash_code()] += 1;
			}

			components[typeid(T).hash_code()][Format(ids[typeid(T).hash_code()])] = component;
			return component;
		}

		//コンポーネントの取得　型の重複はなし
		template<class T>
		T* GetComponent()
		{
			if (not components.contains(typeid(T).hash_code()))
			{
				return nullptr;
			}
			//型の重複がある場合エラー
			if (components[typeid(T).hash_code()].size() > 1)
			{
				throw Error{ U"idを指定してください。\n重複:" + Format(typeid(T).hash_code()) };

				return nullptr;
			}
			return static_cast<T*>(components[typeid(T).hash_code()].begin()->second);
		}

		//コンポーネントの取得 id指定。使いどころは複数のコンポーネントが同一の型で重複しているときとか。
		template<class T>
		T* GetComponent(const String& id)
		{
			if (not components.contains(typeid(T).hash_code()))
			{
				return nullptr;
			}

			if (not components[typeid(T).hash_code()].contains(id))
			{
				return nullptr;
			}

			return static_cast<T*>(components[typeid(T).hash_code()][id]);
		}

		template<class T>
		Array<T*> GetComponentArr(bool sort_by_priority_of_update = true)
		{
			Array<T*> arr;

			if (not components.contains(typeid(T).hash_code()))
				return arr;

			for (const auto& h : components[typeid(T).hash_code()])
			{
				arr << static_cast<T*>(h.second);
			}
			
			if (sort_by_priority_of_update) {
				std::sort(arr.begin(), arr.end(), [this](const T* com1, const T* com2) {return _replace_flag(true,com1,com2); });
			}else{
				std::sort(arr.begin(), arr.end(), [this](const T* com1, const T* com2) {return _replace_flag(false, com1, com2); });
				arr.reverse();
			}
			return arr;
		}

	protected:
		String id = U"entity";

		String name = U"entity";

	private:
		HashTable<size_t, HashTable<String, Component*>> components;
		//iすべてのコンポーネントをここにぶち込む priorityでソートするため
		Array<Component*> allComponentForUpdate;
		Array<Component*> allComponentForDraw;

		bool _replace_flag(bool update_priority, const Component* s,const Component* other)
		{
			if (update_priority) {
				if (s->priority.getUpdate() != other->priority.getUpdate())return s->priority.getUpdate() < other->priority.getUpdate();
			}
			else {
				if (s->priority.getDraw() != other->priority.getDraw())return s->priority.getDraw() < other->priority.getDraw();
			}
			return s->index < other->index;
		};

		HashTable<size_t, size_t>ids;

		HashTable<size_t, HashTable<String, Component*>> Cash;


		template<class T>
		friend T* Component::AddComponent(const String& id, T* component);

		template<class T>
		friend T* Component::AddComponent(T* component);
	};

	template <class T>
	void Component::removeSelf()
	{
		Parent->remove<T>(this);
	};

	template<class T>
	T* Component::AddComponent(const String& id, T* component)
	{
		if (component->Parent == NULL)component->Parent = Parent;
		component->start();
		component->index = 0;
		for (const auto& com_ls : Parent->components)
		{
			component->index += com_ls.second.size();
		}
		for (const auto& com_ls : Parent->Cash)
		{
			component->index += com_ls.second.size();
		}

		Parent->Cash[typeid(T).hash_code()][id] = component;
		return component;
	};

	template<class T>
	T* Component::AddComponent(T* component)
	{
		if (component->Parent == NULL)component->Parent = Parent;
		component->start();
		component->index = 0;
		for (const auto& com_ls : Parent->components)
		{
			component->index += com_ls.second.size();
		}
		for (const auto& com_ls : Parent->Cash)
		{
			component->index += com_ls.second.size();
		}

		if (not Parent->ids.contains(typeid(T).hash_code()))
		{
			Parent->ids[typeid(T).hash_code()] = 0;
		}
		else
		{
			Parent->ids[typeid(T).hash_code()] += 1;
		}

		Parent->Cash[typeid(T).hash_code()][Format(Parent->ids[typeid(T).hash_code()])] = component;
		return component;
	};

	template<class T>
	T* Component::GetComponent()
	{
		return Parent->GetComponent<T>();
	}

	template<class T>
	T* Component::GetComponent(const String& id)
	{
		return Parent->GetComponent<T>(id);
	}

	template<class T>
	Array<T*> Component::GetComponentArr()
	{
		return Parent->GetComponentArr<T>();
	}
}
