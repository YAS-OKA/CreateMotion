﻿# include <Siv3D.hpp> // OpenSiv3D v0.6.6
#include"AnimationSystem.h"
#include"MyEditor/__init__.h"

//設定した角度回転
class Rotate :public TimeMove {
public:
	double rad;
	String target;

	Rotate(const String target, double rad, double timelim) :TimeMove{ timelim }, target{ target }, rad{ rad } {}

	virtual ~Rotate() {}

	void update(Character* character, double dt = Scene::DeltaTime())override {
		dt = calTime(dt);
		character->get(target)->angle += dt * rad / timelim;
	}
};
//0~360
double seikika(double theta) {
	if (0 <= theta)return Fmod(theta, 360_deg);//正規化
	else return 360_deg + Fmod(theta, 360_deg);
}
/// @brief 符号判定
/// @param A 
/// @return Aが負なら-1　Aが正なら+1
double sign(double A) {
	return (A > 0) - (A < 0);
}

class RotateTo :public Rotate {
	int32 direction = 0;
public:
	RotateTo(const String& target, double rad, double timelim, bool clockWise) :Rotate{ target,rad,timelim }
	{
		rad = seikika(rad);
		if (clockWise)direction = 1; else direction = -1;
	}
	RotateTo(const String& target, double rad, double timelim) :Rotate{ target,rad,timelim }
	{
		rad = seikika(rad);
	}
	void start(Character* character)override {
		//character->get(target)->angle = rad;
		if(timelim==0)character->get(target)->angle = rad;

		double delta = rad - seikika(character->get(target)->angle);
		if (direction == 0)
		{
			//rad = rad - seikika(character->get(target)->angle);
			abs(delta) > 180_deg ? direction = sign(delta) * (-1) : direction = sign(delta);
		}

		if (direction > 0)
		{
			//時計回り
			delta < 0 ? rad = 360_deg + delta : rad = delta;
		}
		else
		{
			//反時計回り
			delta > 0 ? rad = delta - 360_deg : rad = delta;
		}
				
	}
	//Rotateのupdateを使う
};
/// @brief 等速直線で移動する
class Translate :public TimeMove
{
public:
	Vec2 dp;
	String target;

	Translate(const String& target, Vec2 deltaPos, double time)
		:TimeMove{ time }, dp(deltaPos) {};

	void start(Character* character)override
	{
		if (timelim == 0)
		{
			character->get(target)->pos += dp;
			return;
		}
	}

	void update(Character* character, double dt = Scene::DeltaTime())override {
		dt = calTime(dt);
		if (timelim == 0)return;
		character->get(target)->pos += dt * dp / timelim;
	}
};

class Mirror :public TimeMove {
public:
	double rate;
	int32 mirror;
	bool trueMirror = false;
	/// @brief 
	/// @param mirror -1左　0反転　1右
	/// @param rate 
	Mirror(int32 mirror, double rate = 360_deg) :TimeMove{ 180_deg / rate }, rate{ rate }, mirror{ mirror } {
	}

	void start(Character* character)override {
		TimeMove::start(character);

		if (mirror == 0) {
			character->mirror = not character->mirror;
			trueMirror = not character->mirror;
		}
		else if (0 < mirror) {
			character->mirror = false;
			trueMirror = false;
		}
		else {
			character->mirror = true;
			trueMirror = true;
		}
	}

	void update(Character* character, double dt = Scene::DeltaTime())override {
		dt = calTime(dt);
		if (trueMirror) {
			character->angle += dt * rate;
		}
		else {
			character->angle -= dt * rate;
		}
	}
};

//設定した秒数待つ
class Wait :public TimeMove {
public:
	Wait(double timelim) :TimeMove{ timelim } {}

	void update(Character* character, double dt = Scene::DeltaTime())override {
		dt = calTime(dt);
	}
};

class MotionLoader {
public:

	CSV csv;
	HashTable<String, std::function<Move* (const Array<String>&)>>moveResolver;

	MotionLoader(const CSV& csv) :csv{ csv } {
		resetResolver();

	}

	void resetResolver() {
		moveResolver[U"Rotate"] = [](const Array<String>& list) {return new Rotate{ list[1],Parse<double>(list[2]) * 1_deg,Parse<double>(list[3]) }; };
		moveResolver[U"Wait"] = [](const Array<String>& list) {return new Wait{ Parse<double>(list[1]) }; };
		moveResolver[U"RotateTo"] = [](const Array<String>& list) {
			if (list.size() <= 3)return new RotateTo(list[1], Parse<double>(list[2]) * 1_deg, 0);//0秒で指定した角度へ
			else if (list.size() <= 4)return new RotateTo(list[1], Parse<double>(list[2]) * 1_deg, Parse<double>(list[3]));//指定した秒数で指定した角度へ
			else return new RotateTo(list[1], Parse<double>(list[2]) * 1_deg, Parse<double>(list[3]), Parse<bool>(list[4]));//指定した秒数で指定された方向に指定された角度へ
		};
		//現在地から指定した分の移動
		moveResolver[U"Move"] = [](const Array<String>& list) {return new Translate(list[1], Vec2{ Parse<double>(list[2]),Parse<double>(list[3]) }, Parse<double>(list[4])); };

	}

