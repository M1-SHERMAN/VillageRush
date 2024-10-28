#ifndef _CONFIG_MANAGER_H_
#define _CONFIG_MANAGER_H_

/**
 * @brief ゲーム設定管理クラス
 *
 * このクラスは、ゲームの全体的な設定を管理するシングルトンクラスです。
 *
 * 主な機能:
 * - ゲームの基本設定（ウィンドウサイズ、タイトルなど）の管理
 * - プレイヤー、防御塔、敵のパラメータ設定
 * - マップ情報とウェーブデータの管理
 * - JSONファイルからの設定読み込み
 *
 * 使用方法:
 * - ConfigManager::instance()->load_config() で設定をロード
 * - ConfigManager::instance()->get_xxx() メソッドで各種設定にアクセス
 * - 必要に応じてset_xxx() メソッドで設定を変更
 *
 */

#include "map.h"
#include "manager.h"
#include "wave.h"

#include <SDL.h>
#include <string>
#include <cJSON.h>
#include <fstream>
#include <sstream>
#include <iostream>

/*ゲームのグローバルデータセンター*/
class ConfigManager : public Manager<ConfigManager>
{
	friend class Manager<ConfigManager>;

public:
	// 基本テンプレート
	struct BasicTemplate
	{
		std::string window_title = u8"ヴィレッジラッシュ";
		int window_width = 1280;
		int window_height = 720;
	};

	// プレイヤーテンプレート
	struct PlayerTemplate
	{
		double speed = 3;					 // プレイヤーのスピード
		double normal_attack_interval = 0.5; // 通常攻撃の間隔
		double normal_attack_damage = 0;	 // 通常攻撃のダメージ
		double skill_interval = 10;			 // スキルの間隔
		double skill_damage = 1;			 // スキルのダメージ
	};

	// 防御塔テンプレート
	struct TowerTemplate
	{
		double interval[10] = {1};	   // 設置間隔
		double damage[10] = {25};	   // ダメージ
		double view_range[10] = {5};   // 視野範囲
		double cost[10] = {50};		   // 建設コスト
		double upgrade_cost[9] = {75}; // アップグレードコスト
	};

	// 敵テンプレート
	struct EnemyTemplate
	{
		double hp = 100;			   // 体力
		double speed = 1;			   // 敵のスピード
		double damage = 1;			   // 防御ポイントへのダメージ
		double reward_ratio = 0.5;	   // 報酬確率
		double recover_interval = 10;  // 回復間隔
		double recover_range = 0;	   // 回復範囲
		double recover_intensity = 25; // 回復強度
	};

public:
	Map map;					 // マップ
	std::vector<Wave> wave_list; // ウェーブデータ

	// 各防御塔のレベル
	int level_archer = 0;
	int level_axeman = 0;
	int level_gunner = 0;

	// ゲーム状態の記録
	bool is_game_win = true;
	bool is_game_over = false;

	// タイルマップテクスチャ生成後の境界矩形
	SDL_Rect rect_tile_map = {0};

	// 基本 & プレイヤーテンプレート
	BasicTemplate basic_template;
	PlayerTemplate player_template;

	// 防御塔テンプレート
	TowerTemplate archer_template;
	TowerTemplate axeman_template;
	TowerTemplate gunner_template;

	// 敵テンプレート
	EnemyTemplate slim_template;
	EnemyTemplate king_slim_template;
	EnemyTemplate skeleton_template;
	EnemyTemplate goblin_template;
	EnemyTemplate goblin_priest_template;

