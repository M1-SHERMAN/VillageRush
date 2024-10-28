#ifndef _TOWER_MANAGER_H_
#define _TOWER_MANAGER_H_
/**
 * @brief 防御塔管理クラス
 *
 * このクラスは、ゲーム内の防御塔の生成、管理、更新を担当するシングルトンクラスです。
 *
 * 主な機能:
 * - 異なる種類の防御塔（アーチャー、斧使い、砲兵）の生成と管理
 * - 防御塔のリストの維持と更新
 * - 防御塔の攻撃、アップグレード、売却の処理
 * - 防御塔のレンダリング
 * - 防御塔の設置コストとアップグレードコストの計算
 *
 * 使用方法:
 * - TowerManager::instance()->create_tower() を使用して新しい防御塔を生成
 * - on_update() メソッドを毎フレーム呼び出して防御塔の状態を更新
 * - on_render() メソッドを使用して防御塔をレンダリング
 * - get_place_cost() と get_upgrade_cost() メソッドでコストを取得
 *
 */
#include "manager.h"
#include "tower.h"
#include "archer_tower.h"
#include "axeman_tower.h"
#include "gunner_tower.h"
#include "config_manager.h"
#include "resources_manager.h"

#include <vector>

class TowerManager : public Manager<TowerManager>
{
	friend class Manager<TowerManager>;

public:
	// すべての防御塔の状態を更新する
	void on_update(double delta)
	{
		for (Tower *tower : tower_list)
			tower->on_update(delta);
	}

	// すべての防御塔を描画する
	void on_render(SDL_Renderer *renderer)
	{
		for (Tower *tower : tower_list)
			tower->on_render(renderer);
	}

	// 防御塔の設置コストを取得する
	double get_place_cost(TowerType type)
	{
		static ConfigManager *instance = ConfigManager::instance();

		switch (type)
		{
		case Archer:
			return instance->archer_template.cost[instance->level_archer];
			break;
		case Axeman:
			return instance->axeman_template.cost[instance->level_axeman];
			break;
		case Gunner:
			return instance->gunner_template.cost[instance->level_gunner];
			break;
		}

		return 0;
	}
	// 防御塔のアップグレードコストを取得する
	double get_upgrade_cost(TowerType type)
	{
		static ConfigManager *instance = ConfigManager::instance();

		// 塔のタイプに応じて適切なアップグレードコストを返す
		switch (type)
		{
		case Archer:
			return instance->level_archer == 9 ? -1 : instance->archer_template.upgrade_cost[instance->level_archer]; // 最高レベルに達しているかチェック
			break;
		case Axeman:
			return instance->level_axeman == 9 ? -1 : instance->axeman_template.upgrade_cost[instance->level_axeman];
			break;
		case Gunner:
			return instance->level_gunner == 9 ? -1 : instance->gunner_template.upgrade_cost[instance->level_gunner];
			break;
		}

		return 0;
	}

	// 防御塔の視野範囲を取得する
	double get_view_range(TowerType type)
	{
		static ConfigManager *instance = ConfigManager::instance();

		// 塔のタイプとレベルに応じて適切な視野範囲を返す
		switch (type)
		{
		case Archer:
			return instance->archer_template.view_range[instance->level_archer];
			break;
		case Axeman:
			return instance->axeman_template.view_range[instance->level_axeman];
			break;
		case Gunner:
			return instance->gunner_template.view_range[instance->level_gunner];
			break;
		}

		return 0;
	}

	// 指定された位置に新しい防御塔を設置する
	void place_tower(TowerType type, const SDL_Point &idx)
	{
		// 防御塔のポインタを定義
		Tower *tower = nullptr;

		// 与えられたタイプに応じて適切な防御塔を作成
		switch (type)
		{
		case Archer:
			tower = new ArcherTower();
			break;
		case Axeman:
			tower = new AxemanTower();
			break;
		case Gunner:
			tower = new GunnerTower();
			break;
		default:
			tower = new ArcherTower();
			break;
		}

		// 防御塔の設置位置を計算
		static Vector2 position;
		static const SDL_Rect &rect = ConfigManager::instance()->rect_tile_map;

		position.x = rect.x + idx.x * SIZE_TILE + SIZE_TILE / 2;
		position.y = rect.y + idx.y * SIZE_TILE + SIZE_TILE / 2;

		// 防御塔の位置を設定し、塔リストに追加
		tower->set_position(position);
		tower_list.push_back(tower);
		ConfigManager::instance()->map.place_tower(idx); // マップ上で防御塔の位置をマーク

		// 設置音を再生
		static const ResourcesManager::SoundPool &sound_pool = ResourcesManager::instance()->get_sound_pool();
		Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_PlaceTower)->second, 0);
	}

	// 指定されたタイプの防御塔をアップグレードする
	void upgrade_tower(TowerType type)
	{
		static ConfigManager *instance = ConfigManager::instance();

		// タイプに応じて防御塔のレベルを上げる（最大9レベルまで）
		switch (type)
		{
		case Archer:
			instance->level_archer = instance->level_archer >= 9 ? 9 : instance->level_archer + 1;
			break;
		case Axeman:
			instance->level_axeman = instance->level_axeman >= 9 ? 9 : instance->level_axeman + 1;
			break;
		case Gunner:
			instance->level_gunner = instance->level_gunner >= 9 ? 9 : instance->level_gunner + 1;
			break;
		}

		// アップグレード音を再生
		static const ResourcesManager::SoundPool &sound_pool = ResourcesManager::instance()->get_sound_pool();
		Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_TowerLevelUp)->second, 0);
	}

protected:
	TowerManager() = default;
	~TowerManager() = default;

private:
	std::vector<Tower *> tower_list;
};
#endif // !_TOWER_MANAGER_H_
