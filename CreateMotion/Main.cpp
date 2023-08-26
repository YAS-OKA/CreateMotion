# include <Siv3D.hpp> // OpenSiv3D v0.6.6
#include"AnimationSystem.h"

//設定した角度回転
class Rotate :public TimeMove {
public:
	double rad;
	String target;

	Rotate(const String target, double rad, double timelim) :TimeMove{ timelim }, target{ target }, rad{ rad } {}

	void update(Character* character, double dt = Scene::DeltaTime())override {
		dt = calTime(dt);
		character->get(target)->angle += dt * rad / timelim;
	}
};

class SetAngle :public Move {
public:
	double rad;
	String target;
	SetAngle(const String& target, double rad) :target{ target }, rad{ rad } {}

	void start(Character* character)override {
		character->get(target)->angle = rad;
	}

	void update(Character* character, double dt = Scene::DeltaTime())override {}

	bool isActive()override {
		return false;
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
	}

	Motion LoadMotion(String region = U"") {

		Motion motion;

		CSV tmpCsv = csv;

		size_t index = 0;

		if (region != U"") {
			for (size_t row = 0; row < tmpCsv.rows(); ++row)
			{
				if ((tmpCsv.columns(row) != 0) && tmpCsv[row][0] == region) {
					index = row + 1;
					break;
				}
			}
		}

		HashTable<String, String>variableTable;

		Motion* tmpMotion = new Motion{};
		tmpMotion->parallel = true;

		//size_t - int =size_t なので tmpCsv.rows()が0のときほぼ無限ループになる。
		//場合わけで解決？ -1いらなくない？
		for (size_t row = index; row < tmpCsv.rows(); ++row)
		{
			if (tmpCsv.columns(row) == 0) {
				motion.add(tmpMotion);
				tmpMotion = new Motion{};
				tmpMotion->parallel = true;
			}
			else if (tmpCsv[row][0][0] == U'#') {
				break;
			}
			else if (tmpCsv[row][0][0] == U'@') {
				variableTable[tmpCsv[row][0]] = tmpCsv[row][1];
			}
			else {
				for (size_t col = 0; col < tmpCsv.columns(row); ++col) {
					//変数を数値に置き換え
					if (variableTable.contains(tmpCsv[row][col])) {
						tmpCsv[row][col] = variableTable[tmpCsv[row][col]];
					}
				}
				tmpMotion->add(moveResolver[tmpCsv[row][0]](tmpCsv[row]));
			}
		}
		//あとで考える
		motion.add(tmpMotion);
		return motion;
	}
};

void Main()
{
	//==========体の用意==========
	Character character{ JSON::Load(U"tim-kun/test.json"),2 };

	MotionLoader loader{ CSV{U"motion.txt"} };

	Motion motion = loader.LoadMotion();

	//==========描画に追加==========
	DrawManager manager;
	character.setDrawManager(&manager);

	while (System::Update())
	{
		//if (MouseL.down()) {
		//	MotionLoader loader{ CSV{U"motion.txt"} };
		//	character.addMotion(loader.LoadMotion());
		//}
		motion.update(&character);

		character.update();
		character.touchGround(500);

		manager.draw();

		RectF{ 0,500,800,100 }.draw(Palette::Green);
	}
}

