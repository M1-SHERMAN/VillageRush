#pragma once
#ifndef _PLACE_PANEL_H_
#define _PLACE_PANEL_H_

#include "panel.h"
#include "coin_manager.h"
#include "tower_manager.h"
#include "resources_manager.h"

#include <SDL2_gfxPrimitives.h>

class PlacePanel : public Panel
{
public:
	PlacePanel()
	{
		// リソースマネージャーのテクスチャプールを取得
		const ResourcesManager::TexturePool &texture_pool = ResourcesManager::instance()->get_texture_pool();

		// テクスチャプールから異なる状態のテクスチャリソースを取得
		tex_idle = texture_pool.find(ResID::Tex_UIPlaceIdle)->second;
		tex_hovered_top = texture_pool.find(ResID::Tex_UIPlaceHoveredTop)->second;
		tex_hovered_left = texture_pool.find(ResID::Tex_UIPlaceHoveredLeft)->second;
		tex_hovered_right = texture_pool.find(ResID::Tex_UIPlaceHoveredRight)->second;
	};
	~PlacePanel() = default;

	// パネルの状態を更新、基底クラスのon_updateメソッドをオーバーライド
	void on_update(SDL_Renderer *renderer) override
	{
		// TowerManagerのシングルトンインスタンスを取得、防御塔の建設コストを取得するために使用
		static TowerManager *instance = TowerManager::instance();

		// 各種防御塔ユニットの建設コストを取得
		val_top = (int)instance->get_place_cost(TowerType::Axeman);
		val_left = (int)instance->get_place_cost(TowerType::Archer);
		val_right = (int)instance->get_place_cost(TowerType::Gunner);

		// 各種防御塔ユニットの視野範囲を取得し、実際のサイズに変換
		reg_top = (int)instance->get_view_range(TowerType::Axeman) * SIZE_TILE;
		reg_left = (int)instance->get_view_range(TowerType::Archer) * SIZE_TILE;
		reg_right = (int)instance->get_view_range(TowerType::Gunner) * SIZE_TILE;

		// 基底クラスPanelのon_updateメソッドを呼び出し、他の更新ロジックを続行
		Panel::on_update(renderer);
	}

	// パネルをレンダリング、基底クラスのon_renderメソッドをオーバーライド
	void on_render(SDL_Renderer *renderer) override
	{
		// パネルが表示されていない場合、直ちに戻る
		if (!visible)
			return;

		// マウスのホバー位置に基づいて表示する範囲を決定
		int reg = 0;
		switch (hover_target)
		{
		case Panel::HoveredTarget::Top:
			reg = reg_top;
			break;
		case Panel::HoveredTarget::Left:
			reg = reg_left;
			break;
		case Panel::HoveredTarget::Right:
			reg = reg_right;
			break;
		}

		// 範囲が0より大きい場合、その範囲の円形領域を描画
		if (reg > 0)
		{
			// 円形領域を塗りつぶし、タワー防衛ユニットの攻撃範囲を表示
			filledCircleRGBA(renderer, center_pos.x, center_pos.y, reg,
							 color_region_content.r, color_region_content.g, color_region_content.b, color_region_content.a);
			// 円形の境界線を描画
			aacircleRGBA(renderer, center_pos.x, center_pos.y, reg,
						 color_region_edge.r, color_region_edge.g, color_region_edge.b, color_region_edge.a);
		}

		// 基底クラスPanelのon_renderメソッドを呼び出し、他のレンダリングロジックを続行
		Panel::on_render(renderer);
	}

protected:
	// 上部領域がクリックされたときに実行される操作
	void on_click_top_area() override
	{
		// CoinManagerのシングルトンインスタンスを取得、コイン操作を処理するために使用
		CoinManager *instance = CoinManager::instance();
		// コインの数が十分であれば、斧兵タワーを配置し、対応するコインを減少させる
		if (val_top <= instance->get_current_coin_num())
		{
			TowerManager::instance()->place_tower(TowerType::Axeman, idx_tile_selected);
			instance->decrease_coin(val_top);
		}
	}

	void on_click_left_area() override
	{
		CoinManager *instance = CoinManager::instance();
		if (val_left <= instance->get_current_coin_num())
		{
			TowerManager::instance()->place_tower(TowerType::Archer, idx_tile_selected);
			instance->decrease_coin(val_left);
		}
	}

	void on_click_right_area() override
	{
		CoinManager *instance = CoinManager::instance();
		if (val_right <= instance->get_current_coin_num())
		{
			TowerManager::instance()->place_tower(TowerType::Gunner, idx_tile_selected);
			instance->decrease_coin(val_right);
		}
	}

private:
	// 領域の境界線と内容の色を定義
	const SDL_Color color_region_edge = {30, 80, 162, 175};	  // 境界線の色（青）
	const SDL_Color color_region_content = {0, 149, 217, 75}; // 内容の色（緑）

private:
	// 上部、左側、右側の領域の視野範囲を定義
	int reg_top = 0, reg_left = 0, reg_right = 0;
};

#endif // !_PLACE_PANEL_H_
