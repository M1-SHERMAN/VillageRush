#ifndef _TOWER_H_
#define _TOWER_H_

/**
 * @brief 防御塔基底クラス
 *
 * このクラスは、ゲーム内の防御塔の基本機能を提供する抽象基底クラスです。
 *
 * 主な機能:
 * - 防御塔の位置、向き、攻撃範囲の管理
 * - 敵の検出と攻撃
 * - アニメーション（アイドル状態と攻撃状態）の制御
 * - 攻撃のクールダウン管理
 * - レベルアップと能力値の更新
 *
 * 使用方法:
 * - このクラスを継承して具体的な防御塔クラスを作成する
 * - on_update() メソッドを毎フレーム呼び出して防御塔の状態を更新
 * - on_render() メソッドを使用して防御塔をレンダリング
 * - upgrade() メソッドを呼び出してレベルアップを行う
 *
 * 注意事項:
 * - このクラスは抽象クラスであり、直接インスタンス化せずに継承して使用すること
 * - 継承先のクラスでは、create_bullet() メソッドを適切に実装する必要がある
 * - アニメーションの設定やタイマーの管理に注意し、適切に初期化すること
 */

#include "vector2.h"
#include "animation.h"
#include "tower_type.h"
#include "bullet_manager.h"
#include "facing.h"
#include "config_manager.h"
#include "resources_manager.h"
#include "enemy_manager.h"
#include "timer.h"

#include <SDL.h>

class Tower
{
public:
	Tower()
	{
		// 射撃タイマーをワンショットに設定し、タイムアウト後に射撃可能にする
		timer_fire.set_one_shot(true);
		timer_fire.set_on_timeout(
			[&]()
			{
				can_fire = true; // タイマーがタイムアウトしたら、射撃を許可
			});

		// 各方向のアイドルアニメーションを設定（上下左右）
		anim_idle_up.set_loop(true);
		anim_idle_up.set_interval(0.2); // 各フレームの時間を0.2秒に設定

		anim_idle_down.set_loop(true);
		anim_idle_down.set_interval(0.2);

		anim_idle_left.set_loop(true);
		anim_idle_left.set_interval(0.2);

		anim_idle_right.set_loop(true);
		anim_idle_right.set_interval(0.2);

		// 各方向の射撃アニメーションを設定（上下左右）
		anim_fire_up.set_loop(false); // 射撃アニメーションはループしない
		anim_fire_up.set_interval(0.2);
		anim_fire_up.set_on_finished(
			[&]()
			{
				update_idle_animation(); // 射撃アニメーション完了後、アイドルアニメーションに戻る
			});

		anim_fire_down.set_loop(false);
		anim_fire_down.set_interval(0.2);
		anim_fire_down.set_on_finished(
			[&]()
			{
				update_idle_animation();
			});

		anim_fire_left.set_loop(false);
		anim_fire_left.set_interval(0.2);
		anim_fire_left.set_on_finished(
			[&]()
			{
				update_idle_animation();
			});

		anim_fire_right.set_loop(false);
		anim_fire_right.set_interval(0.2);
		anim_fire_right.set_on_finished(
			[&]()
			{
				update_idle_animation();
			});
	};

	~Tower() = default;

	void set_position(const Vector2 &position)
	{
		this->position = position;
	}

	const Vector2 &get_size() const
	{
		return size;
	}

	const Vector2 &get_position() const
	{
		return position;
	}

	// フレームごとの更新関数、タイマーとアニメーションの更新を処理
	void on_update(double delta)
	{
		timer_fire.on_update(delta);	// 射撃タイマーを更新
		anim_current->on_update(delta); // 現在のアニメーションを更新

		if (can_fire) // 射撃可能な場合、射撃ロジックを呼び出す
		{
			on_fire();
		}
	}

	// 防御塔を画面にレンダリング
	void on_render(SDL_Renderer *renderer)
	{
		static SDL_Point point;
		point.x = (int)(position.x - size.x / 2);
		point.y = (int)(position.y - size.y / 2);

		anim_current->on_render(renderer, point);
	}

protected:
	// 防御塔のサイズ
	Vector2 size;

	// 防御塔のアイドルアニメーション
	Animation anim_idle_up;
	Animation anim_idle_down;
	Animation anim_idle_left;
	Animation anim_idle_right;

	// 防御塔の射撃アニメーション
	Animation anim_fire_up;
	Animation anim_fire_down;
	Animation anim_fire_left;
	Animation anim_fire_right;

	// 防御塔のタイプ、デフォルトは弓兵
	TowerType tower_type = TowerType::Archer;

	// 防御塔の射撃速度、デフォルトは0
	double fire_speed = 0;

	// 防御塔の弾丸タイプ、デフォルトは矢
	BulletType bullet_type = BulletType::Arrow;

private:
	Timer timer_fire;							// 射撃タイマー
	Vector2 position;							// 防御塔の位置
	bool can_fire = true;						// 射撃可能かどうかを制御
	Facing facing = Facing::Right;				// 防御塔の向き、デフォルトは右向き
	Animation *anim_current = &anim_idle_right; // 防御塔の現在のアニメーション、デフォルトは右向きのアイドルアニメーション

private:
	// 現在の防御塔の向きに基づいてアイドルアニメーションを切り替え
	void update_idle_animation()
	{
		switch (facing)
		{
		case Left:
			anim_current = &anim_idle_left;
			break;
		case Right:
			anim_current = &anim_idle_right;
			break;
		case Up:
			anim_current = &anim_idle_up;
			break;
		case Down:
			anim_current = &anim_idle_down;
			break;
		}
	}

