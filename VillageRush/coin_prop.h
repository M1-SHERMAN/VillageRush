#ifndef _COIN_PROP_H_
#define _COIN_PROP_H_

#include "tile.h"
#include "vector2.h"
#include "timer.h"
#include "resources_manager.h"

#include <SDL.h>

class CoinProp
{
public:
	CoinProp()
	{
		// タイマーをワンショットに設定し、持続時間を interval_* 秒に設定
		timer_jump.set_one_shot(true);
		timer_jump.set_wait_time(interval_jump);
		timer_jump.set_on_timeout(
			[&]()
			{
				is_jumping = false; // ジャンプ終了、浮遊状態に入る
			});

		timer_disappear.set_one_shot(true);
		timer_disappear.set_wait_time(interval_disappear);
		timer_disappear.set_on_timeout(
			[&]()
			{
				is_valid = false; // コインが無効になり、削除待ち
			});

		// 速度を初期化、左右のどちらかをランダムに選択
		velocity.x = (rand() % 2 ? 1 : -1) * 2 * SIZE_TILE;
		velocity.y = -3 * SIZE_TILE;
	};

	~CoinProp() = default;

	void set_position(const Vector2 &position)
	{
		this->position = position;
	}

	const Vector2 &get_position() const
	{
		return position;
	}

	const Vector2 &get_size() const
	{
		return size;
	}

	void make_invalid()
	{
		is_valid = false;
	}

	bool can_remove()
	{
		return !is_valid;
	}

	void on_update(double delta)
	{
		// タイマーを更新
		timer_jump.on_update(delta);
		timer_disappear.on_update(delta);

		if (is_jumping)
		{
			// ジャンプ中は重力の影響を受ける
			velocity.y += gravity * delta;
		}
		else
		{
			// 浮遊中は水平速度が0、垂直速度は上下の浮遊をシミュレート
			velocity.x = 0;
			velocity.y = sin(SDL_GetTicks64() / 1000 * 4) * 30 * delta;
		}

		position += velocity * delta;
	}

	void on_render(SDL_Renderer *renderer)
	{
		// レンダリング領域の矩形を定義
		static SDL_Rect rect = {0, 0, (int)size.x, (int)size.y};

		// リソースマネージャーからコインのテクスチャを取得
		static SDL_Texture *tex_coin = ResourcesManager::instance()
										   ->get_texture_pool()
										   .find(ResID::Tex_Coin)
										   ->second;

		// レンダリング位置を設定
		rect.x = (int)(position.x - size.x / 2);
		rect.y = (int)(position.y - size.y / 2);

		// コインのテクスチャをレンダラーに描画
		SDL_RenderCopy(renderer, tex_coin, nullptr, &rect);
	}

private:
	Vector2 position; // コインの位置
	Vector2 velocity; // コインの速度（ジャンプ中|浮遊中）

	Timer timer_jump;
	Timer timer_disappear;

	bool is_valid = true;
	bool is_jumping = true;

	double gravity = 500;			// 重力加速度
	double interval_jump = 0.75;	// ジャンプの持続時間(秒)
	double interval_disappear = 10; // コインの存在時間(秒)
	Vector2 size = {16, 16};		// コインのサイズ(ピクセル)
};

#endif // !_COIN_PROP_H_