	// ゲーム開始時のデータ
	const double num_initial_hp = 10;
	const double num_initial_coin = 100;
	const double num_coin_per_prop = 10;

public:
	/*レベル設定を読み込み、成功した場合はtrue、失敗した場合はfalseを返す*/
	bool load_level_config(const std::string &path)
	{
		/*解析前のチェック操作*/

		// ファイルを開き、正常に開けたかチェック
		std::ifstream file(path);
		if (!file.good())
		{
			return false;
		}

		// ファイルの内容を文字列ストリームに読み込む
		std::stringstream str_stream;
		str_stream << file.rdbuf(); // rdbuf()はファイルの内容を文字列ストリームに読み込むために使用。 << はデータの流れる方向を示す
		file.close();

		// cJSONライブラリを使用してファイルから読み取ったJSON形式の文字列を解析
		cJSON *json_root = cJSON_Parse(str_stream.str().c_str());

		// JSONの解析に失敗した場合、またはルート要素が配列でない場合は失敗を返す
		if (!json_root)
		{
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
									 u8"エラー",
									 u8"レベル設定の読み込み：JSONファイルの解析に失敗しました",
									 nullptr);
			cJSON_Delete(json_root);
			return false;
		}
		if (json_root->type != cJSON_Array)
		{
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
									 u8"エラー",
									 u8"レベル設定の読み込み：JSONデータタイプが配列ではありません",
									 nullptr);
			cJSON_Delete(json_root);
			return false;
		}

		/*データの解析開始*/

		// JSON配列の各要素を巡回し、各ウェーブの設定を処理
		cJSON *json_wave = nullptr;
		cJSON_ArrayForEach(json_wave, json_root)
		{
			// 現在の要素がJSONオブジェクトでない場合はスキップ
			if (json_wave->type != cJSON_Object)
				continue;

			// 新しいウェーブオブジェクトを作成し、ウェーブリストに追加
			wave_list.emplace_back();
			Wave &wave = wave_list.back();

			// 各ウェーブの報酬を解析
			cJSON *json_wave_rewards = cJSON_GetObjectItem(json_wave, "rewards");
			if (json_wave_rewards && json_wave_rewards->type == cJSON_Number)
				wave.rewards = json_wave_rewards->valuedouble;

			// 各ウェーブの間隔を解析
			cJSON *json_wave_interval = cJSON_GetObjectItem(json_wave, "interval");
			if (json_wave_interval && json_wave_interval->type == cJSON_Number)
				wave.interval = json_wave_interval->valuedouble;

			// 各ウェーブの生成イベントを解析
			cJSON *json_wave_spawn_list = cJSON_GetObjectItem(json_wave, "spawn_list");
			if (json_wave_spawn_list && json_wave_spawn_list->type == cJSON_Array)
			{
				cJSON *json_spawn_event = nullptr;
				cJSON_ArrayForEach(json_spawn_event, json_wave_spawn_list)
				{
					if (json_spawn_event->type != cJSON_Object)
						continue;

					// 新しい生成イベントを作成し、生成イベントリストに追加
					wave.spawn_event_list.emplace_back();
					Wave::SpawnEvent &spawn_event = wave.spawn_event_list.back();

					// 各イベントの生成間隔を解析
					cJSON *json_spawn_event_interval = cJSON_GetObjectItem(json_spawn_event, "interval");
					if (json_spawn_event_interval && json_spawn_event_interval->type == cJSON_Number)
						spawn_event.interval = json_spawn_event_interval->valuedouble;

					// 各イベントの生成ポイントを解析
					cJSON *json_spawn_event_spawn_point = cJSON_GetObjectItem(json_spawn_event, "point");
					if (json_spawn_event_spawn_point && json_spawn_event_spawn_point->type == cJSON_Number)
						spawn_event.spawn_point = json_spawn_event_spawn_point->valueint;

					// 各イベントの敵タイプを解析
					cJSON *json_spawn_event_enemy_type = cJSON_GetObjectItem(json_spawn_event, "enemy");
					if (json_spawn_event_enemy_type && json_spawn_event_enemy_type->type == cJSON_String)
					{
						// 対応する列挙値をマッチング
						const std::string str_enemy_type = json_spawn_event_enemy_type->valuestring;
						if (str_enemy_type == "Slim")
							spawn_event.enemy_type = EnemyType::Slim;
						else if (str_enemy_type == "KingSlim")
							spawn_event.enemy_type = EnemyType::KingSlim;
						else if (str_enemy_type == "Skeleton")
							spawn_event.enemy_type = EnemyType::Skeleton;
						else if (str_enemy_type == "Goblin")
							spawn_event.enemy_type = EnemyType::Goblin;
						else if (str_enemy_type == "GoblinPriest")
							spawn_event.enemy_type = EnemyType::GoblinPriest;
					}
				}
				// このウェーブの生成イベントリストが空の場合、このウェーブを削除
				if (wave.spawn_event_list.empty())
					wave_list.pop_back();
			}
		}
		// 完了後、JSON解析で使用したメモリを解放
		cJSON_Delete(json_root);

		// ウェーブリストが空の場合、読み込みに失敗したとみなしfalseを返す
		if (wave_list.empty())
		{
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
									 u8"エラー",
									 u8"レベル設定の読み込み：ウェーブリストが空です",
									 nullptr);
			return false;
		}

		// すべてのウェーブ設定の読み込みに成功したため、trueを返す
		return true;
	}

	bool load_game_config(const std::string &path)
	{
		// ファイルを開き、正常に開けたかチェック
		std::ifstream file(path);
		// ファイルが開けなかった場合、falseを返す
		if (!file.good())
		{
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
									 u8"エラー",
									 u8"ゲーム設定の読み込み：設定ファイルを開けません",
									 nullptr);
			return false;
		}

		// ファイルの内容を文字列ストリームに読み込み、ファイル全体の内容を読み取る
		std::stringstream str_stream;
		str_stream << file.rdbuf();
		file.close(); // ファイルの読み取りが完了したらファイルを閉じる

		// cJSONライブラリを使用して読み取ったJSON形式の文字列をcJSONオブジェクトに解析
		cJSON *json_root = cJSON_Parse(str_stream.str().c_str());

		// JSONの解析に失敗した場合、またはルート要素がJSONオブジェクトでない場合はfalseを返す
		if (!json_root || json_root->type != cJSON_Object)
		{
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
									 u8"エラー",
									 u8"ゲーム設定の読み込み：JSONの解析に失敗したか、ルート要素がJSONオブジェクトではありません",
									 nullptr);
			return false;
		}

		// 解析されたJSONオブジェクトから特定の子オブジェクトを取得：基本データ、プレイヤーデータ、防御塔データ、敵データ
		cJSON *json_basic = cJSON_GetObjectItem(json_root, "basic");
		cJSON *json_player = cJSON_GetObjectItem(json_root, "player");
		cJSON *json_tower = cJSON_GetObjectItem(json_root, "tower");
		cJSON *json_enemy = cJSON_GetObjectItem(json_root, "enemy");

		// これらの子オブジェクトが正常に取得され、タイプが正しいかチェック。エラーがある場合、JSONオブジェクトを削除しfalseを返す
		if (!json_basic || !json_player || !json_tower || !json_enemy || json_basic->type != cJSON_Object || json_player->type != cJSON_Object || json_tower->type != cJSON_Object || json_enemy->type != cJSON_Object)
		{
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
									 u8"エラー",
									 u8"ゲーム設定の読み込み：子オブジェクトのチェック中にエラーが発生しました",
									 nullptr);
			cJSON_Delete(json_root);
			return false;
		}

		// 各テンプレートデータを解析し、JSONデータを対応するテンプレート解析関数に渡す
		parse_basic_template(basic_template, json_basic);
		parse_player_template(player_template, json_player);

		parse_tower_template(archer_template, cJSON_GetObjectItem(json_tower, "archer"));
		parse_tower_template(axeman_template, cJSON_GetObjectItem(json_tower, "axeman"));
		parse_tower_template(gunner_template, cJSON_GetObjectItem(json_tower, "gunner"));

		parse_enemy_template(slim_template, cJSON_GetObjectItem(json_enemy, "slim"));
		parse_enemy_template(king_slim_template, cJSON_GetObjectItem(json_enemy, "king_slim"));
		parse_enemy_template(skeleton_template, cJSON_GetObjectItem(json_enemy, "skeleton"));
		parse_enemy_template(goblin_template, cJSON_GetObjectItem(json_enemy, "goblin"));
		parse_enemy_template(goblin_priest_template, cJSON_GetObjectItem(json_enemy, "goblin_priest"));

		// 解析完了後、JSONファイルを閉じ、trueを返す
		cJSON_Delete(json_root);
		return true;
	}