	// 現在の防御塔の向きに基づいて射撃アニメーションを切り替え
	void update_fire_animation()
	{
		switch (facing)
		{
		case Left:
			anim_current = &anim_fire_left;
			break;
		case Right:
			anim_current = &anim_fire_right;
			break;
		case Up:
			anim_current = &anim_fire_up;
			break;
		case Down:
			anim_current = &anim_fire_down;
			break;
		}
	}
	// 敵の探索: 攻撃視野範囲内で最も進行度の高い敵を目標として攻撃
	Enemy *find_target_enemy()
	{
		double process = -1;		   // 最も進行度の高い敵の進行度を記録
		double view_range = 0;		   // 視野範囲を初期化
		Enemy *enemy_target = nullptr; // 見つかった目標敵を格納

		// 設定マネージャーのインスタンスを取得し、異なる防御塔の設定を取得するために使用
		static ConfigManager *instance = ConfigManager::instance();

		// 防御塔のタイプに応じて、対応するレベルの攻撃視野範囲を取得
		switch (tower_type)
		{
		case Archer:
			view_range = instance->archer_template.view_range[instance->level_archer];
			break;
		case Axeman:
			view_range = instance->axeman_template.view_range[instance->level_axeman];
			break;
		case Gunner:
			view_range = instance->gunner_template.view_range[instance->level_gunner];
			break;
		}

		// 敵マネージャーから敵リストを取得
		EnemyManager::EnemyList &enemy_list = EnemyManager::instance()->get_enemy_list();

		// すべての敵をループ
		for (Enemy *enemy : enemy_list)
		{
			// 敵と防御塔の距離が視野範囲内かどうかを判断
			if ((enemy->get_position() - position).length() <= view_range * SIZE_TILE)
			{
				// その敵の経路上の進行度を取得
				double new_process = enemy->get_route_process();
				// その敵の進行度が現在記録されている最大進行度より大きい場合、目標敵を更新
				if (new_process > process)
				{
					enemy_target = enemy;
					process = new_process;
				}
			}
		}
		// 見つかった目標敵を返す（条件に合う敵がいない場合はnullptrを返す）
		return enemy_target;
	}

	// 射撃
	void on_fire()
	{
		// 敵探索メソッドを呼び出して目標敵のポインタを取得
		Enemy *target_enemy = find_target_enemy();
		// 目標敵がいない場合、リターン
		if (!target_enemy)
			return;

		// 射撃不可に設定
		can_fire = false;
		// 設定ファイルを取得
		static ConfigManager *instance = ConfigManager::instance();
		// 効果音ファイルを取得
		static const ResourcesManager::SoundPool &sound_pool = ResourcesManager::instance()->get_sound_pool();

		// 属性を初期化：射撃間隔、ダメージ
		double interval = 0, damage = 0;
		// 防御塔のタイプに応じて異なる属性を設定
		switch (tower_type)
		{
		case Archer:
			interval = instance->archer_template.interval[instance->level_archer];
			damage = instance->archer_template.damage[instance->level_archer];
			switch (rand() % 2)
			{
			case 0:
				Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_ArrowFire_1)->second, 0);
				break;
			case 1:
				Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_ArrowFire_2)->second, 0);
				break;
			}
			break;

		case Axeman:
			interval = instance->axeman_template.interval[instance->level_axeman];
			damage = instance->axeman_template.damage[instance->level_axeman];
			Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_AxeFire)->second, 0);
			break;

		case Gunner:
			interval = instance->gunner_template.interval[instance->level_gunner];
			damage = instance->gunner_template.damage[instance->level_gunner];
			Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_ShellFire)->second, 0);
			break;
		}

		// 射撃間隔を再設定
		timer_fire.set_wait_time(interval);
		timer_fire.restart();

		// 敵と防御塔の方向（敵の位置を表すベクトル - 自身のベクトル）
		Vector2 direction = target_enemy->get_position() - position;
		// 弾丸を生成して発射、方向は敵の位置、速度とダメージは防御塔の属性に基づいて設定
		BulletManager::instance()->spawn_bullet(bullet_type, position, direction.normalize() * fire_speed * SIZE_TILE, damage);

		// 防御塔がX軸方向のアニメーションを表示する必要があるかどうかを判断（左右向き）
		bool is_show_x_anim = abs(direction.x) >= abs(direction.y);
		if (is_show_x_anim)
		{
			// 向きを左右に設定
			facing = direction.x > 0 ? Facing::Right : Facing::Left;
		}
		else
		{
			// 向きを上下に設定
			facing = direction.y > 0 ? Facing::Down : Facing::Up;
		}

		// 射撃アニメーションを更新してリセット
		update_fire_animation();
		anim_current->reset();
	}
};

#endif // !_TOWER_H_
