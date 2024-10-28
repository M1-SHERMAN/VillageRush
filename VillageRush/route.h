#ifndef _ROUTE_H_
#define _ROUTE_H_

#include "tile.h"

#include <SDL.h>
#include <vector>

class Route
{
public:
	// SDL_Pointを使用して経路点を表現
	typedef std::vector<SDL_Point> IdxList; // IdxListは経路点情報を格納するために使用

public:
	Route() = default;

	// コンストラクタ：マップと開始点を受け取り、経路を計算
	Route(const TileMap &map, const SDL_Point &idx_origin)
	{
		// マップの幅と高さを取得し、開始経路点を定義
		size_t width_map = map[0].size();
		size_t height_map = map.size();
		SDL_Point idx_next = idx_origin;

		// マップを巡回し、経路点の終点を見つけるか、進行不可能になるまでループ
		while (true)
		{
			// マップの境界を超えているかチェック
			if (idx_next.x >= width_map || idx_next.y >= height_map)
				break;

			// 重複経路が存在するかチェック
			if (check_duplicate_idx(idx_next))
				break;
			else
				// 現在の点を経路に追加
				idx_list.push_back(idx_next);

			// 現在のTile情報を取得
			bool is_next_dir_exist = true; // 次の方向が存在するかを示すフラグ
			const Tile &tile = map[idx_next.y][idx_next.x];

			// 現在の点が防御点の場合、経路探索を停止
			if (tile.special_flag == 0)
				break;

			// tileの方向に基づいて次の経路点の座標を更新
			switch (tile.direction)
			{
			case Tile::Direction::Up:
				idx_next.y--;
				break;
			case Tile::Direction::Down:
				idx_next.y++;
				break;
			case Tile::Direction::Left:
				idx_next.x--;
				break;
			case Tile::Direction::Right:
				idx_next.x++;
				break;
			default:
				// 方向が存在しない場合、経路探索を停止
				is_next_dir_exist = false;
				break;
			}
			if (!is_next_dir_exist)
				break;
		}
	}
	~Route() = default;

	// 経路点リストを取得
	const IdxList &get_idx_list() const
	{
		return idx_list;
	}

private:
	IdxList idx_list;

private:
	// 現在の点が経路内に既に存在するかをチェック（重複を避けるため）
	bool check_duplicate_idx(const SDL_Point &target_idx)
	{
		// 既存の経路点を巡回し、目標点と重複しているかチェック
		for (const SDL_Point &idx : idx_list)
		{
			// x座標が同じで、y座標も一致する場合、経路点が重複
			if (idx.x == target_idx.x && idx.y == target_idx.y)
				return true;
		}
		return false;
	};
};
#endif // !_ROUTE_H_
