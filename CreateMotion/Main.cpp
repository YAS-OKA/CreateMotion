# include <Siv3D.hpp> // OpenSiv3D v0.6.6
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
		if (character->get(target) == nullptr)return;
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
		if (character->get(target) == nullptr)return;
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
		:TimeMove{ time },target(target), dp(deltaPos)
	{

	};

	void start(Character* character)override
	{
		TimeMove::start(character);
		if (timelim == 0)
		{
			character->get(target)->pos += dp;
			return;
		}
		//character->get(target)->pos = { 100,100 };
	}

	void update(Character* character, double dt = Scene::DeltaTime())override {
		dt = calTime(dt);
		if (timelim == 0)return;
		//rint << character->get(target)->pos;
		if (character->get(target) == nullptr)return;
		character->get(target)->pos += dt * dp / timelim;
	}
};

class SetPos :public Translate
{
public:
	Vec2 pos;
	SetPos(const String& target,Vec2 pos, double time)
		:Translate(target, { 0,0 }, time),pos(pos)
	{}

	void start(Character* character)override
	{
		TimeMove::start(character);
		if (timelim == 0)
		{
			character->get(target)->pos = pos;
			return;
		}
		dp = pos - character->get(target)->pos;
	}
};

class ChangeTexture :public TimeMove
{
public:
	String path;
	String target;

	ChangeTexture(const String& target, const String& path, double time = 0)
		:TimeMove(time), target(target),path(path)
	{
		TextureAsset::Register(path, path);
	}

	void start(Character* character)override
	{
		TimeMove::start(character);
		if (timelim == 0)
		{
			character->get(target)->textureName = path;
		}
	}

	void update(Character* character, double dt = Scene::DeltaTime())override
	{
		dt = calTime(dt);
		if(not isActive())character->get(target)->textureName = path;
	}
};

class ChangeColor :public TimeMove
{
public:
	String target;
	ColorF color;
	HashTable<Joint*,ColorF> start_color;
	bool following;
	ChangeColor(const String& target, const ColorF& color, double time, bool following = true)
		:TimeMove(time),color(color),target(target),following(following)
	{}

	//ChangeColor(const String& target,String )

	void start(Character* character)override
	{
		TimeMove::start(character);

		Array<Joint*> targets;
		if (following)
		{
			targets=character->get(target)->getAll();
		}
		else
		{
			targets << character->get(target);
		}
		for (auto& joint : targets)
		{
			start_color.emplace(joint,joint->color);
		}
	}

	void update(Character* character, double dt = Scene::DeltaTime())override
	{
		dt = calTime(dt);
		for (auto&  joint_color: start_color)
		{
			joint_color.first->color = joint_color.second.lerp(color, time / timelim);
		}
	}
};

class ChangeScale :public TimeMove
{
public:
	String target;
	SizeF scale;
	HashTable<Joint*, SizeF> d_scale;
	HashTable<Joint*, SizeF> d_position;
	HashTable<Joint*, SizeF> d_center;

	bool following;
	ChangeScale(const String& target, double x_scale,double y_scale, double time, bool following = true)
		:TimeMove(time), scale(SizeF{x_scale,y_scale}), target(target), following(following)
	{}

	//ChangeColor(const String& target,String )

	void start(Character* character)override
	{
		TimeMove::start(character);

		Array<Joint*> targets;
		if (following)
		{
			targets = character->get(target)->getAll();
		}
		else
		{
			targets << character->get(target);
		}
		bool flag = false;
		for (auto& joint : targets)
		{
			d_scale.emplace(joint, joint->size * scale - joint->size);
			d_center.emplace(joint, joint->rotatePos * scale - joint->rotatePos);
			d_position.emplace(joint, joint->pos*scale-joint->pos);
			if (not flag)
			{
				flag = true;

			}
		}
		
		if (targets[0] == character->joint)
		{
			d_position[targets[0]] = ( - targets[0]->rotatePos)*scale;
		}
		else
		{
			//d_position[targets[0]] = { 0,0 };
			d_position[targets[0]] = targets[0]->rotatePos - targets[0]->rotatePos * scale;
		}

		if(timelim==0)
		{
			for (auto& joint_scale : d_scale)
			{
				joint_scale.first->size += joint_scale.second;
			}
			for (auto& joint_position : d_position)
			{
				joint_position.first->pos += joint_position.second;
			}
			for (auto& joint_center : d_center)
			{
				joint_center.first->rotatePos += joint_center.second;
			}
		}
	}

	void update(Character* character, double dt = Scene::DeltaTime())override
	{
		dt = calTime(dt);
		if (timelim == 0)return;
		for (auto& joint_scale : d_scale)
		{
			joint_scale.first->size += ( dt / timelim) * joint_scale.second;
		}
		for (auto& joint_scale : d_position)
		{
			joint_scale.first->pos += (dt / timelim) * joint_scale.second;
		}
		for (auto& joint_scale : d_center)
		{
			joint_scale.first->rotatePos += (dt / timelim) * joint_scale.second;
		}
	}
};



