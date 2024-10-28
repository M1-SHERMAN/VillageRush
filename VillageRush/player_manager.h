#ifndef _PLAYER_MANAGER_H_
#define _PLAYER_MANAGER_H_

#include "manager.h"
#include "resources_manager.h"
#include "config_manager.h"
#include "enemy_manager.h"
#include "vector2.h"
#include "animation.h"
#include "timer.h"
#include "facing.h"
#include "tile.h"
#include "map.h"

#include <SDL.h>

class PlayerManager : public Manager<PlayerManager>
{
	friend class Manager<PlayerManager>;

public:
	void on_input(const SDL_Event &event)
	{
		switch (event.type)
		{
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym)
			{
			case SDLK_a:
				is_move_left = true;
				break;
			case SDLK_d:
				is_move_right = true;
				break;
			case SDLK_w:
				is_move_up = true;
				break;
			case SDLK_s:
				is_move_down = true;
				break;
			case SDLK_j:
				on_release_flash();
				break;
			case SDLK_k:
				on_release_impact();
				break;
			default:
				break;
			}
			break;
		case SDL_KEYUP:
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_a:
				is_move_left = false;
				break;
			case SDLK_d:
				is_move_right = false;
				break;
			case SDLK_w:
				is_move_up = false;
				break;
			case SDLK_s:
				is_move_down = false;
				break;
			default:
				break;
			}
			break;
		}
		default:
			break;
		}
	}

	void on_update(double delta)
	{
		timer_auto_increase_mp.on_update(delta);
		timer_release_flash_cd.on_update(delta);

		Vector2 direction =
			Vector2(is_move_right - is_move_left, is_move_down - is_move_up).normalize(); // ブール値をintに変換し、単位ベクトルに変換
		velocity = direction * speed * SIZE_TILE;

		if (!is_releasing_flash && !is_releasing_impact)
		{
			position += velocity * delta;

			const SDL_Rect &rect_map = ConfigManager::instance()->rect_tile_map;
			// プレイヤーがマップの境界を越えないようにする
			if (position.x < rect_map.x)
				position.x = rect_map.x;
			if (position.x > rect_map.x + rect_map.w)
				position.x = rect_map.x + rect_map.w;
			if (position.y < rect_map.y)
				position.y = rect_map.y;
			if (position.y > rect_map.y + rect_map.h)
				position.y = rect_map.y + rect_map.h;

			// 速度に基づいて向きを更新、水平方向のアニメーションを優先
			if (velocity.y > 0)
				facing = Facing::Down;
			if (velocity.y < 0)
				facing = Facing::Up;
			if (velocity.x > 0)
				facing = Facing::Right;
			if (velocity.x < 0)
				facing = Facing::Left;

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
			default:
				break;
			}
		}
		else
		{
			switch (facing)
			{
			case Left:
				anim_current = &anim_attack_left;
				break;
			case Right:
				anim_current = &anim_attack_right;
				break;
			case Up:
				anim_current = &anim_attack_up;
				break;
			case Down:
				anim_current = &anim_attack_down;
				break;
			default:
				break;
			}
		}
		anim_current->on_update(delta);

		// フラッシュの解放（通常攻撃）
		if (is_releasing_flash)
		{
			anim_effect_flash_current->on_update(delta);

			EnemyManager::EnemyList &enemy_list = EnemyManager::instance()->get_enemy_list();

			for (Enemy *enemy : enemy_list)
			{
				if (enemy->can_remove())
					continue;

				const Vector2 &position = enemy->get_position();
				// ヒットボックスに基づく攻撃判定
				if (position.x >= rect_hitbox_flash.x && position.x <= rect_hitbox_flash.x + rect_hitbox_flash.w && position.y >= rect_hitbox_flash.y && position.y <= rect_hitbox_flash.y + rect_hitbox_flash.h)
				{
					enemy->decrease_hp(ConfigManager::instance()->player_template.normal_attack_damage * delta);
					if (enemy->can_remove())
					{
						enemy->try_spawn_coin_prop(position, enemy->get_reward_ratio());
					}
				}
			}
		}

		// インパクトの解放（特殊攻撃）
		if (is_releasing_impact)
		{
			anim_effect_impact_current->on_update(delta);

			EnemyManager::EnemyList &enemy_list = EnemyManager::instance()->get_enemy_list();

			for (Enemy *enemy : enemy_list)
			{
				if (enemy->can_remove())
					continue;

				const Vector2 &position = enemy->get_position();
				if (position.x >= rect_hitbox_impact.x && position.x <= rect_hitbox_impact.x + rect_hitbox_impact.w && position.y >= rect_hitbox_impact.y && position.y <= rect_hitbox_impact.y + rect_hitbox_impact.h)
				{
					enemy->decrease_hp(ConfigManager::instance()->player_template.skill_damage * delta);
					enemy->slow_down();
					if (enemy->can_remove())
					{
						enemy->try_spawn_coin_prop(position, enemy->get_reward_ratio());
					}
				}
			}
		}

		// コインを拾う
		CoinManager::CoinPropList &coin_prop_list = CoinManager::instance()->get_coin_prop_list();
		static const ResourcesManager::SoundPool &sound_pool = ResourcesManager::instance()->get_sound_pool();
		// すべてのコインを走査
		for (CoinProp *coin_prop : coin_prop_list)
		{
			if (coin_prop->can_remove())
				continue;

			const Vector2 &pos_coin_prop = coin_prop->get_position();
			// 衝突が発生したかどうかを判定（プレイヤーの矩形とコインの中心位置）
			if (pos_coin_prop.x >= position.x - size.x / 2 && pos_coin_prop.x <= position.x + size.x / 2 && pos_coin_prop.y >= position.y - size.y / 2 && pos_coin_prop.y <= position.y + size.y / 2)
			{
				coin_prop->make_invalid();
				CoinManager::instance()->increase_coin(15);

				Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_Coin)->second, 0);
			}
		}
	}

	void on_render(SDL_Renderer *renderer)
	{
		static SDL_Point point;

		// プレイヤーの現在のアニメーションをレンダリング
		point.x = (int)(position.x - size.x / 2);
		point.y = (int)(position.y - size.y / 2);
		anim_current->on_render(renderer, point);

		// フラッシュアニメーションをレンダリング
		if (is_releasing_flash)
		{
			point.x = rect_hitbox_flash.x;
			point.y = rect_hitbox_flash.y;
			anim_effect_flash_current->on_render(renderer, point);
		}

		// インパクトアニメーションをレンダリング
		if (is_releasing_impact)
		{
			point.x = rect_hitbox_impact.x;
			point.y = rect_hitbox_impact.y;
			anim_effect_impact_current->on_render(renderer, point);
		}
	}

	// 現在のMPを取得
	double get_current_mp() const
	{
		return mp;
	}

