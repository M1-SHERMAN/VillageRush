#ifndef _TIMER_H_
#define _TIMER_H_

#include <functional>

class Timer
{
public:
	Timer() = default;
	~Timer() = default;

	// タイマーを再起動する
	void restart()
	{
		pass_time = 0;	 // 経過時間をリセット
		shotted = false; // トリガー状態をリセットし、再トリガーを許可
	}

	// 待機時間を設定する
	void set_wait_time(double val)
	{
		wait_time = val;
	}

	// 一回だけトリガーするかどうかを設定する
	void set_one_shot(bool flag)
	{
		one_shot = flag;
	}

	// タイムアウト時に呼び出される関数を設定する
	void set_on_timeout(std::function<void()> on_timeout)
	{
		this->on_timeout = on_timeout; // タイムアウト処理関数を保存
	}

	// タイマーを一時停止する
	void pause()
	{
		paused = true;
	}

	// タイマーを再開する
	void resume()
	{
		paused = false;
	}

	// タイマー情報を更新する。毎回時間増分を渡し、累積時間を更新する
	void on_update(double delta)
	{
		if (paused)
			return; // 一時停止状態の場合、何も実行しない

		pass_time += delta; // 経過時間を累積する

		// 累積時間が設定された待機時間を超えた場合、タイマーをトリガーする必要があるかチェックする
		if (pass_time >= wait_time)
		{
			// タイマーが複数回トリガーする場合、または一回トリガーだがまだトリガーされていない場合、トリガー可能とマークする
			bool can_shot = (!one_shot || (one_shot && !shotted));
			shotted = true;

			// トリガー条件を満たし、タイムアウト処理関数が設定されている場合、タイムアウト操作を実行する
			if (can_shot && on_timeout)
				on_timeout();

			// 消費された待機時間を差し引き、次の可能なトリガーに備える
			pass_time -= wait_time;
		}
	}

private:
	double pass_time = 0;  // 経過時間を記録
	double wait_time = 0;  // 設定された待機時間
	bool paused = false;   // タイマーが一時停止状態かどうかを記録
	bool shotted = false;  // タイマーが既にトリガーされたかどうかを記録
	bool one_shot = false; // 一回だけトリガーを許可するかどうか

	std::function<void()> on_timeout; // タイムアウト処理関数を格納。タイマーが設定時間に達したときに呼び出される
};

#endif // !_TIMER_H_
