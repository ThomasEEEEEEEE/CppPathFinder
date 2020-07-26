#define OLC_PGE_APPLICATION
#include <vector>
using std::vector;
#include "olcPixelGameEngine.h"

enum TileState
{
	EMPTY = 0,
	START,
	BLOCKED,
	END
};

class Tile
{
public:
	Tile(TileState s)
	{
		state = s;
	}
	TileState state;
};

class Finder : public olc::PixelGameEngine
{
public:
	Finder()
	{
		sAppName = "Path Finder";
	}

public:
	int AppWidth = ScreenWidth();
	int AppHeight = ScreenHeight();
	int MapWidth = 64;
	int MapHeight = 36;
	int TileSize = 16;
	Tile * StartTile = nullptr;
	Tile * EndTile = nullptr;
	vector<Tile> Map;

	bool OnUserCreate() override
	{
		for (int i = 0; i < MapWidth * MapHeight; ++i)
		{
			Map.push_back(Tile(EMPTY));
		}
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		Clear(olc::BLACK);
		
		if (GetKey(olc::CTRL).bHeld)
		{
			int index = (GetMouseY() / TileSize) * MapWidth + (GetMouseX() / TileSize);
			if (GetMouse(0).bPressed)
			{
				Map[index].state = START;
			}
			else if (GetMouse(1).bPressed)
			{
				Map[index].state = END;
			}
		}
		else if (GetMouse(0).bPressed)
		{
			int index = (GetMouseY() / TileSize) * MapWidth + (GetMouseX() / TileSize);
			if (Map[index].state % 2 == 0)
				Map[index].state = static_cast<TileState>((Map[index].state + 2) % 4);
		}

		for (int i = 0; i < MapWidth; ++i)
		{
			for (int j = 0; j < MapHeight; ++j)
			{
				if (Map[j * MapWidth + i].state == EMPTY)
					FillRect(i * TileSize + 2, j * TileSize + 2, TileSize - 4, TileSize - 4, olc::BLUE);
				else if (Map[j * MapWidth + i].state == BLOCKED)
					FillRect(i * TileSize + 2, j * TileSize + 2, TileSize - 4, TileSize - 4, olc::GREY);
				else if (Map[j * MapWidth + i].state == START)
					FillRect(i * TileSize + 2, j * TileSize + 2, TileSize - 4, TileSize - 4, olc::GREEN);
				else if (Map[j * MapWidth + i].state == END)
					FillRect(i * TileSize + 2, j * TileSize + 2, TileSize - 4, TileSize - 4, olc::RED);
			}
		}

		return true;
	}
};


int main()
{
	Finder demo;
	if (demo.Construct(1024, 576, 1, 1))
		demo.Start();

	return 0;
}