//class SetZ :public Move
//{
//public:
//	double z;
//	String target;
//	SetZ(const String& target, double z, bool following = true)
//		:z(z), target(target), following(following)
//	{}
//};

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
		moveResolver[U"MoveTo"] = [](const Array<String>& list) {return new SetPos(list[1], Vec2{ Parse<double>(list[2]),Parse<double>(list[3]) }, Parse<double>(list[4])); };
		moveResolver[U"ChangeTexture"] = [](const Array<String>& list) {
			if (list.size() < 4)return new ChangeTexture(list[1], list[2]);
			else return new ChangeTexture(list[1], list[2], Parse<double>(list[3]));
		};
		moveResolver[U"ChangeColor"] = [](const Array<String>& list){
			if(list.size()<=7)return new ChangeColor{ list[1], ColorF{ Parse<double>(list[2]),Parse<double>(list[3]),Parse<double>(list[4]),Parse<double>(list[5]) }, Parse<double>(list[6]) };
			else return new ChangeColor{ list[1], ColorF{ Parse<double>(list[2]),Parse<double>(list[3]),Parse<double>(list[4]),Parse<double>(list[5]) }, Parse<double>(list[6]),Parse<bool>(list[7])};
		};
		moveResolver[U"ChangeScale"] = [](const Array<String>& list) {
			if (list.size() < 6)return new ChangeScale{ list[1],Parse<double>(list[2]),Parse<double>(list[3]),Parse<double>(list[4]) };
			else return new ChangeScale{ list[1],Parse<double>(list[2]),Parse<double>(list[3]),Parse<double>(list[4]) ,Parse<bool>(list[5]) };
		};
	}

	Motion LoadMotion(String region = U"", bool debug = false) {

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
				if (moveResolver.contains(tmpCsv[row][0])) {
					try {
						tmpMotion->add(moveResolver[tmpCsv[row][0]](tmpCsv[row]));
						addFlg = true;
					}
					catch (...) {
						Print << U"おそらく引数が間違っています。";
					}
				}
				else if (debug)
				{
					Print << U"存在しない命令です。";
				}
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
	int32 window_w = 1200, window_h = 700;
	Window::Resize(window_w, window_h);
	//for (auto& elem : mj::JsonElems)  //なぜかJsonElems変更できない...
	//	if (elem.first == U"Position")
	//		elem.second = Format(Vec2{ window_w / 2, window_h/2 });

	//for (auto& elem : mj::JsonElems) {
	//	Print << elem;
	//}

	MyEditor editor;
	//==========体の用意==========
	Character character{ JSON::Load(U""),2 };
	Character start_character{ JSON::Load(U""),2 };

	MotionLoader loader{ CSV{U""} };

	//Motion motion =
	loader.LoadMotion();

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

	double timeScale = 1.0;
	TextEditState timeScaleText{U"1.0"};
	Font font1{ 25 };

	while (System::Update())
	{
		ClearPrint();
		jsonOk = character.joint != nullptr;
		motionOk = MotionPath.has_value();

		if (editor.working) {
			editor.update();
			editor.draw();
			continue;
		}

		if (MotionSelect(U"モデル読み込み", {10,10}, JsonPath,FileFilter::JSON()))
		{
			//モーションの画面に初めて移ったときの処理
			if (JsonPath) {
				character = Character{ JSON::Load(*JsonPath),2 };
				manager.clear();
				character.setDrawManager(&manager);
			}
		}
		if (MotionSelect(U"スクリプト", {10,50}, MotionPath,FileFilter::Text()))
		{
			if (MotionPath)loader = MotionLoader{ CSV{*MotionPath} };
		}

		if (SimpleGUI::Button(U"リセット", { 10,UiStartY + 40 }))
		{
			if (JsonPath) {
				character = Character{ JSON::Load(*JsonPath),2 };
				manager.clear();
				character.setDrawManager(&manager);
			}
		}

		SimpleGUI::CheckBox(drawFlg, U"通常表示", Vec2{ 10, UiStartY+90 }, 160);
		SimpleGUI::CheckBox(debugFlg, U"デバッグ表示", Vec2{ 10, UiStartY+130 }, 160);
		SimpleGUI::CheckBox(touchGround, U"地面に着かせる", Vec2{ 10,UiStartY + 170 }, 180);
		SimpleGUI::TextBox(text, Vec2{ 100, UiStartY });
		font1(U"倍速").draw(Vec2{ Scene::Width() - 120,10 });
		SimpleGUI::TextBox(timeScaleText, Vec2{ Scene::Width() - 60,10 }, 50);
		try {
			//倍速をセットする。
			timeScale = Parse<double>(timeScaleText.text);
		}
		catch (...){}

		if (SimpleGUI::Button(U"実行", Vec2{ 10, UiStartY },unspecified,jsonOk and motionOk)) {
			//loader.LoadMotion();
			MotionLoader l{ CSV{*MotionPath} };
			/*Motion motion{};
			motion.add(new Translate(U"body", { 100,0 }, 1));*/
			character.motions.clear();
			character.addMotion(l.LoadMotion(text.text, true));
		}
		RectF ground{ 0,500,Scene::Width()*2,100};
		{
			const auto t = camera.getTransformer2D(true);

			if (touchGround)ground .draw(Palette::Green);


			if (jsonOk) {
				if (touchGround)character.touchGround(500);
				character.update(timeScale*Scene::DeltaTime());

				manager.update();
				if (drawFlg)manager.draw();
				if (debugFlg)manager.drawDebug();
			}
		}

		if (SimpleGUI::Button(U"モデルエディタへ", { 20,Scene::Height() - 55 }))
		{
			editor.working = true;
		}
		camera.update(Scene::DeltaTime());
	}
}
