#ifndef _BULLET_H_
#define _BULLET_H_

/**
 * @brief 弾丸クラス
 *
 * このクラスは、ゲーム内の弾丸オブジェクトを表現します。
 * 弾丸の移動、ダメージ処理、アニメーション、および敵との衝突検出などの機能を提供します。
 *
 * 主な機能:
 * - 弾丸の移動と位置の更新
 * - ダメージ値の設定と取得
 * - アニメーションの制御
 * - 敵との衝突検出
 * - 弾丸の有効性管理
 *
 * このクラスは、タワーディフェンスゲームにおいて、
 * プレイヤーや防御塔が発射する弾丸の挙動を制御するために使用されます。
 */

#include "vector2.h"
#include "enemy.h"
#include "animation.h"
#include "config_manager.h"

class Bullet
{
public:
	Bullet() = default;

	~Bullet() = default;

	// 弾のスピードと回転角度を設定
	void set_velocity_and_rotation(const Vector2 &velocity)
	{
		this->velocity = velocity;

		// 回転が許可されている場合、スピードに基づいて回転角度を計算
		if (can_rotate)
		{
			double randian = std::atan2(velocity.y, velocity.x); // ラジアンを計算
			angle_anim_rotate = randian * 180 / 3.1415926535;	 // ラジアンを度に変換
		}
	}

	// 弾の位置を設定
	void set_position(const Vector2 &position)
	{
		this->position = position;
	}

	// 弾のダメージ値を設定
	void set_damage(double damage)
	{
		this->damage = damage;
	}

	// 弾のサイズを取得
	const Vector2 &get_size() const
	{
		return size;
	}

	// 弾の位置を取得
	const Vector2 &get_position() const
	{
		return position;
	}

	// 弾のダメージ値を取得
	const double get_damage() const
	{
		return damage;
	}

	// 弾のダメージ範囲を取得
	const double get_damage_range() const
	{
		return damage_range;
	}

	// 弾の衝突検出機能を無効化
	void disable_collide()
	{
		is_collisionable = false;
	}

	// 弾が衝突可能かどうかを判定
	bool can_collide() const
	{
		return is_collisionable;
	}

	// 弾を無効としてマーク
	void make_invalid()
	{
		is_valid = false;
		is_collisionable = false;
	}

	// 弾が削除可能かどうかを判定
	bool can_remove() const
	{
		return !is_valid;
	}

	// 弾の状態を毎フレーム更新
	virtual void on_update(double delta)
	{
		animation.on_update(delta);	  // アニメーションを更新
		position += velocity * delta; // スピードに基づいて弾の位置を更新

		// マップの境界矩形を取得（境界チェック用）
		static const SDL_Rect &rect_map = ConfigManager::instance()->rect_tile_map;

		// 弾がマップの境界を超えているかチェック、超えていれば無効とマーク
		if (position.x - size.x / 2 <= rect_map.x || position.x + size.x / 2 >= rect_map.x + rect_map.w || position.y - size.y / 2 <= rect_map.y || position.y + size.y / 2 >= rect_map.y + rect_map.h)
		{
			is_valid = false;
		}
	}

	// 弾をレンダリング
	virtual void on_render(SDL_Renderer *renderer)
	{
		static SDL_Point point;

		// 弾のレンダリング位置を計算
		point.x = (int)(position.x - size.x / 2);
		point.y = (int)(position.y - size.y / 2);

		// アニメーション内容をレンダリング（回転角度付き）
		animation.on_render(renderer, point, angle_anim_rotate);
	}

	// 弾の衝突ロジックを処理
	virtual void on_collide(Enemy *enemy)
	{
		is_valid = false;
		is_collisionable = false;
	}

protected:
	Vector2 size;	  // サイズ
	Vector2 velocity; // スピード
	Vector2 position; // 位置

	Animation animation;	 // アニメーション内容
	bool can_rotate = false; // 回転可能かどうか

	double damage = 0;		  // ダメージ
	double damage_range = -1; // 範囲

private:
	bool is_valid = true;		  // 有効かどうか
	bool is_collisionable = true; // 衝突可能かどうか（例：爆発アニメーション再生中は衝突不可）
	double angle_anim_rotate = 0; // 回転角度
};

#endif // !_BULLET_H_