protected:
	ConfigManager() = default;
	~ConfigManager() = default;

private:
	void parse_basic_template(BasicTemplate &tpl, cJSON *json_root)
	{
		// json_rootがnullでなく、JSONオブジェクトであることを確認。条件を満たさない場合は関数を終了。
		if (!json_root || json_root->type != cJSON_Object)
			return;

		// JSONオブジェクトから対応するフィールドの値を取得。
		cJSON *json_window_title = cJSON_GetObjectItem(json_root, "window_title");
		cJSON *json_window_width = cJSON_GetObjectItem(json_root, "window_width");
		cJSON *json_window_height = cJSON_GetObjectItem(json_root, "window_height");

		// 正常に取得でき、かつ型が正しい場合、tpl構造体の対応するメンバ変数に値を代入
		if (json_window_title && json_window_title->type == cJSON_String)
			tpl.window_title = json_window_title->valuestring;
		if (json_window_width && json_window_width->type == cJSON_Number)
			tpl.window_width = json_window_width->valueint;
		if (json_window_height && json_window_height->type == cJSON_Number)
			tpl.window_height = json_window_height->valueint;
	}

	void parse_player_template(PlayerTemplate &tpl, cJSON *json_root)
	{
		// json_rootがnullでなく、JSONオブジェクトであることを確認。条件を満たさない場合は関数を終了。
		if (!json_root || json_root->type != cJSON_Object)
			return;

		// JSONオブジェクトから対応するフィールドの値を取得。
		cJSON *json_speed = cJSON_GetObjectItem(json_root, "speed");
		cJSON *json_normal_attack_interval = cJSON_GetObjectItem(json_root, "normal_attack_interval");
		cJSON *json_normal_attack_damage = cJSON_GetObjectItem(json_root, "normal_attack_damage");
		cJSON *json_skill_interval = cJSON_GetObjectItem(json_root, "skill_interval");
		cJSON *json_skill_damage = cJSON_GetObjectItem(json_root, "skill_damage");

		// 正常に取得でき、かつ類型が正しい場合、tpl構造体の対応するメンバ変数に値を代入
		if (json_speed && json_speed->type == cJSON_Number)
			tpl.speed = json_speed->valuedouble;

		if (json_normal_attack_interval && json_normal_attack_interval->type == cJSON_Number)
			tpl.normal_attack_interval = json_normal_attack_interval->valuedouble;
		if (json_normal_attack_damage && json_normal_attack_damage->type == cJSON_Number)
			tpl.normal_attack_damage = json_normal_attack_damage->valuedouble;

		if (json_skill_interval && json_skill_interval->type == cJSON_Number)
			tpl.skill_interval = json_skill_interval->valuedouble;
		if (json_skill_damage && json_skill_damage->type == cJSON_Number)
			tpl.skill_damage = json_skill_damage->valuedouble;
	}

	void parse_number_array(double *arr, int max_len, cJSON *json_root)
	{
		// json_rootがnullでなく、JSON配列であることを確認。条件を満たさない場合は関数を終了。
		if (!json_root || json_root->type != cJSON_Array)
			return;

		// インデックスを-1で初期化
		int idx = -1;
		cJSON *json_element = nullptr;

		// json_rootに対応するJSON配列を巡回し、各要素をjson_elementに代入。
		cJSON_ArrayForEach(json_element, json_root)
		{
			idx++; // インデックスをインクリメント

			// 現在の要素が数値型でない、または配列の最大長を超えた場合はスキップ。
			if (json_element->type != cJSON_Number || idx >= max_len)
				continue;

			// JSON数値を配列arrに格納。
			arr[idx] = json_element->valuedouble;
		}
	}

	void parse_tower_template(TowerTemplate &tpl, cJSON *json_root)
	{
		// json_rootがnullでなく、JSONオブジェクトであることを確認。条件を満たさない場合は関数を終了。
		if (!json_root || json_root->type != cJSON_Object)
			return;

		cJSON *json_interval = cJSON_GetObjectItem(json_root, "interval");
		cJSON *json_damage = cJSON_GetObjectItem(json_root, "damage");
		cJSON *json_view_range = cJSON_GetObjectItem(json_root, "view_range");
		cJSON *json_cost = cJSON_GetObjectItem(json_root, "cost");
		cJSON *json_upgrade_cost = cJSON_GetObjectItem(json_root, "upgrade_cost");

		parse_number_array(tpl.interval, 10, json_interval);
		parse_number_array(tpl.damage, 10, json_damage);
		parse_number_array(tpl.view_range, 10, json_view_range);
		parse_number_array(tpl.cost, 10, json_cost);
		parse_number_array(tpl.upgrade_cost, 9, json_upgrade_cost);
	}

	void parse_enemy_template(EnemyTemplate &tpl, cJSON *json_root)
	{
		// json_rootがnullでなく、JSONオブジェクトであることを確認。条件を満たさない場合は関数を終了。
		if (!json_root || json_root->type != cJSON_Object)
			return;

		cJSON *json_hp = cJSON_GetObjectItem(json_root, "hp");
		cJSON *json_speed = cJSON_GetObjectItem(json_root, "speed");
		cJSON *json_damage = cJSON_GetObjectItem(json_root, "damage");
		cJSON *json_reward_ratio = cJSON_GetObjectItem(json_root, "reward_ratio");
		cJSON *json_recover_interval = cJSON_GetObjectItem(json_root, "recover_interval");
		cJSON *json_recover_range = cJSON_GetObjectItem(json_root, "recover_range");
		cJSON *json_recover_intensity = cJSON_GetObjectItem(json_root, "recover_intensity");

		if (json_hp && json_hp->type == cJSON_Number)
			tpl.hp = json_hp->valuedouble;
		if (json_speed && json_speed->type == cJSON_Number)
			tpl.speed = json_speed->valuedouble;
		if (json_damage && json_damage->type == cJSON_Number)
			tpl.damage = json_damage->valuedouble;
		if (json_reward_ratio && json_reward_ratio->type == cJSON_Number)
			tpl.reward_ratio = json_reward_ratio->valuedouble;
		if (json_recover_interval && json_recover_interval->type == cJSON_Number)
			tpl.recover_interval = json_recover_interval->valuedouble;
		if (json_recover_range && json_recover_range->type == cJSON_Number)
			tpl.recover_range = json_recover_range->valuedouble;
		if (json_recover_intensity && json_recover_intensity->type == cJSON_Number)
			tpl.recover_intensity = json_recover_intensity->valuedouble;
	}
};
#endif // !_CONFIG_MANAGER_H_
