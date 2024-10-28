#ifndef _WAVE_MANAGER_H_
#define _WAVE_MANAGER_H_

/**
 * @brief 波管理クラス
 *
 * このクラスは、ゲーム内の敵の波の生成と管理を制御するシングルトンクラスです。
 *
 * 主な機能:
 * - 波の開始と終了の管理
 * - 敵の生成タイミングの制御
 * - 波ごとの報酬の管理
 * - ゲームの進行状況の追跡（勝利条件、敗北条件の判定）
 *
 * 使用方法:
 * - WaveManager::instance()->on_update(delta) を毎フレーム呼び出して波の状態を更新
 * - is_wave_started フラグを使用して現在の波の状態を確認
 * - idx_wave を使用して現在の波のインデックスを取得
 *
 * 注意事項:
 * - このクラスはシングルトンパターンを使用しているため、直接インスタンス化せず、
 *   instance() メソッドを通じてアクセスすること
 * - ConfigManager, EnemyManager, CoinManager との連携が重要なため、
 *   これらのクラスの状態変更に注意すること
 * - ゲームの終了条件（勝利/敗北）の判定ロジックが含まれているため、
 *   適切なタイミングでゲーム状態を更新することが重要
 */

#include "timer.h"
#include "manager.h"
#include "config_manager.h"
#include "enemy_manager.h"
#include "coin_manager.h"

/* 波管理クラス、敵の波の生成と管理を制御するためのクラス */
class WaveManager : public Manager<WaveManager>
{
	friend class Manager<WaveManager>;

public:
	/* 各フレームの更新、波の進行と敵の生成を処理する */
	void on_update(double delta)
	{
		// 設定マネージャーのインスタンスを取得
		static ConfigManager *instance = ConfigManager::instance();

		// ゲームが終了している場合、即座に戻る
		if (instance->is_game_over)
		{
			return;
		}

		// 波がまだ開始されていない場合は波開始タイマーを更新、そうでなければ敵生成タイマーを更新
		if (!is_wave_started)
			timer_start_wave.on_update(delta);
		else
			timer_spawn_enemy.on_update(delta);

		// 最後の敵が生成され、すべての敵が倒されている場合、次の波の準備をするかゲームを終了する
		if (is_spawned_last_enemy && EnemyManager::instance()->check_cleared())
		{
			// プレイヤーに報酬を与える
			CoinManager::instance()->increase_coin(instance->wave_list[idx_wave].rewards);

			// 次の波の準備
			idx_wave++;
			if (idx_wave >= instance->wave_list.size()) // 波が総波数を超えた場合、ゲームに勝利して終了
			{
				instance->is_game_win = true;
				instance->is_game_over = true;
			}
			else
			{
				// 状態をリセットし、次の波の準備をする
				idx_spawn_event = 0;
				is_wave_started = false;
				is_spawned_last_enemy = false;
				const Wave &wave = instance->wave_list[idx_wave];
				timer_start_wave.set_wait_time(wave.interval); // 次の波の開始時間を設定
				timer_start_wave.restart();					   // タイマーを再起動
			}
		}
	}

protected:
	WaveManager()
	{
		// 設定マネージャーから波リストを取得
		static const std::vector<Wave> &wave_list = ConfigManager::instance()->wave_list;

		// 波開始タイマーを設定、一度だけトリガー
		timer_start_wave.set_one_shot(true);
		timer_start_wave.set_wait_time(wave_list[0].interval); // 最初の波の開始間隔を設定
		timer_start_wave.set_on_timeout(
			[&]()
			{
				// タイマーが終了したら、波を開始し敵生成タイマーを起動
				is_wave_started = true;
				timer_spawn_enemy.set_wait_time(wave_list[idx_wave].spawn_event_list[0].interval);
				timer_spawn_enemy.restart();
			});

		// 敵生成タイマーを設定、敵を一体ずつ生成
		timer_spawn_enemy.set_one_shot(true);
		timer_spawn_enemy.set_on_timeout(
			[&]()
			{
				// 現在の波の敵生成イベントリストを取得
				const std::vector<Wave::SpawnEvent> &spawn_event_list = wave_list[idx_wave].spawn_event_list;
				const Wave::SpawnEvent &spawn_event = spawn_event_list[idx_spawn_event];

				// 敵を一体生成
				EnemyManager::instance()->spawn_enemy(spawn_event.enemy_type, spawn_event.spawn_point);

				// 生成イベントのインデックスを更新し、すべての敵が生成されたかチェック
				idx_spawn_event++;
				if (idx_spawn_event >= spawn_event_list.size())
				{
					is_spawned_last_enemy = true; // 最後の敵が生成されたことをマーク
					return;
				}
				timer_spawn_enemy.set_wait_time(spawn_event_list[idx_spawn_event].interval);
				timer_spawn_enemy.restart();
			});
	}
	~WaveManager() = default;

private:
	int idx_wave = 0;		 // 現在の波のインデックス
	int idx_spawn_event = 0; // 現在の波における敵生成イベントのインデックス

	Timer timer_start_wave;				// 波開始タイマー
	Timer timer_spawn_enemy;			// 敵生成タイマー
	bool is_wave_started = false;		// 現在の波が開始されたかどうかのフラグ
	bool is_spawned_last_enemy = false; // 最後の敵が生成されたかどうかのフラグ
};

#endif // !_WAVE_MANAGER_H_
