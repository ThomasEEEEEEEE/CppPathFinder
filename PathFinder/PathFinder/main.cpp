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
	Tile(int X, int Y, TileState s)
	{
		state = s;
		x = X;
		y = Y;
		g = 1;
		h = 0;
		f = INT_MAX;
		parent = nullptr;
	}
	TileState state;
	int x;
	int y;
	int g;
	int h;
	int f;
	Tile* parent;
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
			Map.push_back(Tile(i % MapWidth, i % MapHeight, EMPTY));
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
					DoAStar();
			}
			else if (GetMouse(1).bPressed)
			{
				if (EndTile != nullptr) EndTile->state = EMPTY;
				Map[index].state = END;
				EndTile = &Map[index];

				if (StartTile && EndTile)
					DoAStar();
			}
		}
		else if (GetMouse(0).bPressed)
		{
			int index = (GetMouseY() / TileSize) * MapWidth + (GetMouseX() / TileSize);
			if (Map[index].state % 2 == 0) //Only change state if not start or end
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

	void ClearMap()
	{
		for (int i = 0; i < MapWidth * MapHeight; ++i)
		{
			Map[i].g = 1;
			Map[i].f = 0;
			Map[i].h = 0;
			Map[i].parent = nullptr;
		}
	}

	vector<Tile> DoAStar()
	{
		auto GetDistance = [](Tile* a, Tile* b)
		{
			return sqrtf((a->x - b->x) * (a->x - b->x) + (a->y - b->y) * (a->y - b->y));
		};

		ClearMap();

		Tile* Current = StartTile;
		vector<Tile*> OpenList;
		vector<Tile*> ClosedList;

		OpenList.push_back(StartTile);

		while (OpenList.size > 0)
		{
			Tile* SmallestF = nullptr;
			int SmallestFPos = 0;

			//Find the Tile in the open list with the smallest f value
			for (int i = 0; i < OpenList.size; ++i)
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
			//Add if not blocked
			Neighbors.push_back(&Map[(SmallestF->y - 1) * MapWidth + (SmallestF->x)]);
			Neighbors.push_back(&Map[(SmallestF->y + 1) * MapWidth + (SmallestF->x)]);
			Neighbors.push_back(&Map[(SmallestF->y) * MapWidth + (SmallestF->x - 1)]);
			Neighbors.push_back(&Map[(SmallestF->y) * MapWidth + (SmallestF->x + 1)]);

			for (Tile* t : Neighbors)
			{
				/*i) if successor is the goal, stop search
          successor.g = q.g + distance between 
                              successor and q
          successor.h = distance from goal to 
          successor (This can be done using many 
          ways, we will discuss three heuristics- 
          Manhattan, Diagonal and Euclidean 
          Heuristics)
          
          successor.f = successor.g + successor.h

        ii) if a node with the same position as 
            successor is in the OPEN list which has a 
           lower f than successor, skip this successor

        iii) if a node with the same position as 
            successor  is in the CLOSED list which has
            a lower f than successor, skip this successor
            otherwise, add  the node to the open list*/
			}

			ClosedList.push_back(SmallestF);
		}
	}
};

int main()
{
	Finder demo;
	if (demo.Construct(1024, 576, 1, 1))
		demo.Start();

	return 0;
}