public:
	PlayerManager()
	{
		// MPを自動回復するタイマー、0.1秒ごとに発動、ループしない
		timer_auto_increase_mp.set_one_shot(false);
		timer_auto_increase_mp.set_wait_time(0.1);
		timer_auto_increase_mp.set_on_timeout(
			[&]()
			{
				double interval = ConfigManager::instance()->player_template.skill_interval;
				mp = std::min(mp + 100 / (interval / 0.1), 100.0);
			});

		// フラッシュ解放のクールダウンを計算するタイマー、設定ファイルに基づいて長さを決定、ループする
		timer_release_flash_cd.set_one_shot(true);
		timer_release_flash_cd.set_wait_time(
			ConfigManager::instance()->player_template.skill_interval);
		timer_release_flash_cd.set_on_timeout(
			[&]()
			{
				can_release_flash = true;
			});

		// テクスチャプールを取得し、それぞれのアニメーションを設定
		const ResourcesManager::TexturePool &tex_pool = ResourcesManager::instance()->get_texture_pool();
		// アイドルと攻撃アニメーション
		SDL_Texture *tex_player = tex_pool.find(ResID::Tex_Player)->second;

		anim_idle_up.set_loop(true);
		anim_idle_up.set_interval(0.1);
		anim_idle_up.set_frame_data(tex_player, 4, 8, {4, 5, 6, 7});
		anim_idle_down.set_loop(true);
		anim_idle_down.set_interval(0.1);
		anim_idle_down.set_frame_data(tex_player, 4, 8, {0, 1, 2, 3});
		anim_idle_left.set_loop(true);
		anim_idle_left.set_interval(0.1);
		anim_idle_left.set_frame_data(tex_player, 4, 8, {8, 9, 10, 11});
		anim_idle_right.set_loop(true);
		anim_idle_right.set_interval(0.1);
		anim_idle_right.set_frame_data(tex_player, 4, 8, {12, 13, 14, 15});

		anim_attack_up.set_loop(true);
		anim_attack_up.set_interval(0.1);
		anim_attack_up.set_frame_data(tex_player, 4, 8, {20, 21});
		anim_attack_down.set_loop(true);
		anim_attack_down.set_interval(0.1);
		anim_attack_down.set_frame_data(tex_player, 4, 8, {16, 17});
		anim_attack_left.set_loop(true);
		anim_attack_left.set_interval(0.1);
		anim_attack_left.set_frame_data(tex_player, 4, 8, {24, 25});
		anim_attack_right.set_loop(true);
		anim_attack_right.set_interval(0.1);
		anim_attack_right.set_frame_data(tex_player, 4, 8, {28, 29});

		// スキル解放アニメーション
		anim_effect_flash_up.set_loop(false);
		anim_effect_flash_up.set_interval(0.1);
		anim_effect_flash_up.set_frame_data(tex_pool.find(ResID::Tex_EffectFlash_Up)->second, 5, 1, {0, 1, 2, 3, 4});
		anim_effect_flash_up.set_on_finished([&]()
											 { is_releasing_flash = false; });
		anim_effect_flash_down.set_loop(false);
		anim_effect_flash_down.set_interval(0.1);
		anim_effect_flash_down.set_frame_data(tex_pool.find(ResID::Tex_EffectFlash_Down)->second, 5, 1, {4, 3, 2, 1, 0});
		anim_effect_flash_down.set_on_finished([&]()
											   { is_releasing_flash = false; });
		anim_effect_flash_left.set_loop(false);
		anim_effect_flash_left.set_interval(0.1);
		anim_effect_flash_left.set_frame_data(tex_pool.find(ResID::Tex_EffectFlash_Left)->second, 1, 5, {4, 3, 2, 1, 0});
		anim_effect_flash_left.set_on_finished([&]()
											   { is_releasing_flash = false; });
		anim_effect_flash_right.set_loop(false);
		anim_effect_flash_right.set_interval(0.1);
		anim_effect_flash_right.set_frame_data(tex_pool.find(ResID::Tex_EffectFlash_Right)->second, 1, 5, {0, 1, 2, 3, 4});
		anim_effect_flash_right.set_on_finished([&]()
												{ is_releasing_flash = false; });

		anim_effect_impact_up.set_loop(false);
		anim_effect_impact_up.set_interval(0.1);
		anim_effect_impact_up.set_frame_data(tex_pool.find(ResID::Tex_EffectImpact_Up)->second, 5, 1, {0, 1, 2, 3, 4});
		anim_effect_impact_up.set_on_finished([&]()
											  { is_releasing_impact = false; });
		anim_effect_impact_down.set_loop(false);
		anim_effect_impact_up.set_interval(0.1);
		anim_effect_impact_down.set_frame_data(tex_pool.find(ResID::Tex_EffectImpact_Down)->second, 5, 1, {4, 3, 2, 1, 0});
		anim_effect_impact_down.set_on_finished([&]()
												{ is_releasing_impact = false; });
		anim_effect_impact_left.set_loop(false);
		anim_effect_impact_up.set_interval(0.1);
		anim_effect_impact_left.set_frame_data(tex_pool.find(ResID::Tex_EffectImpact_Left)->second, 1, 5, {4, 3, 2, 1, 0});
		anim_effect_impact_left.set_on_finished([&]()
												{ is_releasing_impact = false; });
		anim_effect_impact_right.set_loop(false);
		anim_effect_impact_up.set_interval(0.1);
		anim_effect_impact_right.set_frame_data(tex_pool.find(ResID::Tex_EffectImpact_Right)->second, 1, 5, {0, 1, 2, 3, 4});
		anim_effect_impact_right.set_on_finished([&]()
												 { is_releasing_impact = false; });

		// 初期位置の初期化（防御点の左側に配置）
		static const SDL_Point &idx_home = ConfigManager::instance()->map.get_idx_home();
		position.x = idx_home.x * SIZE_TILE - 48;
		position.y = idx_home.y * SIZE_TILE;

		// 速度の初期化
		speed = ConfigManager::instance()->player_template.speed;

		// サイズの設定
		size.x = 96, size.y = 96;
	};
	~PlayerManager() = default;

