#ifndef _MAP_H_
#define _MAP_H_

#include "tile.h"
#include "route.h"

#include <SDL.h>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>

class Map
{
public:
	typedef std::unordered_map<int, Route> SpawnerRoutePool;

public:
	Map() = default;
	~Map() = default;

	// 指定されたパスからマップを読み込む
	bool load(const std::string &path)
	{
		std::ifstream file(path); // ファイルを開くためのifstream
		if (!file.good())
			return false;

		// 補助変数の初期化
		TileMap tile_map_temp;		// マップを一時的に保存
		int idx_x = -1, idx_y = -1; // マップの列と行を記録
		std::string str_line;		// ファイルから読み取った各行のデータを保存

		// ファイルを1行ずつ読み取る
		while (std::getline(file, str_line))
		{
			str_line = trim_str(str_line);
			if (str_line.empty())
				continue;

			// 空でない行を読み取るたびに、行番号を増やし、マップ全体に新しい行を追加
			idx_x = -1, idx_y++;
			tile_map_temp.emplace_back();

			std::string str_tile;
			std::stringstream str_stream(str_line);
			while (std::getline(str_stream, str_tile, ',')) // str_streamの値を1つずつ読み取り、「,」で分割し、str_tileに保存
			{
				// タイルデータを1つ読み取るたびに:
				idx_x++;								  // 列番号を増やす
				tile_map_temp[idx_y].emplace_back();	  // 新しい列を追加
				Tile &tile = tile_map_temp[idx_y].back(); // 現在の行のTileを取得
				load_tile_from_string(tile, str_tile);
			}
		}
		file.close();

		if (tile_map_temp.empty() || tile_map_temp[0].empty())
		{
			return false;
		}
		tile_map = tile_map_temp;

		generate_map_cache();

		return true;
	}

	// マップの幅を取得
	size_t get_width() const
	{
		if (tile_map.empty())
			return 0;
		return tile_map[0].size();
	}

	// マップの高さを取得
	size_t get_height() const
	{
		return tile_map.size();
	}

	// マップ全体の情報を取得
	const TileMap &get_tile_map() const
	{
		return tile_map;
	}

	// 防御ポイントを取得
	const SDL_Point &get_idx_home() const
	{
		return idx_home;
	}

	// 進行ルートを取得
	const SpawnerRoutePool &get_spawner_route_pool() const
	{
		return spawner_route_pool;
	}

	// 防御タワーを設置
	void place_tower(const SDL_Point &idx_tile)
	{
		tile_map[idx_tile.y][idx_tile.x].has_tower = true;
	}

private:
	TileMap tile_map;
	SDL_Point idx_home = {0};
	SpawnerRoutePool spawner_route_pool;

private:
	// 文字列の両端の空白を削除するために使用
	std::string trim_str(const std::string &str)
	{
		size_t begin_idx = str.find_first_not_of(" \t");
		if (begin_idx == std::string::npos)
			return "";
		size_t end_idx = str.find_last_not_of(" \t");
		size_t idx_range = end_idx - begin_idx + 1;

		return str.substr(begin_idx, idx_range);
	}

	void load_tile_from_string(Tile &tile, const std::string &str)
	{
		// 補助変数の初期化
		std::string str_tidy = trim_str(str);

		// 読み取った値を保存するため
		std::string str_value;
		std::vector<int> values;

		// 文字列の読み書き操作のためのstringstream
		std::stringstream str_stream(str_tidy);

		// 文字列を1行ずつ解析
		while (std::getline(str_stream, str_value, '\\')) // str_streamの値を1つずつ読み取り、「\\」で分割し、str_valueに保存
		{
			int value;
			try
			{
				value = std::stoi(str_value); // str_valueを整数に変換しようとする
			}
			catch (const std::invalid_argument &)
			{
				value = -1;
			}
			values.push_back(value); // 整数値を解析するたびに、valuesにプッシュ
		}

		// 解析された値を割り当てる
		tile.terrian = (values.size() < 1 || values[0] < 0) ? 0 : values[0];
		tile.decoration = (values.size() < 2) ? -1 : values[1];
		tile.direction = (Tile::Direction)((values.size() < 3 || values[2] < 0) ? 0 : values[2]);
		tile.special_flag = (values.size() <= 3) ? -1 : values[3];
	}

	void generate_map_cache()
	{
		// マップ上の各セルを走査
		for (int y = 0; y < get_height(); y++)
		{
			for (int x = 0; x < get_width(); x++)
			{
				const Tile &tile = tile_map[y][x];
				if (tile.special_flag < 0) // 特殊フラグがない場合はスキップ
					continue;

				// 特殊フラグがある場合
				if (tile.special_flag == 0) // この特殊フラグが防御ポイントの場合
				{
					idx_home.x = x;
					idx_home.y = y;
				}
				else
				{
					// この特殊フラグがモンスター出現ポイント（>0）の場合、モンスターの進行ルートを生成
					spawner_route_pool[tile.special_flag] = Route(tile_map, {x, y});
				}
			}
		}
	}
};

#endif // !_MAP_H_
