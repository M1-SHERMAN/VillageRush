#ifndef _SHELL_BULLET_H_
#define _SHELL_BULLET_H_

#include "bullet.h"
#include "resources_manager.h"

class ShellBullet : public Bullet
{
public:
	ShellBullet()
	{
		// 弾丸のテクスチャを取得
		static SDL_Texture *tex_shell = ResourcesManager::instance()
											->get_texture_pool()
											.find(ResID::Tex_BulletShell)
											->second;

		// 爆発エフェクトのテクスチャを取得
		static SDL_Texture *tex_explode = ResourcesManager::instance()
											  ->get_texture_pool()
											  .find(ResID::Tex_EffectExplode)
											  ->second;

		// アニメーションインデックスを設定（2フレーム）
		static const std::vector<int> idx_list = {0, 1};				  // 弾丸アニメーションフレーム
		static const std::vector<int> idx_explode_list = {0, 1, 2, 3, 4}; // 爆発アニメーションフレーム

		// アニメーションのループ、間隔、フレームデータを設定
		// 砲弾
		animation.set_loop(true);
		animation.set_interval(0.1);
		animation.set_frame_data(tex_shell, 2, 1, idx_list);

		// 爆発
		animation_explode.set_loop(false);
		animation_explode.set_interval(0.1);
		animation_explode.set_frame_data(tex_explode, 5, 1, idx_list);
		animation_explode.set_on_finished(
			[&]() // 爆発アニメーション終了時のコールバック関数
			{
				make_invalid(); // 弾丸を無効にする
			});

		// 回転不可に設定
		can_rotate = false;
		// ダメージ範囲を設定
		damage_range = 96;
		// サイズを設定
		size.x = 48, size.y = 48;
	};

	~ShellBullet() = default;

	void on_update(double delta) override
	{
		// 弾丸がまだ衝突可能か（爆発していないか）を判断
		if (can_collide())
		{
			// まだ爆発していない場合、弾丸の飛行ロジックを続行
			Bullet::on_update(delta);
			return;
		}

		// 爆発している場合、爆発アニメーションを更新
		animation_explode.on_update(delta);
	}

	void on_render(SDL_Renderer *renderer) override
	{
		// 弾丸がまだ衝突可能か（爆発していないか）を判断
		if (can_collide())
		{
			// まだ爆発していない場合、弾丸の飛行アニメーションをレンダリング
			Bullet::on_render(renderer);
			return;
		}

		// 爆発している場合、爆発アニメーションの位置を計算してレンダリング
		static SDL_Point point;

		// 爆発アニメーションのレンダリング位置を計算
		point.x = (int)(position.x - 96 / 2);
		point.y = (int)(position.y - 96 / 2);

		// 爆発アニメーションをレンダリング
		animation_explode.on_render(renderer, point);
	};

	void on_collide(Enemy *enemy) override
	{
		// 衝突音効を取得して再生
		static const ResourcesManager::SoundPool &sound_pool = ResourcesManager::instance()->get_sound_pool();

		Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_ShellHit)->second, 0);

		// 衝突を無効にし、爆発状態に移行
		disable_collide();
	}

private:
	Animation animation_explode;
};

#endif // !_SHELL_BULLET_H_
