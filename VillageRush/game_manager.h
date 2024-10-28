#ifndef _GAME_MANAGER_H
#define _GAME_MANAGER_H

/**
 * @brief ゲーム全体を管理するクラス
 *
 * このクラスは、ゲームの全体的な流れと状態を制御します。
 * 各種マネージャーの初期化、更新、描画の統括、およびゲームループの管理を行います。
 *
 * 主な機能:
 * - ゲームの初期化と終了処理
 * - ゲームループの制御
 * - 各種マネージャー（敵、タワー、弾丸など）の管理
 * - ユーザー入力の処理
 * - シーン管理
 * - リソース（画像、音声など）の管理
 * - UI要素（ステータスバー、パネルなど）の管理
 *
 * このクラスは、シングルトンパターンを使用して実装されており、
 * ゲーム全体で唯一のインスタンスとして機能します。
 */

#include "manager.h"
#include "config_manager.h"
#include "resources_manager.h"
#include "enemy_manager.h"
#include "wave_manager.h"
#include "tower_manager.h"
#include "bullet_manager.h"
#include "player_manager.h"
#include "status_bar.h"
#include "panel.h"
#include "place_panel.h"
#include "upgrade_panel.h"
#include "banner.h"

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

class GameManager : public Manager<GameManager>
{
	friend class Manager<GameManager>;

public:
	int run(int argc, char **argv)
	{

		// BGMをフェードインで再生
		Mix_FadeInMusic(ResourcesManager::instance()->get_music_pool().find(ResID::Music_BGM)->second, -1, 1500);

		// SDLの高精度タイマーとタイマーの刻みを取得し、フレームレート制御に使用
		Uint64 last_counter = SDL_GetPerformanceCounter();
		const Uint64 counter_freq = SDL_GetPerformanceFrequency();

		while (!is_quit)
		{
			// ユーザー入力の処理
			while (SDL_PollEvent(&event))
			{
				on_input();
			}

			// フレームレートの制御
			Uint64 current_counter = SDL_GetPerformanceCounter();
			double delta = (double)(current_counter - last_counter) / counter_freq;
			last_counter = current_counter;
			if (delta * 1000.f < 1000.f / 60)
			{
				SDL_Delay((Uint32)(1000.f / 60 - delta * 1000));
			}

			// データの更新
			on_update(delta);

			// レンダリングの準備
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderClear(renderer);

			// 現在のフレームの内容をレンダリング
			on_render();

			// レンダリングの実行
			SDL_RenderPresent(renderer);
		}
		return 0;
	}

protected:
	GameManager()
	{
		// SDLと関連コンポーネントの初期化
		init_assert(!SDL_Init(SDL_INIT_EVERYTHING), u8"SDL2の初期化に失敗しました");
		init_assert(IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG), u8"SDL_imageの初期化に失敗しました");
		init_assert(Mix_Init(MIX_INIT_MP3), u8"SDL_mixerの初期化に失敗しました");
		init_assert(!TTF_Init(), u8"SDL_ttfの初期化に失敗しました");

		// オーディオの初期化
		Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

		// IMEのUIを表示するように設定
		SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

		// 設定情報の読み込み
		ConfigManager *config = ConfigManager::instance();
		init_assert(config->map.load("map.csv"), u8"ゲームマップの読み込みに失敗しました");
		init_assert(config->load_level_config("level.json"), u8"レベル設定の読み込みに失敗しました");
		init_assert(config->load_game_config("config.json"), u8"ゲーム設定の読み込みに失敗しました");

		// 設定情報を使用してウィンドウを作成
		window = SDL_CreateWindow(config->basic_template.window_title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
								  config->basic_template.window_width, config->basic_template.window_height, SDL_WINDOW_SHOWN);
		init_assert(window, u8"ゲームウィンドウの作成に失敗しました");

		// レンダラーの作成
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
		init_assert(renderer, u8"レンダラーの作成に失敗しました");

		// ゲームリソースの読み込み
		init_assert(ResourcesManager::instance()->load_from_file(renderer), u8"ゲームリソースの読み込みに失敗しました");

