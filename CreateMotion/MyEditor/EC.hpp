#pragma once
#include<Siv3D.hpp>

class Entity;

class Priority
{
	//値が大きい方が先に更新される
	double priority = 0;
public:
	double getPriority() const { return priority; };
	void setPriority(double priority) { Priority::priority = priority; };
};

//コンポーネントの基底クラス
class Component
{
public:
	Component() {}
	virtual ~Component() {}
	virtual void start() {}
	virtual void update(double dt) {}

	Priority priority;

	Entity* owner = nullptr;
	//追加番号　priorityが等しい場合、これをもとにソート
	int32 index;
};

class EntityManager;
//コンポーネントを適用した基底クラス
//HashTableを使っているので計算量は基本O(1)？
//同一の型のコンポーネントを追加することは可能　その場合idを指定しないとGetComponentはできない（バグ防止）
//型が重複してるならGetComponentArrを使うことを推奨
class Entity
{
public:
	Entity() {}

	virtual ~Entity()
	{
		//削除されず残ったコンポーネントのポインタを削除
		deleteCash();
		deleteComponents();
		deleteGarbages();
	};

	void setManager(EntityManager* m)
	{
		manager = m;
	}

	virtual void update(double dt) {};

	//コンポーネントをアップデート
	virtual void update_components(double dt)
	{
		if (garbages.size() > 0)deleteGarbages();
		//コンポーネントを追加
		for (auto& com_ls : cache)
		{
			if (com_ls.second.empty())continue;
			for (auto& com : com_ls.second)
			{
				com.second->start();
				components[com_ls.first][com.first] = com.second;
				allComponentForUpdate << com.second;
				componentsNum++;
			}
		}
		cache.clear();
		cashNum = 0;

		std::stable_sort(allComponentForUpdate.begin(), allComponentForUpdate.end(), [=](const Component* com1, const Component* com2) {return _replace_flag(com1, com2); });

		for (auto& com : allComponentForUpdate)com->update(dt);
	};

	//一致するコンポーネントを削除 下のオーバーロードの関数を呼び出す
	template<class T>
	void remove(T* com)
	{
		Optional<String> id = _get_id<T>(com);
		if (not id.has_value())return;
		remove<T>(*id);
	}
	//コンポーネントの削除 コンポーネントの型が重複している場合下のif文でid指定で消す 削除したコンポーネントはgarbagesへ
	template<class T>
	void remove(const String& id = U"")
	{
		if (not components.contains(typeid(T)))return;

		if (components[typeid(T)].contains(id))
		{
			allComponentForUpdate.remove(components[typeid(T)][id]);
			garbages << components[typeid(T)][id];
			components[typeid(T)].erase(id);
			if (ids.contains(typeid(T)))ids[typeid(T)] -= 1;
			//空だったらキーもクリア
			if (components[typeid(T)].empty())components.erase(typeid(T));
			--componentsNum;
		}
		else if(id==U"")
		{//全ぶ消す
			for (auto& com : components[typeid(T)]) {
				garbages << com.second;
				allComponentForUpdate.remove(com.second);
			}
			components.erase(typeid(T));
			if (ids.contains(typeid(T)))ids[typeid(T)] -= 1;
			--componentsNum;
		}

	}

	//コンポーネントの追加　idがかぶったら上書き
	template<class T, class... Args>
	T* addComponentNamed(const String& id, Args&&... args)
	{
		auto component = new T(args...);
		component->owner = this;
		component->start();
		component->index = componentsNum;

		components[typeid(T)][id] = component;
		componentsNum++;
		allComponentForUpdate << component;
		return component;
	}
	//コンポーネントの追加
	template<class T,class... Args>
	T* addComponent(Args&&... args)
	{
		auto component = new T(args...);
		component->owner= this;
		component->start();
		component->index = componentsNum;

		if (not ids.contains(typeid(T)))
		{
			ids[typeid(T)] = 0;
		}
		else
		{
			ids[typeid(T)] += 1;
		}
		components[typeid(T)][Format(ids[typeid(T)])] = component;
		componentsNum++;
		allComponentForUpdate << component;
		return component;
	}
	
	//コンポーネントの取得　型の重複はなし
	template<class T>
	T* getComponent()
	{
		if (not components.contains(typeid(T)))
		{
			return nullptr;
		}
		//型の重複がある場合エラー
		if (components[typeid(T)].size() > 1)
		{
			throw Error{ U"idを指定してください。\n重複:" + Format(typeid(T).hash_code()) };
		}
		return static_cast<T*>(components[typeid(T)].begin()->second);
	}

	template<class T>
	const T* getComponent() const
	{
		if (not components.contains(typeid(T)))
		{
			return nullptr;
		}
		//型の重複がある場合エラー
		if (components.at(typeid(T)).size() > 1)
		{
			throw Error{ U"idを指定してください。\n重複:" + Format(typeid(T).name) };
		}
		return static_cast<T*>(components.at(typeid(T)).begin()->second);
	}

	//コンポーネントの取得 id指定。使いどころは複数のコンポーネントが同一の型で重複しているときとか。
	template<class T>
	T* getComponent(const String& id)
	{
		if (not components.contains(typeid(T)))
		{
			return nullptr;
		}

		if (not components[typeid(T)].contains(id))
		{
			return nullptr;
		}

		return static_cast<T*>(components[typeid(T)][id]);
	}

