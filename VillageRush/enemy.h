#ifndef _ENEMY_H_
#define _ENEMY_H_

/**
 * @brief 敵キャラクタークラス
 *
 * このクラスは、ゲーム内の敵キャラクターの動作、状態、スキルを管理します。
 *
 * 主な機能:
 * - 敵の移動と経路追跡
 * - 体力と速度の管理
 * - スキルの発動とタイマー制御
 * - ヒットアニメーションの制御
 * - コインドロップの処理
 *
 * 使用方法:
 * - Enemy オブジェクトを生成し、初期化する
 * - on_update() メソッドを毎フレーム呼び出して敵の状態を更新
 * - on_render() メソッドを使用して敵をレンダリング
 * - take_damage() メソッドでダメージを与える
 * - set_skill() メソッドでスキルを設定
 *
 * 注意事項:
 * - 敵の初期化時に適切な経路とスキルを設定することが重要
 * - メモリリークを防ぐため、不要になった敵オブジェクトの適切な削除が必要
 * - スキルのコールバック関数は適切に設定し、nullptrチェックを行うこと
 */

#include "timer.h"
#include "vector2.h"
#include "animation.h"
#include "route.h"
#include "config_manager.h"
#include "coin_manager.h"

#include <functional>
#include <random>
	class Enemy
{
public:
	typedef std::function<void(Enemy *enemy)> SkillCallback; // スキル発動のコールバック関数タイプを定義

public:
	Enemy()
	{
		timer_skill.set_one_shot(false); // スキルタイマー、繰り返し発動
		timer_skill.set_on_timeout([&]()
								   { on_skill_released(this); }); // スキル発動時のコールバック

		timer_sketch.set_one_shot(true);   // ヒットアニメーションタイマー、一回のみ発動
		timer_sketch.set_wait_time(0.075); // ヒットアニメーションの持続時間を設定
		timer_sketch.set_on_timeout([&]()
									{ is_show_sketch = false; }); // タイマー終了時にヒットアニメーションを終了

		timer_restore_speed.set_one_shot(true); // 速度回復タイマー、一回のみ発動
		timer_restore_speed.set_on_timeout([&]()
										   { speed = max_speed; }); // 最大速度に回復
	}
	~Enemy() = default;

	/*フレームごとの更新関数、移動、アニメーション、タイマーの更新を処理*/
	void on_update(double delta)
	{
		timer_skill.on_update(delta);		  // スキルタイマーを更新
		timer_sketch.on_update(delta);		  // ヒットアニメーションタイマーを更新
		timer_restore_speed.on_update(delta); // 速度回復タイマーを更新

		/*速度とフレーム間隔に基づいて移動距離を計算*/
		Vector2 move_distance = velocity * delta;			  // 現在のフレームの最大移動距離
		Vector2 target_distance = position_target - position; // 現在のフレームから目標点までの距離

		// 目標点に近づいたかどうかを判断
		if (target_distance.approx_zero())
		{
			idx_target++;										  // 目標点のインデックスを更新
			refresh_position_target();							  // 次の目標位置を更新
			direction = (position_target - position).normalize(); // 移動方向を更新
		}

		position += move_distance < target_distance ? move_distance : target_distance; // 実際の移動距離
		// 速度を計算、方向 * 速度 * 単一グリッドの距離
		velocity.x = direction.x * speed * SIZE_TILE;
		velocity.y = direction.y * speed * SIZE_TILE;

		/*速度と状態に基づいて現在のアニメーションを選択*/
		bool is_show_x_amin = abs(velocity.x) >= abs(velocity.y);

		if (is_show_sketch)
		{
			if (is_show_x_amin)
				anim_current = velocity.x > 0 ? &anim_right_sketch : &anim_left_sketch;
			else
				anim_current = velocity.y > 0 ? &anim_down_sketch : &anim_up_sketch;
		}
		else
		{
			if (is_show_x_amin)
				anim_current = velocity.x > 0 ? &anim_right : &anim_left;
			else
				anim_current = velocity.y > 0 ? &anim_down : &anim_up;
		}

		anim_current->on_update(delta);
	}

	void on_render(SDL_Renderer *renderer)
	{
		// 描画に必要な静的変数を定義
		static SDL_Rect rect;
		static SDL_Point point;
		static const Vector2 size_hp_bar = {40, 8};					 // HPバーのサイズ
		static const int offset_y = 2;								 // HPバーのY軸オフセット
		static const SDL_Color color_border = {116, 185, 124, 255};	 // HPバーの枠線の色（緑）
		static const SDL_Color color_content = {226, 255, 194, 255}; // HPバーの中身の色（淡緑）

		// 敵キャラクターの描画位置を計算
		point.x = (int)(position.x - size.x / 2);
		point.y = (int)(position.y - size.y / 2);
		// 現在のアニメーションフレームを描画
		anim_current->on_render(renderer, point);

		// HPバーを描画
		if (hp < max_hp)
		{
			rect.x = (int)(position.x - size_hp_bar.x / 2);
			rect.y = (int)(position.y - size.y / 2 - size_hp_bar.y - offset_y);
			rect.w = (int)(size_hp_bar.x * (hp / max_hp));
			rect.h = (int)(size_hp_bar.y);
			SDL_SetRenderDrawColor(renderer, color_content.r, color_content.g, color_content.b, color_content.a);
			SDL_RenderFillRect(renderer, &rect);

			rect.w = (int)size_hp_bar.x;
			SDL_SetRenderDrawColor(renderer, color_border.r, color_border.g, color_border.b, color_border.a);
			SDL_RenderDrawRect(renderer, &rect);
		}
	}