		// タイルマップテクスチャの生成
		init_assert(generate_tile_map_texture(), u8"タイルマップテクスチャの生成に失敗しました");

		// ステータスバーの位置設定
		status_bar.set_position(15, 15);

		// パネルの初期化
		place_panel = new PlacePanel();
		upgrade_panel = new UpgradePanel();

		// 結果バナーの初期化
		banner = new Banner();
	}

	~GameManager()
	{
		// リソースの解放
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);

		// SDLサブシステムの終了
		TTF_Quit();
		Mix_Quit();
		IMG_Quit();
		SDL_Quit();
	}

private:
	SDL_Event event;
	bool is_quit = false;

	SDL_Window *window = nullptr;
	SDL_Renderer *renderer = nullptr;

	SDL_Texture *tex_tile_map = nullptr;

	StatusBar status_bar;
	Panel *place_panel = nullptr;
	Panel *upgrade_panel = nullptr;

	Banner *banner = nullptr;

private:
	// 初期化時のアサーション
	void init_assert(bool flag, const char *err_msg)
	{
		if (flag)
			return;

		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, u8"ゲームの起動に失敗しました", err_msg, window);
		exit(-1);
	}

	// 入力処理
	void on_input()
	{
		// マウスクリックの中心位置と選択されたタイルのインデックスを静的に保存
		static SDL_Point pos_center;
		static SDL_Point idx_tile_selected;
		static ConfigManager *instance = ConfigManager::instance();

		// SDLイベントタイプに基づいて異なる入力処理を行う
		switch (event.type)
		{
		case SDL_QUIT:
			is_quit = true;
			break;
		case SDL_MOUSEBUTTONDOWN:

			// ゲームが終了している場合は処理をスキップ
			if (instance->is_game_over)
				break;

			// マウスの現在位置のタイルインデックスを取得
			if (get_cursor_idx_tile(idx_tile_selected, event.motion.x, event.motion.y))
			{
				// 選択されたタイルの中心座標を取得
				get_selected_tile_ceneter_pos(pos_center, idx_tile_selected);
				// 選択されたタイルがホームかどうかをチェック
				if (check_home(idx_tile_selected))
				{
					// ホームの場合、アップグレードパネルを表示
					upgrade_panel->set_idx_tile(idx_tile_selected);
					upgrade_panel->set_center_pos(pos_center);
					upgrade_panel->show();
				}
				// ホームでない場合、タワーを配置できるかチェック
				else if (can_place_tower(idx_tile_selected))
				{
					// タワーを配置できる場合、配置パネルを表示
					place_panel->set_idx_tile(idx_tile_selected);
					place_panel->set_center_pos(pos_center);
					place_panel->show();
				}
			}
			break;
		default:
			break;
		}

		// ゲームが終了していない場合、入力イベントを続ける
		if (!instance->is_game_over)
		{
			place_panel->on_input(event);
			upgrade_panel->on_input(event);
			PlayerManager::instance()->on_input(event);
		}
	}

	// ゲーム状態の更新
	void on_update(double delta)
	{
		static bool is_game_over_last_tick = false;

		static ConfigManager *instance = ConfigManager::instance();
		if (!instance->is_game_over)
		{
			// 各マネージャーとUIの更新
			status_bar.on_update(renderer);
			place_panel->on_update(renderer);
			upgrade_panel->on_update(renderer);

			WaveManager::instance()->on_update(delta);
			EnemyManager::instance()->on_update(delta);
			BulletManager::instance()->on_update(delta);
			TowerManager::instance()->on_update(delta);
			CoinManager::instance()->on_update(delta);
			PlayerManager::instance()->on_update(delta);

			return;
		}

		// 前のフレームでゲームが終了していなかったが、このフレームで終了した場合
		if (!is_game_over_last_tick && instance->is_game_over)
		{
			static const ResourcesManager::SoundPool &sounld_pool = ResourcesManager::instance()->get_sound_pool();

			// BGMをフェードアウトし、勝利または敗北の効果音を再生
			Mix_FadeOutMusic(1500);
			Mix_PlayChannel(-1, sounld_pool.find(instance->is_game_win ? ResID::Sound_Win : ResID::Sound_Loss)->second, 0);
		}

		is_game_over_last_tick = instance->is_game_over;

		// バナーの更新と表示終了チェック
		banner->on_update(delta);
		if (banner->check_end_display())
		{
			is_quit = true;
		}
	}

	// ゲーム画面のレンダリング
	void on_render()
	{
		static ConfigManager *instance = ConfigManager::instance();
		static SDL_Rect &rect_dst = instance->rect_tile_map;
		SDL_RenderCopy(renderer, tex_tile_map, nullptr, &rect_dst);

		// 各マネージャーのレンダリング
		EnemyManager::instance()->on_render(renderer);
		BulletManager::instance()->on_render(renderer);
		TowerManager::instance()->on_render(renderer);
		CoinManager::instance()->on_render(renderer);

		PlayerManager::instance()->on_render(renderer);
		if (!instance->is_game_over)
		{
			// ゲーム中のUIレンダリング
			place_panel->on_render(renderer);
			upgrade_panel->on_render(renderer);
			status_bar.on_render(renderer);

			return;
		}
		// ゲーム終了時のバナー表示
		int width_screen, height_screen;
		SDL_GetWindowSizeInPixels(window, &width_screen, &height_screen);
		banner->set_center_position({(double)width_screen / 2, (double)height_screen / 2});
		banner->on_render(renderer);
	}

	// タイルマップテクスチャの生成
	bool generate_tile_map_texture()
	{
		// マップとタイル情報の取得
		const Map &map = ConfigManager::instance()->map;
		const TileMap &tile_map = map.get_tile_map();
		SDL_Rect &rect_tile_map = ConfigManager::instance()->rect_tile_map;

		// タイルセットのテクスチャ取得
		SDL_Texture *tex_tile_set = ResourcesManager::instance()->get_texture_pool().find(ResID::Tex_Tileset)->second;

		// タイルセットの幅と高さを取得
		int width_tex_tile_set, height_tex_tile_set;
		SDL_QueryTexture(tex_tile_set, nullptr, nullptr, &width_tex_tile_set, &height_tex_tile_set);
		// タイルセットテクスチャの1行あたりのタイル数を計算
		int num_tile_single_line = (int)std::ceil((double)width_tex_tile_set / SIZE_TILE);

		// タイルマップテクスチャの幅と高さを計算（ピクセル単位）
		int width_tex_tile_map, height_tex_tile_map;
		width_tex_tile_map = (int)map.get_width() * SIZE_TILE;
		height_tex_tile_map = (int)map.get_height() * SIZE_TILE;

		// タイルマップのテクスチャを作成
		tex_tile_map = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
										 SDL_TEXTUREACCESS_TARGET, width_tex_tile_map, height_tex_tile_map);
		if (!tex_tile_map)
			return false;

		// タイルマップをウィンドウ中央に配置
		ConfigManager *config = ConfigManager::instance();
		rect_tile_map.x = (config->basic_template.window_width - width_tex_tile_map) / 2;
		rect_tile_map.y = (config->basic_template.window_height - height_tex_tile_map) / 2;
		rect_tile_map.w = width_tex_tile_map;
		rect_tile_map.h = height_tex_tile_map;

		// テクスチャのブレンドモードを設定
		SDL_SetTextureBlendMode(tex_tile_map, SDL_BLENDMODE_BLEND);

		// レンダリングターゲットをタイルマップテクスチャに設定
		SDL_SetRenderTarget(renderer, tex_tile_map);

		// マップを走査し、各タイルをレンダリング
		for (int y = 0; y < map.get_height(); y++)
		{
			for (int x = 0; x < map.get_width(); x++)
			{
				SDL_Rect rect_src;
				const Tile &tile = tile_map[y][x];

				// 現在のタイルの描画位置を計算
				const SDL_Rect &rect_dst =
					{
						x * SIZE_TILE, y * SIZE_TILE,
						SIZE_TILE, SIZE_TILE};

				rect_src =
					{
						// タイルセット内のタイル位置を計算し、レンダリング
						(tile.terrian % num_tile_single_line) * SIZE_TILE,
						(tile.terrian / num_tile_single_line) * SIZE_TILE,
						SIZE_TILE, SIZE_TILE};

				SDL_RenderCopy(renderer, tex_tile_set, &rect_src, &rect_dst);

				// タイルに装飾がある場合、追加でレンダリング
				if (tile.decoration >= 0)
				{
					rect_src =
						{
							(tile.decoration % num_tile_single_line) * SIZE_TILE,
							(tile.decoration / num_tile_single_line) * SIZE_TILE,
							SIZE_TILE, SIZE_TILE};
					SDL_RenderCopy(renderer, tex_tile_set, &rect_src, &rect_dst);
				};
			}
		}
		// 防御拠点タイルのレンダリング
		const SDL_Point &idx_home = map.get_idx_home();
		const SDL_Rect rect_dst =
			{
				idx_home.x * SIZE_TILE, idx_home.y * SIZE_TILE,
				SIZE_TILE, SIZE_TILE};
		SDL_RenderCopy(renderer, ResourcesManager::instance()->get_texture_pool().find(ResID::Tex_Home)->second, nullptr, &rect_dst);

		// レンダリングを終了し、デフォルトのレンダリングターゲットに戻す
		SDL_SetRenderTarget(renderer, nullptr);

		return true;
	}

	// プレイヤーが選択したタイルがホームタイルかどうかをチェック
	bool check_home(const SDL_Point &idx_tile_selected)
	{
		static const Map &map = ConfigManager::instance()->map;
		static const SDL_Point &idx_home = map.get_idx_home();
		return (idx_home.x == idx_tile_selected.x && idx_home.y == idx_tile_selected.y);
	}

	// 画面上の座標からタイルのインデックスを取得
	bool get_cursor_idx_tile(SDL_Point &idx_tile_selected, int screen_x, int screen_y) const
	{
		static const Map &map = ConfigManager::instance()->map;
		static const SDL_Rect &rect_tile_map = ConfigManager::instance()->rect_tile_map;

		// マウス位置がタイルマップ領域外の場合はfalseを返す
		if (screen_x < rect_tile_map.x || screen_x > rect_tile_map.x + rect_tile_map.w || screen_y < rect_tile_map.y || screen_y > rect_tile_map.y + rect_tile_map.h)
			return false;

		// 選択されたタイルのインデックス座標を計算し、マップの幅と高さを超えないように制限
		idx_tile_selected.x = std::min((screen_x - rect_tile_map.x) / SIZE_TILE, (int)map.get_width() - 1);
		idx_tile_selected.y = std::min((screen_y - rect_tile_map.y) / SIZE_TILE, (int)map.get_height() - 1);

		return true;
	}

	// 指定されたタイルに防御塔を配置できるかどうかをチェック
	bool can_place_tower(const SDL_Point &idx_tile_selected) const
	{

		static const Map &map = ConfigManager::instance()->map;
		const Tile &tile = map.get_tile_map()[idx_tile_selected.y][idx_tile_selected.x];

		// タイルに装飾がなく、方向制限がなく、防御塔がない場合にtrueを返す
		return (tile.decoration < 0 && tile.direction == Tile::Direction::None && !tile.has_tower);
	}

	// プレイヤーが選択したタイルの中心位置を取得
	void get_selected_tile_ceneter_pos(SDL_Point &pos, const SDL_Point &idx_tile_selected) const
	{
		static const SDL_Rect &rect_tile_map = ConfigManager::instance()->rect_tile_map;

		// 選択されたタイルの中心位置を計算
		pos.x = rect_tile_map.x + idx_tile_selected.x * SIZE_TILE + SIZE_TILE / 2;
		pos.y = rect_tile_map.y + idx_tile_selected.y * SIZE_TILE + SIZE_TILE / 2;
	}
};

#endif // !_GAME_MANAGER_H
