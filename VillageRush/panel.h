#ifndef _PANEL_H_
#define _PANEL_H_

#include "resources_manager.h"
#include "tile.h"

#include <SDL.h>
#include <string>

class Panel
{
public:
	Panel()
	{
		// 選択カーソルのテクスチャを初期化
		tex_select_cursor = ResourcesManager::instance()->get_texture_pool().find(ResID::Tex_UISelectCursor)->second;
	}

	~Panel()
	{
		SDL_DestroyTexture(tex_text_background);
		SDL_DestroyTexture(tex_text_foreground);
	}

	// パネルを表示
	void show()
	{
		visible = true;
	}

	// 選択されたタイルのインデックスを設定
	void set_idx_tile(const SDL_Point &idx)
	{
		idx_tile_selected = idx;
	}

	// パネルの中心位置を設定
	void set_center_pos(const SDL_Point &pos)
	{
		center_pos = pos;
	}

	// マウスの移動やクリックなどの入力イベントを処理
	void on_input(const SDL_Event &event)
	{
		// パネルが表示されていない場合は入力を処理しない
		if (!visible)
			return;

		switch (event.type)
		{
		case SDL_MOUSEMOTION:
		{
			// マウス位置を取得し、衝突検出用のターゲット領域を定義
			SDL_Point pos_cursor = {event.motion.x, event.motion.y};
			SDL_Rect rect_target = {0, 0, size_button, size_button};

			// マウスが上部領域内にあるかチェック
			rect_target.x = center_pos.x - width / 2 + offset_top.x;
			rect_target.y = center_pos.y - height / 2 + offset_top.y;
			if (SDL_PointInRect(&pos_cursor, &rect_target))
			{
				hover_target = HoveredTarget::Top;
				return;
			}

			// マウスが左側領域内にあるかチェック
			rect_target.x = center_pos.x - width / 2 + offset_left.x;
			rect_target.y = center_pos.y - height / 2 + offset_left.y;
			if (SDL_PointInRect(&pos_cursor, &rect_target))
			{
				hover_target = HoveredTarget::Left;
				return;
			}

			// マウスが右側領域内にあるかチェック
			rect_target.x = center_pos.x - width / 2 + offset_right.x;
			rect_target.y = center_pos.y - height / 2 + offset_right.y;
			if (SDL_PointInRect(&pos_cursor, &rect_target))
			{
				hover_target = HoveredTarget::Right;
				return;
			}

			// どのターゲット領域内にもない場合、ホバーなしに設定
			hover_target = HoveredTarget::None;
		}
		break;
		case SDL_MOUSEBUTTONUP:
		{
			// マウスボタンリリースイベントを処理し、ホバー領域に応じて適切なクリックイベントをトリガー
			switch (hover_target)
			{
			case Panel::HoveredTarget::Top:
				on_click_top_area();
				break;
			case Panel::HoveredTarget::Left:
				on_click_left_area();
				break;
			case Panel::HoveredTarget::Right:
				on_click_right_area();
				break;
			}

			// マウスクリック後にパネルを非表示
			visible = false;
		}
		break;
		default:
			break;
		}
	}

	// パネルの状態を更新
	virtual void on_update(SDL_Renderer *renderer)
	{
		// リソースマネージャーからフォントリソースを取得
		static TTF_Font *font = ResourcesManager::instance()->get_font_pool().find(ResID::Font_Main)->second;

		// ホバーターゲットがない場合は更新不要
		if (hover_target == HoveredTarget::None)
			return;

		// ホバーターゲットを選択
		int val = 0;
		switch (hover_target)
		{
		case Panel::HoveredTarget::Top:
			val = val_top;
			break;
		case Panel::HoveredTarget::Left:
			val = val_left;
			break;
		case Panel::HoveredTarget::Right:
			val = val_right;
			break;
		}

		// 古いテクスチャリソースを破棄
		SDL_DestroyTexture(tex_text_background);
		tex_text_background = nullptr;
		SDL_DestroyTexture(tex_text_foreground);
		tex_text_foreground = nullptr;

		// ホバーターゲットの値に基づいてテキストテクスチャを生成
		std::string str_val = val < 0 ? "MAX" : std::to_string(val);
		SDL_Surface *suf_text_background = TTF_RenderText_Blended(font, str_val.c_str(), color_text_background);
		SDL_Surface *suf_text_foreground = TTF_RenderText_Blended(font, str_val.c_str(), color_text_foreground);

		// テクスチャを作成し、テキストの幅と高さを取得
		width_text = suf_text_background->w, height_text = suf_text_background->h;
		tex_text_background = SDL_CreateTextureFromSurface(renderer, suf_text_background);
		tex_text_foreground = SDL_CreateTextureFromSurface(renderer, suf_text_foreground);

		// サーフェスリソースを解放
		SDL_FreeSurface(suf_text_background);
		SDL_FreeSurface(suf_text_foreground);
	}

