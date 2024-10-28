#ifndef _UPGRADE_PANEL_H_
#define _UPGRADE_PANEL_H_

#include "panel.h"
#include "coin_manager.h"
#include "tower_manager.h"
#include "resources_manager.h"

class UpgradePanel : public Panel
{
public:
	UpgradePanel()
	{
		// リソースマネージャーのテクスチャプールを取得
		const ResourcesManager::TexturePool &texture_pool = ResourcesManager::instance()->get_texture_pool();

		// テクスチャプールから異なる状態のテクスチャリソースを取得
		tex_idle = texture_pool.find(ResID::Tex_UIUpgradeIdle)->second;
		tex_hovered_top = texture_pool.find(ResID::Tex_UIUpgradeHoveredTop)->second;
		tex_hovered_left = texture_pool.find(ResID::Tex_UIUpgradeHoveredLeft)->second;
		tex_hovered_right = texture_pool.find(ResID::Tex_UIUpgradeHoveredRight)->second;
	};
	~UpgradePanel() = default;

	// パネルの状態を更新、基底クラスのon_updateメソッドをオーバーライド
	void on_update(SDL_Renderer *renderer)
	{
		// TowerManagerのシングルトンを取得、防御塔のアップグレードコストを取得するために使用
		static TowerManager *instance = TowerManager::instance();

		// 3つの異なるタワータイプのアップグレードコストを更新
		val_top = (int)instance->get_upgrade_cost(TowerType::Axeman);
		val_left = (int)instance->get_upgrade_cost(TowerType::Archer);
		val_right = (int)instance->get_upgrade_cost(TowerType::Gunner);

		Panel::on_update(renderer);
	}

protected:
	/* 異なるパネルがクリックされたときの対応する関数 */
	void on_click_top_area() override
	{
		CoinManager *instance = CoinManager::instance();
		if (val_top > 0 && val_top <= instance->get_current_coin_num())
		{
			TowerManager::instance()->upgrade_tower(TowerType::Axeman);
			instance->decrease_coin(val_top);
		}
	}

	void on_click_left_area() override
	{
		CoinManager *instance = CoinManager::instance();
		if (val_left > 0 && val_left <= instance->get_current_coin_num())
		{
			TowerManager::instance()->upgrade_tower(TowerType::Archer);
			instance->decrease_coin(val_left);
		}
	}

	void on_click_right_area() override
	{
		CoinManager *instance = CoinManager::instance();
		if (val_right > 0 && val_right <= instance->get_current_coin_num())
		{
			TowerManager::instance()->upgrade_tower(TowerType::Gunner);
			instance->decrease_coin(val_right);
		}
	}
};

#endif // !_UPGRADE_PANEL_H_
