/**********************************************************
* Author: Thomas Eberhart (ThomasEEEEEEEE)
* 
* Description: This application shows off the A* algorithm. 
* The user is shown a grid of tiles. They can place a start tile, 
* an end tile, or an obstacle anywhere on the grid. Every time a 
* new tile is placed, the algorithm will find the shortest path
* between the start and end tiles and display it.
* 
* Left clicking on a tile will place an obstacle. 
* Left clicking while holding CTRL on a tile will place the start tile. 
* Right clicking while holding CTRL on a tile will place the end tile.
* 
* Note: The code for this program commonly refers to something
* that I dubbed a "streak". In this context, a streak is when
* the mouse is held down and numerous tiles are placed in one
* "streak" of the mouse.
**********************************************************/
#define OLC_PGE_APPLICATION
#include <vector>
#include <algorithm>
using std::vector;
#include "olcPixelGameEngine.h"

enum TileState
{
	EMPTY = 0, //Empty tile that can be used for pathing
	START, //Start of the path
	BLOCKED, //Obstacle that cannot be used for pathing
	END, //End of the path
	PATHED //Empty tile that has been used for pathing
};

enum StreakState
{
	INACTIVE, //No active streak
	BLANK, //Current streak is placing empty tiles
	BLOCK //Current streak is placing blocking tiles
};

class Tile
{
public:
	Tile(int _x, int _y, TileState s)
	{
		state = s;
		x = _x;
		y = _y;
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
	int MapWidth = 64; //Map is 64 tiles wide
	int MapHeight = 36; //Map is 36 tiles high
	int TileSize = 16; //Each tile is 16 pixels
	Tile* StartTile = nullptr;
	Tile* EndTile = nullptr;
	vector<Tile> Map;
	StreakState StreakState = INACTIVE; //When a streak occurs, only allow changing one state to another

	bool OnUserCreate() override
	{
		//Create the map consisting of empty tiles
		for (int i = 0; i < MapWidth * MapHeight; ++i)
		{
			Map.push_back(Tile(i % MapWidth, i / MapWidth, EMPTY));
		}
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		//Clear the screen to black
		Clear(olc::BLACK);

		//Clicking when CTRL is held will place a start or end tile
		if (GetKey(olc::CTRL).bHeld)
		{
			//Get the index into the Map vector by converting pixel space coordinates into tile space
			int index = (GetMouseY() / TileSize) * MapWidth + (GetMouseX() / TileSize);

			if (GetMouse(0).bPressed && Map[index].state != START && Map[index].state != END)
			{
				//If the start tile has been placed somewhere else already, make sure to remove it first
				if (StartTile != nullptr) StartTile->state = EMPTY;

				//Set the state of the new start tile and point the StartTile pointer to this tile
				Map[index].state = START;
				StartTile = &Map[index];

				//If both the start and end tiles have been placed then do the algorithm
				if (StartTile && EndTile)
					DoAStar();
			}
			else if (GetMouse(1).bPressed && Map[index].state != END && Map[index].state != START)
			{
				//If the end tile has been placed somewhere else already, make sure to remove it first
				if (EndTile != nullptr) EndTile->state = EMPTY;

				//Set the state of the new end tile and point the EndTile pointer to this tile
				Map[index].state = END;
				EndTile = &Map[index];

				//If both the start and end tiles have been placed then do the algorithm
				if (StartTile && EndTile)
					DoAStar();
			}
		}
		else if (GetMouse(0).bHeld)
		{
			//Get the index into the Map vector by converting pixel space coordinates into tile space
			int index = (GetMouseY() / TileSize) * MapWidth + (GetMouseX() / TileSize);

			//If this is the first frame that the left mouse button has been pressed then start a streak
			if (GetMouse(0).bPressed)
			{
				if (Map[index].state == EMPTY || Map[index].state == PATHED)
					StreakState = BLANK;
				else if (Map[index].state == BLOCKED)
					StreakState = BLOCK;
			}

			//Only change the current tile if it matches the current streak
			if ((Map[index].state == EMPTY || Map[index].state == PATHED) && StreakState != BLOCK)
				Map[index].state = BLOCKED;
			else if (Map[index].state == BLOCKED && StreakState != BLANK)
				Map[index].state = EMPTY;

			//If both the start and end tiles have been placed then do the algorithm
			if (StartTile && EndTile)
				DoAStar();

		}

		//End the streak when the mouse is released
		if (GetMouse(0).bReleased)
		{
			StreakState = INACTIVE;
		}

		//Drawing the map to the screen
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
		//Set the state of each tile in the path that isn't the start or end state to PATHED
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
				Tile* t = OpenList[i];

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

			if (SmallestF->y > 0) 
				Above = &Map[(SmallestF->y - 1) * MapWidth + (SmallestF->x)];

			if (SmallestF->y < MapHeight - 1) 
				Below = &Map[(SmallestF->y + 1) * MapWidth + (SmallestF->x)];

			if (SmallestF->x > 0) 
				Left = &Map[(SmallestF->y) * MapWidth + (SmallestF->x - 1)];

			if (SmallestF->x < MapWidth - 1) 
				Right = &Map[(SmallestF->y) * MapWidth + (SmallestF->x + 1)];


			if (Above && Above->state != BLOCKED && Above->y < SmallestF->y) 
				Neighbors.push_back(Above);
			if (Below && Below->state != BLOCKED && Below->y > SmallestF->y) 
				Neighbors.push_back(Below);
			if (Left && Left->state != BLOCKED && Left->x < SmallestF->x) 
				Neighbors.push_back(Left);
			if (Right && Right->state != BLOCKED && Right->x > SmallestF->x) 
				Neighbors.push_back(Right);

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
		return vector<Tile*>(); //Return empty path if it cannot be found
	}
};

int main()
{
	Finder finder;
	if (finder.Construct(1024, 576, 1, 1))
		finder.Start();

	return 0;
}