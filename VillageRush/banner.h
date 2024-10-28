#pragma once
#ifndef _BANNER_H_
#define _BANNER_H_

#include "vector2.h"
#include "timer.h"
#include "config_manager.h"
#include "resources_manager.h"

#include <SDL.h>

class Banner
{
public:
	Banner()
	{
		size_foreground = {646, 215};
		size_background = {1282, 209};

		timer_display.set_one_shot(true);
		timer_display.set_wait_time(5);
		timer_display.set_on_timeout(
			[&]()
			{
				is_end_display = true;
			});
	};

	~Banner() = default;

	// 中心位置を設定
	void set_center_position(const Vector2 &pos)
	{
		pos_center = pos;
	}

	// アップデート
	void on_update(double delta)
	{
		// タイマーを更新
		timer_display.on_update(delta);

		// リソースマネージャーからテクスチャプールを取得
		const ResourcesManager::TexturePool &tex_pool = ResourcesManager::instance()->get_texture_pool();
		// コンフィグマネージャーのインスタンスを取得
		const ConfigManager *instance = ConfigManager::instance();

		// ゲームの勝敗に応じて適切なフォアグラウンドテクスチャを設定
		tex_foreground = tex_pool.find(instance->is_game_win ? ResID::Tex_UIWinText : ResID::Tex_UILossText)->second;
		// バックグラウンドテクスチャを設定
		tex_background = tex_pool.find(ResID::Tex_UIGameOverBar)->second;
	}

	// レンダリング
	void on_render(SDL_Renderer *renderer)
	{
		// 描画用の矩形を静的変数として定義
		static SDL_Rect rect_dst;

		// バックグラウンドの描画位置とサイズを設定（中心位置からの相対位置）
		rect_dst.x = (int)(pos_center.x - size_background.x / 2);
		rect_dst.y = (int)(pos_center.y - size_background.y / 2);
		rect_dst.w = size_background.x, rect_dst.h = size_background.y;
		// バックグラウンドテクスチャを描画
		SDL_RenderCopy(renderer, tex_background, nullptr, &rect_dst);

		// フォアグラウンドの描画位置とサイズを設定（中心位置からの相対位置）
		rect_dst.x = (int)(pos_center.x - size_foreground.x / 2);
		rect_dst.y = (int)(pos_center.y - size_foreground.y / 2);
		rect_dst.w = size_foreground.x, rect_dst.h = size_foreground.y;
		// フォアグラウンドテクスチャを描画
		SDL_RenderCopy(renderer, tex_foreground, nullptr, &rect_dst);
	}

	// 表示終了チェック
	bool check_end_display()
	{
		return is_end_display;
	}

private:
	Vector2 pos_center;

	Vector2 size_foreground;
	Vector2 size_background;

	SDL_Texture *tex_foreground = nullptr;
	SDL_Texture *tex_background = nullptr;

	Timer timer_display;
	bool is_end_display = false;
};

#endif // !_BANNER_H_