private:
	// サイズ、方向、速度の定義
	Vector2 size;
	Vector2 position;
	Vector2 velocity;

	// 衝突ボックス
	SDL_Rect rect_hitbox_flash = {0};
	SDL_Rect rect_hitbox_impact = {0};

	// 基本属性
	double mp = 100;
	double speed = 0;

	// スキル解放関連
	bool can_release_flash = true;	  // フラッシュを解放できるかどうか
	bool is_releasing_flash = false;  // フラッシュを解放中かどうか
	bool is_releasing_impact = false; // インパクトを解放中かどうか

	// 移動関連
	bool is_move_up = false;
	bool is_move_down = false;
	bool is_move_left = false;
	bool is_move_right = false;

	/* アニメーション関連 */
	// アイドルアニメーション
	Animation anim_idle_up;
	Animation anim_idle_down;
	Animation anim_idle_left;
	Animation anim_idle_right;
	// 攻撃アニメーション
	Animation anim_attack_up;
	Animation anim_attack_down;
	Animation anim_attack_left;
	Animation anim_attack_right;
	Animation *anim_current = &anim_idle_right;
	// フラッシュエフェクト
	Animation anim_effect_flash_up;
	Animation anim_effect_flash_down;
	Animation anim_effect_flash_left;
	Animation anim_effect_flash_right;
	Animation *anim_effect_flash_current = nullptr;
	// インパクトエフェクト
	Animation anim_effect_impact_up;
	Animation anim_effect_impact_down;
	Animation anim_effect_impact_left;
	Animation anim_effect_impact_right;
	Animation *anim_effect_impact_current = nullptr;

	// タイマー
	Timer timer_release_flash_cd;
	Timer timer_auto_increase_mp;

	// 現在の向きの設定
	Facing facing = Facing::Left;