	// パネルをレンダリング
	virtual void on_render(SDL_Renderer *renderer)
	{
		// パネルが表示されていない場合はレンダリングしない
		if (!visible)
			return;

		// 選択カーソルをレンダリング
		SDL_Rect rect_dst_cursor =
			{
				center_pos.x - SIZE_TILE / 2,
				center_pos.y - SIZE_TILE / 2,
				SIZE_TILE, SIZE_TILE};
		SDL_RenderCopy(renderer, tex_select_cursor, nullptr, &rect_dst_cursor);

		// パネルの背景をレンダリング
		SDL_Rect rect_dst_panel =
			{
				center_pos.x - width / 2,
				center_pos.y - height / 2,
				width, height};
		SDL_Texture *tex_panel = nullptr;
		switch (hover_target)
		{
		case Panel::HoveredTarget::None:
			tex_panel = tex_idle;
			break;
		case Panel::HoveredTarget::Top:
			tex_panel = tex_hovered_top;
			break;
		case Panel::HoveredTarget::Left:
			tex_panel = tex_hovered_left;
			break;
		case Panel::HoveredTarget::Right:
			tex_panel = tex_hovered_right;
			break;
		}
		SDL_RenderCopy(renderer, tex_panel, nullptr, &rect_dst_panel);

		// 背景をレンダリング（ホバーターゲットが存在する場合）
		if (hover_target == HoveredTarget::None)
			return;
		SDL_Rect rect_dst_text;
		rect_dst_text.x = center_pos.x - width_text / 2 + offset_shadow.x;
		rect_dst_text.y = center_pos.y + height / 2 + offset_shadow.y;
		rect_dst_text.w = width_text, rect_dst_text.h = height_text;
		SDL_RenderCopy(renderer, tex_text_background, nullptr, &rect_dst_text);

		// 前景テキストをレンダリング
		rect_dst_text.x -= offset_shadow.x;
		rect_dst_text.y -= offset_shadow.y;
		SDL_RenderCopy(renderer, tex_text_foreground, nullptr, &rect_dst_text);
	}

protected:
	// ホバーターゲット領域の列挙型を定義
	enum class HoveredTarget
	{
		None,
		Top,
		Left,
		Right
	};

protected:
	// パネルの状態と位置に関連するメンバー変数
	bool visible = false;
	SDL_Point idx_tile_selected = {0};
	SDL_Point center_pos = {0};
	SDL_Texture *tex_idle = nullptr;
	SDL_Texture *tex_hovered_top = nullptr;
	SDL_Texture *tex_hovered_left = nullptr;
	SDL_Texture *tex_hovered_right = nullptr;
	SDL_Texture *tex_select_cursor = nullptr;

	// 各領域の値とホバーターゲット
	int val_top = 0, val_left = 0, val_right = 0;
	HoveredTarget hover_target = HoveredTarget::None;

protected:
	// 仮想関数、サブクラスで異なる領域のクリック動作を定義
	virtual void on_click_top_area() = 0;
	virtual void on_click_left_area() = 0;
	virtual void on_click_right_area() = 0;

private:
	// パネルの定数定義（ボタンサイズ、幅、高さ、オフセットなど）
	const int size_button = 48;
	const int width = 144, height = 144;
	const SDL_Point offset_top = {48, 6};
	const SDL_Point offset_left = {8, 80};
	const SDL_Point offset_right = {90, 80};
	const SDL_Point offset_shadow = {3, 3};
	const SDL_Color color_text_background = {175, 175, 175, 255};
	const SDL_Color color_text_foreground = {255, 255, 255, 255};

	// テキストの幅と高さ、およびテクスチャリソース
	int width_text = 0, height_text = 0;
	SDL_Texture *tex_text_background = nullptr;
	SDL_Texture *tex_text_foreground = nullptr;
};

#endif // !_PANEL_H_
