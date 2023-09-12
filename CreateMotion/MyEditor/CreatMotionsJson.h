#pragma once
#include"Entity.hpp"

//次に追加するもの
//一括操作
namespace mj
{
	//ハッシュテーブルのほうがよかった...
	using mp = std::pair<String, Optional<String>>;
	/// @brief jsonの要素を入れる　secondはデフォルト値
	static Array<mp> JsonElems{
		mp{U"Parent",U""},
		mp{U"Position",U"(600,350)"},
		mp{U"RotateCenter",U"(0,0)"},
		mp{U"TexturePath",none},
		mp{U"Z",U"-10"},
		mp{U"Scale",U"1"},
	};
	//画面内にPartsを追加する
	class RegisterParts:public component::Component
	{
	public:
		void start()override;
		void update(double dt)override;

		void addParts(const Array<FilePath>& path);
	private:
		Vec2 pos;
	};

	class LoadJson;
	//m_params[U"Position"]は絶対座標、params(U"Position")は相対座標を返す
	//set_params(U"Position,～)は絶対座標を渡す
	class Parts :public component::Component
	{
	public:
		Parts(const FilePath& path);
		Parts(HashTable<String,String>params);
		void start()override;
		void update(double dt)override;
		void draw()const override;

		void MoveBy(const String& targetParameter,const Vec2& delta);

		void Rotate(double d_rad);

		/*const Circle& getRotateCenterCircle();

		const RectF& get_region();*/

		Vec2 absPos()const;
		//パラメータの取得
		String params(const String& name)const;
		//パーツ名
		String name;
		//パラメータの更新
		void set_params(const String& name, const String& param);

		const String& startPath= U"Characters/";
		//親パーツ
		Parts* PartsParent;

		Vec2 ParentPos;

		Vec2 texCenter() const { return Parse<double>(params(U"Scale")) * tex.size()/ 2; }
		double rad = 0;
		//パーツを描くか
		bool egaku;
	private:
		String path;
		HashTable<String, String>m_params;
		Texture tex;
	};
	class RotateCenter :public component::Component
	{
	public:
		void setParts(Parts* parts);
		bool mouseOver();
		void releaseParts();
		void start()override;
		void update(double dt)override;
		void draw()const override;
	private:
		Circle circle;
		Parts* parts;
	};
	//パーツの当たり判定　透明色の部分をのぞく
	class PartsColliders :public component::Component
	{
	public:
		void makeCollider(Parts* parts,const String& path);
		bool mouseOver(Parts* parts);
		void removeColliderOf(Parts* parts);
		MultiPolygon getCollider(Parts* parts)const;
	private:
		HashTable<Parts*, MultiPolygon> colliders;
	};
	//パーツを動かす
	class MoveParts :public component::Component
	{
	public:
		virtual ~MoveParts() {};
		void start()override;
		virtual void update(double dt)override;
		void select(Parts* parts);
	protected:
		Parts* selectedParts;
	};

	class RotateParts :public component::Component
	{
	public:
		void start()override;
		virtual void update(double dt)override;
		void select(Parts* parts);
	protected:
		Parts* selectedParts;
	};

	class LightUpParts :public component::Component
	{
	public:
		void start()override;
		void update(double dt)override;
		void draw()const override;
		void select(Parts* parts);
	private:
		MultiPolygon hitbox;
		Parts* selectedParts;
		int32 thick;
		ColorF lineColor;
	};

	//RotateCenterを動かす
	class MoveRotateCenter :public MoveParts
	{
	public:	void update(double dt)override;
	};
	//カメラ機能
	class EditorsCamera :public component::Component
	{
	public:
		void start()override;
		void update(double dt)override;

		const Transformer2D getTransformer2D(bool cameraAffected)const;

		double get_x();
		double get_y();
	private:
		double scale_range;
		bool touch_thumb;
		bool touch_bar_thumb;
		double x, y, r;
		RectF bar_thumb;
		RectF scale_bar;
		Circle range;
		Circle thumb;
		Camera2D camera;
		double sensitivity;
		double mouse_sensitivity;

		double _scale_func(double t);
	};
	//パーツのパラメータ編集
	class EditParts :public component::Component
	{
	public:
		void start()override;
		void update(double dt)override;
		//パーツを選択する
		void select(Parts* parts);
		//パーツを解放
		void releaseParts();
		//選択中のパーツを渡す
		Parts* get_selectedParts();
	private:
		Font font;
		Parts* selectedParts;
		int32 x, y, w, h;
		HashTable<String, TextEditState> texts;
		//非表示リスト
		HashSet<String> hiddenList;
	};
	//JSONを出力する
	class SaveParts:public component::Component
	{
	public:
		void start()override;
		void update(double dt)override;
	private:
		Vec2 pos;
	};
	//パーツの削除操作
	class ErasePartsOperate : public component::Component
	{
	public:
		void start()override;
		void update(double dt)override;
	private:
		Vec2 pos1;
		Vec2 pos2;
	};
	class StartMotion : public component::Component
	{
	public:
		void start()override;
		void update(double dt)override;
	private:
		Vec2 pos;
	};
	class LoadJson :public component::Component
	{
	public:
		void start()override;
		void update(double dt)override;
	private:
		void _set_parent(HashTable<String,Parts*> parts, HashTable<String, HashTable<String, String>> JsonTable);
		//__Main__以外をすべて相対座標とみなし、それらの絶対座標を得る。
		Vec2 _getAbsPos(Parts* parts);
		Vec2 pos;
	};

	void updateParentPos(Array<Parts*> parts_list);
}