	void set_on_skill_released(SkillCallback on_skill_released)
	{
		this->on_skill_released = on_skill_released;
	}

	void increase_hp(double val)
	{
		hp += val;
		if (hp > max_hp)
		{
			hp = max_hp;
		}
	}

	void decrease_hp(double val)
	{
		hp -= val;
		if (hp <= 0)
		{
			hp = 0;
			is_valid = false;
		}
		is_show_sketch = true;
		timer_sketch.restart();
	}

	void slow_down()
	{
		speed = max_speed - 0.5;
		timer_restore_speed.set_wait_time(1);
		timer_restore_speed.restart();
	}

	void set_position(const Vector2 &position)
	{
		this->position = position;
	}

	void set_route(const Route *route)
	{
		this->route = route;

		refresh_position_target();
	}

	void make_invalid()
	{
		is_valid = false;
	}

	double get_hp() const
	{
		return hp;
	}

	const Vector2 &get_size() const
	{
		return size;
	}

	const Vector2 &get_position() const
	{
		return position;
	}

	const Vector2 &get_velocity() const
	{
		return velocity;
	}

	double get_damage() const
	{
		return damage;
	}

	double get_reward_ratio() const
	{
		return reward_ratio;
	}

	double get_recover_radius() const
	{
		return SIZE_TILE * recover_range;
	}

	double get_recover_intensity() const
	{
		return recover_intensity;
	}

	bool can_remove() const
	{
		return !is_valid;
	}

	// 敵の進行度（大きいほど防御点に近い）（防御塔の敵探知に使用）
	double get_route_process() const
	{
		if (route->get_idx_list().size() == 1)
		{
			return 1;
		}
		return (double)idx_target / (route->get_idx_list().size() - 1);
	}

	// 確率に基づいてコインを生成
	void try_spawn_coin_prop(const Vector2 &position, double ratio)
	{
		static CoinManager *instance = CoinManager::instance();

		std::random_device rd;
		std::mt19937 generator(rd());
		std::uniform_real_distribution<double> distribution(0.0, 1.0); // (0, 1)の乱数を生成

		if (distribution(generator) <= ratio)
		{
			instance->spawn_coin_prop(position);
		}
	}

protected:
	// 敵のサイズ
	Vector2 size;
	// スキル発動タイマー
	Timer timer_skill;

	// 四方向のアニメーション＆ヒットアニメーション
	Animation anim_up;
	Animation anim_down;
	Animation anim_left;
	Animation anim_right;
	Animation anim_up_sketch;
	Animation anim_down_sketch;
	Animation anim_left_sketch;
	Animation anim_right_sketch;

	// 敵の属性
	double hp = 0; // 現在のHP
	double max_hp = 0;
	double speed = 0;
	double max_speed = 0;
	double damage = 0;
	double reward_ratio = 0;
	double recover_interval = 0;
	double recover_range = 0;
	double recover_intensity = 0;

private:
	Vector2 position;  // 位置
	Vector2 velocity;  // 速度
	Vector2 direction; // 方向

	// 敵が倒されたかどうか
	bool is_valid = true;

	// ヒットアニメーションの再生時間
	Timer timer_sketch;

	// ヒットアニメーションを再生するかどうか
	bool is_show_sketch = false;

	// 現在のアニメーション
	Animation *anim_current = nullptr;

	// スキル発動のコールバック関数
	SkillCallback on_skill_released;

	// 速度回復の時間を保存（減速効果がある可能性）
	Timer timer_restore_speed;

	// 経路探索関連
	const Route *route = nullptr; // 経路
	int idx_target = 0;			  // 最終目標
	Vector2 position_target;	  // 移動の目標位置(ワールド座標)

private:
	void refresh_position_target()
	{
		const Route::IdxList &idx_list = route->get_idx_list();
		if (idx_target < idx_list.size())
		{
			const SDL_Point &point = idx_list[idx_target];
			static const SDL_Rect &rect_tile_map = ConfigManager::instance()->rect_tile_map;

			position_target.x = rect_tile_map.x + point.x * SIZE_TILE + SIZE_TILE / 2;
			position_target.y = rect_tile_map.y + point.y * SIZE_TILE + SIZE_TILE / 2;
		}
	}
};

#endif // !_ENEMY_H_