	template<class T>
	Array<T*> getComponentArray()
	{
		Array<T*> arr;

		if (not components.contains(typeid(T)))
			return arr;

		for (const auto& h : components[typeid(T)])
		{
			arr << static_cast<T*>(h.second);
		}

		std::sort(arr.begin(), arr.end(), [this](const T* com1, const T* com2) {return _replace_flag(com1, com2); });
		
		return arr;
	}

	String name = U"";

	Priority priority;
private:
	int32 componentsNum = 0;
	int32 cashNum = 0;
	HashTable<std::type_index, HashTable<String, Component*>> components;
	//すべてのコンポーネントをここにぶち込む priorityでソートするため
	//また、これらが持つポインタはcomponentsが持つポインタを指すのでdeleteComponents()でメモリが解放される。
	//わざわざdeleteする必要はない。
	Array<Component*> allComponentForUpdate;
	//コンポーネントのガーベージコレクション
	Array<Component*> garbages;
	//コンポーネントを1fためておく
	HashTable<std::type_index, HashTable<String, Component*>> cache;

	HashTable<std::type_index, size_t>ids;

	EntityManager* manager;

	//メモリの解放
	void deleteComponents()
	{
		for (auto& multi_comp : components)
		{
			for (auto& comp : multi_comp.second)
			{
				delete comp.second;
			}
		}
		components.clear();
	};
	void deleteCash()
	{
		for (auto& cash_ls : cache)
		{
			for (auto& com : cash_ls.second)if (com.second != nullptr)delete com.second;
			cash_ls.second.clear();
		}
		cache.clear();
	}
	void deleteGarbages()
	{
		//ガーベージをクリア
		for (auto& garbage : garbages)
		{
			if (garbage != nullptr)delete garbage;
		}
		garbages.clear();
	};
	//優先度で入れ替えを行うかどうか
	bool _replace_flag(const Component* s, const Component* other)
	{
		if (s->priority.getPriority() != other->priority.getPriority())return s->priority.getPriority() > other->priority.getPriority();
		return s->index < other->index;
	};
	
	template<class T>
	Optional<String> _get_id(Component* com)
	{
		if (not components.contains(typeid(T)))return none;
		for (const auto& component : components[typeid(T)])
		{
			if (component.second == com)
			{
				return component.first;
			}
		}
		return none;
	}
};

class EntityManager final {
private:
	Array<std::pair<std::type_index,Entity*>> entitys;
	Array<Entity*> garbages;
	Array<std::pair<std::type_index,Entity*>> cache;
public:
	~EntityManager() {
		clean();
		for (auto& ent : garbages)delete ent;
		garbages.clear();
	}

	void update(double dt) {
		//消去
		if (not garbages.isEmpty()) {
			for (auto& ent : garbages)delete ent;
			garbages.clear();
		}
		//追加
		if (not cache.isEmpty()) {
			entitys.append(cache);
			cache.clear();
		}
		//ソート
		std::stable_sort(
			entitys.begin(),
			entitys.end(),
			[=](const auto ent1, const auto ent2) {return ent1.second->priority.getPriority() > ent2.second->priority.getPriority(); }
		);
		//更新
		for (auto& entity : entitys){
			entity.second->update(dt);
			entity.second->update_components(dt);
		}
	}

	Array<Entity*> allEntitys() {
		Array<Entity*> result;
		for (auto& ent : entitys)result << ent.second;
		return result;
	};

	template<class T>
	Array<T*> find(const Optional<String>& name=none)
	{
		Array<T*> result;
		std::type_index info = typeid(T);
		for (auto& ent : entitys)
		{
			if (ent.first == info)
			{
				//名前が指定されていない場合
				if (not name)result << static_cast<T*>(ent);
				//名前が指定されている場合
				else if (name == ent.second->name)result << static_cast<T*>(ent);
			}
		}
		return result;
	}

	template<class T>
	Entity* findOne(const Optional<String>& name=none)
	{
		auto ent = find<T>(name);
		if (ent.isEmpty())return nullptr;
		else return ent[0];
	}

	//Entityを作る
	template<class T = Entity, class... Args>
	T* birth(Args&&... args) {
		auto entity = new T(args...);
		cache << std::pair<std::type_index,T*>(typeid(T), entity);
		entity->setManager(this);
		return entity;
	}
	//すべてガーベージに
	void clean()
	{
		for (auto& ent : entitys)garbages << ent.second;
		for (auto& ent : cache)garbages << ent.second;
		entitys.clear();
		cache.clear();
	}
	//一致するEntityをgarbagesへ
	void kill(Entity* ent) {
		for (auto it=entitys.begin(),en=entitys.end();it!=en;)
		{
			if (ent == (*it).second) {
				garbages << ent;
				it = entitys.erase(it);
			}
			else {
				++it;
			}
		}
	}
	//名前が一致するEntityをgarbagesへ
	void kill(const String& name)
	{
		for (auto it = entitys.begin(), en = entitys.end(); it != en;)
		{
			if ((*it).second->name==name) {
				garbages << (*it).second;
				it = entitys.erase(it);
			}
			else {
				++it;
			}
		}
	}
};
