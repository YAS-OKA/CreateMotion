#pragma once
#include"Entity.hpp"

namespace mj
{
	using mp = std::pair<String, Optional<String>>;
	/// @brief jsonの要素を入れる　secondはデフォルト値
	static Array<mp> JsonElems{
		mp{U"Parent",U""},
		mp{U"Position",U"(400,300)"},
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
	
	class Parts :public component::Component
	{
	public:
		Parts(const FilePath& path);
		void update(double dt)override;
		void draw()const override;

		void MoveBy(const String& targetParameter,const Vec2& delta);

		const Circle& getRotateCenterCircle();

		const RectF& get_region();

		Vec2 absPos()const;
		//パラメータの取得
		String params(const String& name)const;
		//パーツ名
		const String name;
		//パラメータの更新
		void set_params(const String& name, const String& param);

		const String& startPath= U"CharacterImages/";
		//親パーツ
		Parts* PartsParent;
	private:
		Vec2 ParentPos;
		HashTable<String, String>m_params;
		Texture tex;
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
		Vec2 pos;
	};
	class StartMotion : public component::Component
	{
	public:
		void start()override;
		void update(double dt)override;
	private:
		Vec2 pos;
	};
}

