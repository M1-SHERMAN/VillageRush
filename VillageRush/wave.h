#ifndef _WAVE_H_
#define _WAVE_H_

#include "enemy_type.h"

#include <vector>

struct Wave
{
	// 敵生成イベントの構造体
	struct SpawnEvent
	{
		double interval = 0;
		int spawn_point = 1;
		EnemyType enemy_type = EnemyType::Slim;
	};

	// その他のデータ
	double rewards = 0;	 // ウェーブ報酬
	double interval = 0; // ウェーブ生成間隔
	std::vector<SpawnEvent> spawn_event_list;
};

#endif // !_WAVE_H_
