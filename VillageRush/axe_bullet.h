#ifndef _AXE_BULLET_H_
#define _AXE_BULLET_H_

#include "bullet.h"
#include "resources_manager.h"

class AxeBullet : public Bullet
{
public:
	AxeBullet()
	{
		// 弾のテクスチャを取得
		static SDL_Texture *tex_axe = ResourcesManager::instance()
										  ->get_texture_pool()
										  .find(ResID::Tex_BulletAxe)
										  ->second;

		// アニメーションインデックスを設定（八フレーム）
		static const std::vector<int> idx_list = {0, 1, 2, 3, 4, 5, 6, 7};

		// アニメーションのループ、間隔、フレームを設定
		animation.set_loop(true);
		animation.set_interval(0.1);
		animation.set_frame_data(tex_axe, 4, 2, idx_list);

		// 回転不可能に設定
		can_rotate = false;
		// サイズを設定
		size.x = 48, size.y = 48;
	};

	~AxeBullet() = default;

	void on_collide(Enemy *enemy) override
	{
		// サウンドエフェクトを取得
		static const ResourcesManager::SoundPool &sound_pool = ResourcesManager::instance()->get_sound_pool();

		// 3種類のランダムサウンドエフェクトを設定
		switch (rand() % 3)
		{
		case 0:
			Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_AxeHit_1)->second, 0);
			break;
		case 1:
			Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_AxeHit_2)->second, 0);
			break;
		case 2:
			Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_AxeHit_3)->second, 0);
			break;
		}

		// スローダウン効果を設定
		enemy->slow_down();

		// 親クラスのメソッドを呼び出し、デフォルトの操作を実行
		Bullet::on_collide(enemy);
	}

private:
};

#endif // !_AXE_BULLET_H_
