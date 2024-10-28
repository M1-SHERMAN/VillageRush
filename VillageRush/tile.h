#ifndef _TILE_H_
#define _TILE_H_

#include <vector>

#define SIZE_TILE 48

struct Tile
{
	// 進行方向
	enum class Direction
	{
		None = 0,
		Up,
		Down,
		Left,
		Right
	};

	int terrian = 0;					   // 地形
	int decoration = -1;				   // 装飾
	int special_flag = -1;				   // 特殊フラグ（防御ポイント、モンスター出現ポイントなど）
	Direction direction = Direction::None; // 方向マーカー

	bool has_tower = false; // 現在の位置に防御タワーが存在するかどうかを記録するためのフラグ
};

typedef std::vector<std::vector<Tile>> TileMap; // TileMapという名前の2次元配列を定義

#endif // !_TILE_H_
