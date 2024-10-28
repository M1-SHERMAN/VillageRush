#ifndef _COIN_MANAGER_H_
#define _COIN_MANAGER_H_

/**
 * @brief コイン管理クラス
 *
 * このクラスは、ゲーム内のコインの管理を担当するシングルトンクラスです。
 *
 * 主な機能:
 * - コインの増加と減少の管理
 * - コインプロップ（視覚的表現）のリスト管理
 * - コインプロップの更新とレンダリング
 *
 * 使用方法:
 * - CoinManager::instance()->increase_coin() でコインを増やす
 * - CoinManager::instance()->decrease_coin() でコインを減らす
 * - on_update() メソッドを毎フレーム呼び出してコインプロップを更新
 * - on_render() メソッドを使用してコインプロップをレンダリング
 *
 */

#include "manager.h"
#include "coin_prop.h"
#include "config_manager.h"

#include <vector>
#include <SDL.h>
// コインマネージャークラス：コインの管理を行う
class CoinManager : public Manager<CoinManager>
{
	friend class Manager<CoinManager>;

public:
	// コインプロップのリスト型定義
	typedef std::vector<CoinProp *> CoinPropList;

public:
	// コインを増やす
	void increase_coin(double val)
	{
		num_coin += val;
	}

	// コインを減らす（0未満にはならない）
	void decrease_coin(double val)
	{
		num_coin -= val;
		if (num_coin <= 0)
			num_coin = 0;
	}

	// 更新処理
	void on_update(double delta)
	{
		// 全てのコインプロップを更新
		for (CoinProp *coin_prop : coin_prop_list)
		{
			coin_prop->on_update(delta);
		}

		// 削除可能なコインプロップを除去
		coin_prop_list.erase(std::remove_if(coin_prop_list.begin(), coin_prop_list.end(),
											[](CoinProp *coin_prop)
											{
												bool deletable = coin_prop->can_remove();
												if (deletable)
												{
													return deletable;
												}
											}),
							 coin_prop_list.end());
	}

	// レンダリング処理
	void on_render(SDL_Renderer *renderer)
	{
		// 全てのコインプロップを描画
		for (CoinProp *coin_prop : coin_prop_list)
		{
			coin_prop->on_render(renderer);
		}
	}

	// 現在のコイン数を取得
	double get_current_coin_num()
	{
		return num_coin;
	}

	// コインプロップリストの参照を取得
	CoinPropList &get_coin_prop_list()
	{
		return coin_prop_list;
	}

	// 新しいコインプロップを生成
	void spawn_coin_prop(const Vector2 &position)
	{
		CoinProp *coin_prop = new CoinProp();
		coin_prop->set_position(position);

		coin_prop_list.push_back(coin_prop);
	}

protected:
	// コンストラクタ：初期コイン数を設定
	CoinManager()
	{
		num_coin = ConfigManager::instance()->num_initial_coin;
	};

	// デストラクタ：全てのコインプロップを削除
	~CoinManager()
	{
		for (CoinProp *coin_prop : coin_prop_list)
		{
			delete coin_prop;
		}
	};

private:
	double num_coin = 0;		 // コイン数
	CoinPropList coin_prop_list; // コインプロップのリスト
};

#endif // !_COIN_MANAGER_H_
