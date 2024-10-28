#ifndef _ENEMY_MANAGER_H_
#define _ENEMY_MANAGER_H_

/**
 * @brief 敵管理クラス
 *
 * このクラスは、ゲーム内のすべての敵オブジェクトを管理します。
 *
 * 主な機能:
 * - 敵の生成、更新、削除
 * - 敵と本拠地の衝突検出
 * - 敵と弾丸の衝突検出
 * - 敵のレンダリング
 *
 * 使用方法:
 * - EnemyManager::instance()->spawn_enemy() で新しい敵を生成
 * - EnemyManager::instance()->on_update() で敵の状態を更新
 * - EnemyManager::instance()->on_render() で敵をレンダリング
 */

#include "manager.h"
#include "enemy.h"
#include "map.h"
#include "config_manager.h"
#include "home_manager.h"
#include "slim_enemy.h"
#include "king_slim_enemy.h"
#include "skeleton_enemy.h"
#include "goblin_enemy.h"
#include "goblin_priest_enemy.h"
#include "bullet_manager.h"
#include "coin_manager.h"

#include <vector>
#include <SDL.h>

/*敵の管理クラス、ゲーム内のすべての敵オブジェクトを管理する*/
class EnemyManager : public Manager<EnemyManager>
{
	friend class Manager<EnemyManager>;

public:
	// すべての敵のポインタを格納する敵リストの型を定義
	typedef std::vector<Enemy *> EnemyList;

public:
	// 毎フレーム、すべての敵の状態を更新する
	void on_update(double delta)
	{
		for (Enemy *enemy : enemy_list)
			enemy->on_update(delta); // 各敵の状態を更新

		process_home_collision();	// 敵と本拠地の衝突を処理
		process_bullet_collision(); // 敵と弾丸の衝突を処理

		remove_invalid_enemy(); // 無効な敵を削除
	}

	// すべての敵をレンダリングする
	void on_render(SDL_Renderer *renderer)
	{
		for (Enemy *enemy : enemy_list)
		{
			enemy->on_render(renderer);
		}
	}

	void spawn_enemy(EnemyType type, int idx_spawn_point)
	{
		// 敵の生成位置を保存するための変数
		static Vector2 position;
		// 敵の生成位置を決定するためのタイルマップの矩形領域を取得
		static const SDL_Rect &rect_tile_map = ConfigManager::instance()->rect_tile_map;

		// スポーンポイントに対応するルートを検索するためのスポーナールートプールを取得
		static const Map::SpawnerRoutePool &spawner_route_pool = ConfigManager::instance()->map.get_spawner_route_pool();

		// 与えられたスポーンポイントのインデックスに対応するルートを検索
		const auto &itor = spawner_route_pool.find(idx_spawn_point);
		// インデックスが無効な場合は即座に戻る
		if (itor == spawner_route_pool.end())
			return;

		// 生成された敵のインスタンスを保持するためのポインタ
		Enemy *enemy = nullptr;

		// 与えられた敵タイプに応じて対応する敵インスタンスを生成
		switch (type)
		{
		case EnemyType::Slim:
			enemy = new SlimEnemy();
			break;
		case EnemyType::KingSlim:
			enemy = new KingSlimeEnemy();
			break;
		case EnemyType::Skeleton:
			enemy = new SkeletonEnemy();
			break;
		case EnemyType::Goblin:
			enemy = new GoblinEnemy();
			break;
		case EnemyType::GoblinPriest:
			enemy = new GoblinPriestEnemy();
			break;
		default:
			enemy = new SlimEnemy();
			break;
		}

		// 生成された敵のスキル発動時のコールバック関数を設定
		enemy->set_on_skill_released(
			[&](Enemy *enemy_src)
			{
				// スキルの回復半径を取得
				double recover_radius = enemy_src->get_recover_radius();
				if (recover_radius < 0)
					return;

				// 源敵の位置を取得
				const Vector2 pos_src = enemy_src->get_position();

				// すべての敵をループし、回復半径内にいるかどうかを判断
				for (Enemy *enemy_dst : enemy_list)
				{
					const Vector2 &pos_dst = enemy_dst->get_position();
					double distance = (pos_dst - pos_src).length();
					// 目標敵が回復範囲内にいる場合、そのHPを増加させる
					if (distance <= recover_radius)
						enemy_dst->increase_hp(enemy_src->get_recover_intensity());
				}
			});

		// スポーンポイントに対応するルートのインデックスリストを取得
		const Route::IdxList &idx_list = itor->second.get_idx_list();

		// 敵の生成位置を計算（タイルマップに基づく）
		position.x = rect_tile_map.x + idx_list[0].x * SIZE_TILE + SIZE_TILE / 2;
		position.y = rect_tile_map.y + idx_list[0].y * SIZE_TILE + SIZE_TILE / 2;

		// 敵の初期位置を設定
		enemy->set_position(position);

		// 敵に移動ルートを割り当てる
		enemy->set_route(&itor->second);

		// 敵をグローバルな敵リストに追加
		enemy_list.push_back(enemy);
	}