	Motion LoadMotion(String region = U"") {

		Motion motion;

		CSV tmpCsv = csv;

		size_t index = 0;
		if (region != U"") {
			for (size_t row = 0; row < tmpCsv.rows(); ++row)
			{
				if ((tmpCsv.columns(row) != 0) && tmpCsv[row][0] == U"#{}"_fmt(region)) {
					index = row + 1;
					break;
				}
			}
		}

		HashTable<String, String>variableTable;

		Motion* tmpMotion = new Motion{ true };

		bool addFlg = false;
		//ここ-1にしなくていいと思うんだよなぁ
		for (size_t row = index; row < tmpCsv.rows(); ++row)
		{
			if (tmpCsv.columns(row) == 0) {
				if (addFlg) {
					motion.add(tmpMotion);
					tmpMotion = new Motion{ true };
					addFlg = false;
				}
			}
			else if (tmpCsv[row][0][0] == U'#') {
				break;
			}
			else if (tmpCsv[row][0][0] == U'@') {
				variableTable[tmpCsv[row][0]] = tmpCsv[row][1];
			}
			else {
				for (size_t col = 0; col < tmpCsv.columns(row); ++col) {

					if (variableTable.contains(tmpCsv[row][col])) {
						tmpCsv[row][col] = variableTable[tmpCsv[row][col]];
					}
				}
				tmpMotion->add(moveResolver[tmpCsv[row][0]](tmpCsv[row]));
				addFlg = true;
			}
		}
		//あとで考える
		if (addFlg) {
			motion.add(tmpMotion);
		}
		else delete tmpMotion;

		return motion;
	}
};

bool MotionSelect(String ButtonMessage, Vec2 pos, Optional<String>& Path, FileFilter filter)
{
	bool flag=false;
	if (SimpleGUI::Button(ButtonMessage, pos))
	{
		flag = true;
		Path = Dialog::OpenFile({ filter });
		if (not Path)
		{
			return false;
		}
	}
	return flag;
}

void Main()
{
	MyEditor editor;
	//==========体の用意==========
	Character character{ JSON::Load(U""),2 };

	MotionLoader loader{ CSV{U""} };

	Motion motion = loader.LoadMotion();

	//==========描画に追加==========
	DrawManager manager;

	Optional<String> JsonPath;
	Optional<String> MotionPath;

	mj::EditorsCamera camera;
	camera.start();

	double UiStartY = 90;
	bool drawFlg = true, debugFlg = false;
	bool jsonOk = false, motionOk = false;
	bool touchGround = false;
	TextEditState text;
	while (System::Update())
	{
		jsonOk = character.joint != nullptr;
		motionOk = MotionPath.has_value();

		if (editor.working) {
			editor.update();
			editor.draw();
			continue;
		}

		if (SimpleGUI::Button(U"モデルエディタ画面へ", { 10,Scene::Height()-50 }))
		{
			editor.working = true;
		}
		if (MotionSelect(U"モデル読み込み", {10,10}, JsonPath,FileFilter::JSON()))
		{
			//モーションの画面に初めて移ったときの処理
			if(JsonPath)character = Character{ JSON::Load(*JsonPath),2 };
			character.setDrawManager(&manager);
		}
		if (MotionSelect(U"スクリプト", {10,50}, MotionPath,FileFilter::Text()))
		{
			if (MotionPath)loader = MotionLoader{ CSV{*MotionPath} };
			motion = loader.LoadMotion();
		}

		SimpleGUI::CheckBox(drawFlg, U"通常表示", Vec2{ 10, UiStartY+50 }, 160);
		SimpleGUI::CheckBox(debugFlg, U"デバッグ表示", Vec2{ 10, UiStartY+90 }, 160);
		SimpleGUI::CheckBox(touchGround, U"地面に着かせる", Vec2{ 10,UiStartY + 130 }, 180);
		SimpleGUI::TextBox(text, Vec2{ 100, UiStartY });

		if (SimpleGUI::Button(U"実行", Vec2{ 10, UiStartY },unspecified,jsonOk and motionOk)) {
			character.motions.clear();
			MotionLoader loader{ CSV{U"motion.txt"} };
			character.addMotion(loader.LoadMotion(text.text));
		}

		{
			if(touchGround)RectF{ 0,500,800,100 }.draw(Palette::Green);

			const auto t = camera.getTransformer2D(true);

			if (jsonOk) {
				if (touchGround)character.touchGround(500);
				character.update();

				manager.update();
				if (drawFlg)manager.draw();
				if (debugFlg)manager.drawDebug();
			}
		}
		camera.update(Scene::DeltaTime());
	}
}