private:
	void on_release_flash()
	{
		if (!can_release_flash || is_releasing_flash)
			return;

		switch (facing)
		{
		case Left:
			anim_effect_flash_current = &anim_effect_flash_left;
			rect_hitbox_flash.x = (int)(position.x - size.x / 2 - 280); // ヒットボックスを左側に配置、中心位置から300px離れる
			rect_hitbox_flash.y = (int)(position.y - 68 / 2);			// 高さを中央に配置
			rect_hitbox_flash.w = 300, rect_hitbox_flash.h = 68;
			break;
		case Right:
			anim_effect_flash_current = &anim_effect_flash_right;
			rect_hitbox_flash.x = (int)(position.x + size.x / 2); // 右端に合わせる
			rect_hitbox_flash.y = (int)(position.y - 68 / 2);
			rect_hitbox_flash.w = 300, rect_hitbox_flash.h = 68;
			break;
		case Up:
			anim_effect_flash_current = &anim_effect_flash_up;
			rect_hitbox_flash.x = (int)(position.x - 68 / 2);			// x位置を中央に配置
			rect_hitbox_flash.y = (int)(position.y - size.y / 2 - 280); // yをキャラクターの上端から300px上に配置
			rect_hitbox_flash.w = 68, rect_hitbox_flash.h = 300;
			break;
		case Down:
			anim_effect_flash_current = &anim_effect_flash_down;
			rect_hitbox_flash.x = (int)(position.x - 68 / 2);	  // x位置を中央に配置
			rect_hitbox_flash.y = (int)(position.y + size.y / 2); // 下端に合わせる
			rect_hitbox_flash.w = 68, rect_hitbox_flash.h = 300;
			break;
		default:
			break;
		}

		is_releasing_flash = true;
		anim_effect_flash_current->reset();
		timer_release_flash_cd.restart();

		static const ResourcesManager::SoundPool &sound_pool = ResourcesManager::instance()->get_sound_pool();
		Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_Flash)->second, 0);
	}

	void on_release_impact()
	{
		if (mp < 100 || is_releasing_impact)
			return;

		switch (facing)
		{
		case Left:
			anim_effect_impact_current = &anim_effect_impact_left;
			rect_hitbox_impact.x = (int)(position.x - size.x / 2 - 40);
			rect_hitbox_impact.y = (int)(position.y - 140 / 2);
			rect_hitbox_impact.w = 60, rect_hitbox_impact.h = 140;
			break;
		case Right:
			anim_effect_impact_current = &anim_effect_impact_right;
			rect_hitbox_impact.x = (int)(position.x + size.x / 2);
			rect_hitbox_impact.y = (int)(position.y - 140 / 2);
			rect_hitbox_impact.w = 60, rect_hitbox_impact.h = 140;
			break;
		case Up:
			anim_effect_impact_current = &anim_effect_impact_up;
			rect_hitbox_impact.x = (int)(position.x - 140 / 2);
			rect_hitbox_impact.y = (int)(position.y - size.y / 2 - 40);
			rect_hitbox_impact.w = 140, rect_hitbox_impact.h = 60;
			break;
		case Down:
			anim_effect_impact_current = &anim_effect_impact_down;
			rect_hitbox_impact.x = (int)(position.x - 140 / 2);
			rect_hitbox_impact.y = (int)(position.y + size.y / 2);
			rect_hitbox_impact.w = 140, rect_hitbox_impact.h = 60;
			break;
		default:
			break;
		}

		mp = 0;
		is_releasing_impact = true;
		anim_effect_impact_current->reset();

		static const ResourcesManager::SoundPool &sound_pool = ResourcesManager::instance()->get_sound_pool();
		Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_Impact)->second, 0);
	}
};

#endif // !_PLAYER_MANAGER_H_
