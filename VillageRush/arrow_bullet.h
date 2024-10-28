#ifndef _ARROW_BULLET_H_
#define _ARROW_BULLET_H_

#include "bullet.h"
#include "resources_manager.h"

class ArrowBullet : public Bullet
{
public:
	ArrowBullet()
	{
		// 弾のテクスチャを取得
		static SDL_Texture *tex_arrow = ResourcesManager::instance()
											->get_texture_pool()
											.find(ResID::Tex_BulletArrow)
											->second;

		// アニメーションインデックスを設定（2フレーム）
		static const std::vector<int> idx_list = {0, 1};

		// アニメーションのループ、間隔、フレームを設定
		animation.set_loop(true);
		animation.set_interval(0.1);
		animation.set_frame_data(tex_arrow, 2, 1, idx_list);

		// 回転可能に設定
		can_rotate = true;
		// サイズを設定
		size.x = 48, size.y = 48;
	}
	~ArrowBullet() = default;

	void on_collide(Enemy *enemy) override
	{
		// サウンドエフェクトを取得
		static const ResourcesManager::SoundPool &sound_pool = ResourcesManager::instance()->get_sound_pool();

		// 3種類のランダムサウンドエフェクトを設定
		switch (rand() % 3)
		{
		case 0:
			Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_ArrowHit_1)->second, 0);
			break;
		case 1:
			Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_ArrowHit_2)->second, 0);
			break;
		case 2:
			Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_ArrowHit_3)->second, 0);
			break;
		}

		// 親クラスのメソッドを呼び出し、デフォルトの操作を実行
		Bullet::on_collide(enemy);
	}
};

#endif // !_ARROW_BULLET_H_