	bool check_cleared()
	{
		return enemy_list.empty();
	}

	EnemyList &get_enemy_list()
	{
		return enemy_list;
	}

protected:
	EnemyManager() = default;

	// デストラクタ、すべての敵を破棄
	~EnemyManager()
	{
		for (Enemy *enemy : enemy_list)
		{
			delete enemy;
		}
	}

private:
	EnemyList enemy_list; // 敵リスト、現在のすべての敵のポインタを格納

private:
	// 敵と本拠地の衝突を処理
	void process_home_collision()
	{
		// マップ上の本拠地のインデックス位置を取得
		static const SDL_Point &idx_home = ConfigManager::instance()->map.get_idx_home();
		// マップ上の本拠地のインデックス位置を取得
		static const SDL_Rect &rect_tile_map = ConfigManager::instance()->rect_tile_map;
		// 本拠地の実際のピクセル位置を計算
		static const Vector2 position_home_tile =
			{
				(double)rect_tile_map.x + idx_home.x * SIZE_TILE,
				(double)rect_tile_map.y + idx_home.y * SIZE_TILE,
			};

		// すべての敵をループし、本拠地との衝突をチェック
		for (Enemy *enemy : enemy_list)
		{
			if (enemy->can_remove())
				continue;

			// 敵の現在位置を取得
			const Vector2 &position = enemy->get_position();

			// 敵が本拠地エリアに入ったかどうかをチェック
			if (position.x >= position_home_tile.x && position.y >= position_home_tile.y && position.x <= position_home_tile.x + SIZE_TILE && position.y <= position_home_tile.y + SIZE_TILE)
			{
				// 敵を無効としてマークし、本拠地のHPを減少させる
				enemy->make_invalid();
				HomeManager::instance()->decrease_hp(enemy->get_damage());
			}
		}
	}

	// 敵と弾丸の衝突を処理
	void process_bullet_collision()
	{
		// 現在のゲーム内のすべてのアクティブな弾丸を取得
		static BulletManager::BulletList &bullet_list = BulletManager::instance()->get_bullet_list();

		// 弾丸が敵と衝突しているかどうかを検出
		for (Enemy *enemy : enemy_list)
		{
			if (enemy->can_remove())
				continue;
			// 敵のサイズと位置を取得
			const Vector2 &size_enemy = enemy->get_size();
			const Vector2 &pos_enemy = enemy->get_position();

			for (Bullet *bullet : bullet_list)
			{
				if (!bullet->can_collide())
					continue;
				// 弾丸の位置を取得
				const Vector2 &pos_bullet = bullet->get_position();

				// 衝突検出（弾丸の中心位置とモンスターの矩形の端を使用して検出）
				if (pos_bullet.x >= pos_enemy.x - size_enemy.x / 2 && pos_bullet.y >= pos_enemy.y - size_enemy.y / 2 && pos_bullet.x <= pos_enemy.x + size_enemy.x / 2 && pos_bullet.y <= pos_enemy.y + size_enemy.y / 2)
				{
					// 衝突が発生した場合
					double damage = bullet->get_damage();			  // 弾丸のダメージを取得
					double damage_range = bullet->get_damage_range(); // 弾丸のダメージ範囲を取得

					// 範囲ダメージがない場合
					if (damage_range < 0)
					{
						enemy->decrease_hp(damage); // 弾丸のダメージに基づいて敵のHPを減少
						if (enemy->can_remove())
						{
							enemy->try_spawn_coin_prop(pos_enemy, enemy->get_reward_ratio()); // 確率に応じてコインを生成
						}
					}
					// 範囲ダメージがある場合
					else
					{
						for (Enemy *target_enemy : enemy_list) // 再度すべての敵をループ
						{
							const Vector2 &pos_target_enemy = target_enemy->get_position();

							// 範囲チェック（敵の位置と弾丸の位置の距離が弾丸のダメージ範囲以下）
							if ((pos_target_enemy - pos_bullet).length() <= damage_range)
							{
								target_enemy->decrease_hp(damage);
								if (target_enemy->can_remove())
								{
									target_enemy->try_spawn_coin_prop(pos_target_enemy, target_enemy->get_reward_ratio());
								}
							}
						}
					}
					// 衝突した敵に対して衝突効果を実行
					bullet->on_collide(enemy);
				}
			}
		}
	}

	// 無効な敵を削除
	void remove_invalid_enemy()
	{
		// std::remove_if を使用して無効とマークされた敵を削除し、メモリを解放
		enemy_list.erase(std::remove_if(enemy_list.begin(), enemy_list.end(),
										[](const Enemy *enemy)
										{
											bool deletable = enemy->can_remove();
											if (deletable)
												delete enemy;
											return deletable;
										}),
						 enemy_list.end());
	}
};

#endif // !_ENEMY_MANAGER_H_
