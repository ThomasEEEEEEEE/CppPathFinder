#define OLC_PGE_APPLICATION
#include <vector>
#include <algorithm>
using std::vector;
#include "olcPixelGameEngine.h"

enum TileState
{
	EMPTY = 0,
	START,
	BLOCKED,
	END,
	PATHED
};

class Tile
{
public:
	Tile(int X, int Y, TileState s)
	{
		state = s;
		x = X;
		y = Y;
		g = 1;
		h = 0;
		f = INT_MAX;
		parent = nullptr;
		ChangedThisStreak = false;
	}
	TileState state;
	int x;
	int y;
	int g;
	int h;
	int f;
	Tile* parent;
	bool ChangedThisStreak;
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
	TileState StreakState = PATHED; //When a streak occurs, only allow changing one state to another

	bool OnUserCreate() override
	{
		for (int i = 0; i < MapWidth * MapHeight; ++i)
		{
			Map.push_back(Tile(i % MapWidth, i / MapWidth, EMPTY));
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
				if (StartTile != nullptr) StartTile->state = EMPTY;
				Map[index].state = START;
				StartTile = &Map[index];

				if (StartTile && EndTile)
				{
					DoAStar();
				}
			}
			else if (GetMouse(1).bPressed)
			{
				if (EndTile != nullptr) EndTile->state = EMPTY;
				Map[index].state = END;
				EndTile = &Map[index];

				if (StartTile && EndTile)
				{
					DoAStar();
				}
			}
		}
		else if (GetMouse(0).bHeld)
		{
			int index = (GetMouseY() / TileSize) * MapWidth + (GetMouseX() / TileSize);

			if (GetMouse(0).bPressed)
			{
				StreakState = Map[index].state;
			}

			if (!Map[index].ChangedThisStreak)
			{
				if ((Map[index].state == EMPTY || Map[index].state == PATHED) && StreakState != BLOCKED)
				{
					Map[index].state = BLOCKED;
					Map[index].ChangedThisStreak = true;
				}
				else if (Map[index].state == BLOCKED && StreakState != EMPTY)
				{
					Map[index].state = EMPTY;
					Map[index].ChangedThisStreak = true;
				}

				if (StartTile && EndTile)
				{
					DoAStar();
				}
			}
		}

		if (GetMouse(0).bReleased)
		{
			for (int i = 0; i < MapWidth * MapHeight; ++i)
			{
				Map[i].ChangedThisStreak = false;
			}
			StreakState = PATHED;
		}

		for (int i = 0; i < MapWidth; ++i)
		{
			for (int j = 0; j < MapHeight; ++j)
			{
				switch (Map[j * MapWidth + i].state)
				{
				case EMPTY:
					FillRect(i * TileSize + 2, j * TileSize + 2, TileSize - 4, TileSize - 4, olc::BLUE);
					break;
				case BLOCKED:
					FillRect(i * TileSize + 2, j * TileSize + 2, TileSize - 4, TileSize - 4, olc::GREY);
					break;
				case START:
					FillRect(i * TileSize + 2, j * TileSize + 2, TileSize - 4, TileSize - 4, olc::GREEN);
					break;
				case END:
					FillRect(i * TileSize + 2, j * TileSize + 2, TileSize - 4, TileSize - 4, olc::RED);
					break;
				case PATHED:
					FillRect(i * TileSize + 2, j * TileSize + 2, TileSize - 4, TileSize - 4, olc::YELLOW);
					break;
				default:
					FillRect(i * TileSize + 2, j * TileSize + 2, TileSize - 4, TileSize - 4, olc::DARK_MAGENTA);
					break;
				}
			}
		}

		return true;
	}

	void DrawPath(vector<Tile*> Path)
	{
		for (Tile* t : Path)
		{
			if (t->state != START && t->state != END) 
				t->state = PATHED;
		}
	}

	void ClearMap()
	{
		for (int i = 0; i < MapWidth * MapHeight; ++i)
		{
			Map[i].g = 1;
			Map[i].f = INT_MAX;
			Map[i].h = 0;
			Map[i].parent = nullptr;
			if (Map[i].state == PATHED) Map[i].state = EMPTY;
		}
	}

	vector<Tile*> DoAStar()
	{
		auto GetDistance = [](Tile* a, Tile* b)
		{
			return abs(a->x - b->x) + abs(a->y - b->y);
		};

		ClearMap();

		Tile* Current = StartTile;
		vector<Tile*> OpenList;
		vector<Tile*> ClosedList;

		OpenList.push_back(StartTile);

		while (OpenList.size() > 0)
		{
			Tile* SmallestF = nullptr;
			int SmallestFPos = 0;

			//Find the Tile in the open list with the smallest f value
			for (int i = 0; i < OpenList.size(); ++i)
			{
				Tile * t = OpenList[i];

				if (!SmallestF || SmallestF->f > t->f)
				{
					SmallestF = t;
					SmallestFPos = i;
				}
			}
			//Remove the Tile with the smallest f value from the open list
			OpenList.erase(OpenList.begin() + SmallestFPos);

			vector<Tile*> Neighbors;
			Tile* Above = nullptr;
			Tile* Below = nullptr;
			Tile* Left = nullptr;
			Tile* Right = nullptr;

			if (SmallestF->y > 0) Above = &Map[(SmallestF->y - 1) * MapWidth + (SmallestF->x)];
			if (SmallestF->y < MapHeight - 1) Below = &Map[(SmallestF->y + 1) * MapWidth + (SmallestF->x)];
			if (SmallestF->x > 0) Left = &Map[(SmallestF->y) * MapWidth + (SmallestF->x - 1)];
			if (SmallestF->x < MapWidth - 1) Right = &Map[(SmallestF->y) * MapWidth + (SmallestF->x + 1)];


			if (Above && Above->state != BLOCKED && Above->y < SmallestF->y) Neighbors.push_back(Above);
			if (Below && Below->state != BLOCKED && Below->y > SmallestF->y) Neighbors.push_back(Below);
			if (Left && Left->state != BLOCKED && Left->x < SmallestF->x) Neighbors.push_back(Left);
			if (Right && Right->state != BLOCKED && Right->x > SmallestF->x) Neighbors.push_back(Right);

			for (Tile* t : Neighbors)
			{
				//If neighbor is the goal
				if (t->x == EndTile->x && t->y == EndTile->y)
				{
					t->parent = SmallestF;
					vector<Tile*> Path;
					Tile* itr = EndTile;
					
					while (itr != nullptr)
					{
						Path.push_back(itr);
						itr = itr->parent;
					}
					std::reverse(Path.begin(), Path.end());
					DrawPath(Path);
					return Path;
				}
				//Else if the neighbor is not in the closed list
				else if (std::find(ClosedList.begin(), ClosedList.end(), t) == ClosedList.end())
				{
					t->g = SmallestF->g + 1;
					t->h = GetDistance(EndTile, t);
					int NewF = t->g + t->h;

					
					if (t->f > NewF)
					{
						OpenList.push_back(t);
						t->f = NewF;
						t->parent = SmallestF;
					}
				}
			}

			ClosedList.push_back(SmallestF);
		}
		return vector<Tile*>();
	}
};

int main()
{
	Finder demo;
	if (demo.Construct(1024, 576, 1, 1))
		demo.Start();

	return 0;
}