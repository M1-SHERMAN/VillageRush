# VillageRush - タワーディフェンスゲーム

## 概要
このゲームは、タワーディフェンス型ゲームで、1人のプレイヤーが村を守るために様々な防御塔を配置し、
もう1人のプレイヤーがドラゴンを操作して敵の侵攻を阻止するというものです。C++とSDL2を使用して開発しており、
2Dグラフィックスと簡易的な物理シミュレーションを特徴としています。

## 詳細
### 現時点では実装済みの内容：
1. ゲーム内の防御塔の基本機能を提供する抽象基底クラスを実装しました。これには、防御塔の位置、向き、攻撃範囲の管理、敵の検出と攻撃、アニメーション制御、攻撃のクールダウン管理、レベルアップおよび能力値の更新が含まれます。
2. 防御塔としてアーチャー、斧使い、砲兵を実装し、それぞれに異なる攻撃間隔、ダメージ、視野範囲などのパラメータを設定しました。
3. ゲームの基本設定やプレイヤー、防御塔、敵のパラメータをJSONファイルから読み込む機能を実装しました。
4. 敵、防御塔、弾丸などを管理するマネージャークラスと、ゲームループ全体を制御するクラスを実装しました。

### 今後の実装予定：
1. シーン切り替え機能を追加し、ゲーム開始、終了、レベル選択などのシーンを作成したいと考えています。
2. 現在はプログラムを実行するとゲームがすぐに始まりますが、複数のレベルを追加し、プレイヤーがレベルを選択できるようにしたいと考えています。

## 設計の特徴

### シングルトンパターンの実装
`Manager`テンプレートクラスを使用して、シングルトンパターンを実装しています：


```4:24:VillageRush/manager.h
template <typename T>
class Manager
{
public:
	// 新たなマネージャーインスタンスを作成するために使用
	static T *instance()
	{
		if (!manager)
			manager = new T();
		return manager;
	}

private:
	static T *manager;

protected:
	Manager() = default;
	~Manager() = default;
	Manager(const Manager &) = delete;
	Manager &operator=(const Manager &) = delete;
};
```


このテンプレートクラスの特徴：
- スレッドセーフなシングルトンインスタンスの提供
- コピーコンストラクタとコピー代入演算子の削除による複製防止
- 静的メンバ変数による唯一のインスタンス管理

### 主要なマネージャークラス

#### 1. GameManager
ゲーム全体の制御を担当：
- ゲームループの管理
- SDL2の初期化と終了処理
- 各種マネージャーの更新制御
- イベント処理とレンダリング


```4:21:VillageRush/game_manager.h
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
```


#### 2. ResourcesManager
ゲームリソースの一元管理：
- テクスチャ、音声、フォントなどのリソース管理
- リソースの読み込みと解放
- ハッシュマップによる効率的なリソースアクセス


```111:120:VillageRush/resources_manager.h
class ResourcesManager : public Manager<ResourcesManager>
{
	friend class Manager<ResourcesManager>;

public:
	typedef std::unordered_map<ResID, TTF_Font *> FontPool;
	typedef std::unordered_map<ResID, Mix_Chunk *> SoundPool;
	typedef std::unordered_map<ResID, Mix_Music *> MusicPool;
	typedef std::unordered_map<ResID, SDL_Texture *> TexturePool;

```


#### 3. ConfigManager
設定データの集中管理：
- JSONファイルからの設定読み込み
- ゲーム内パラメータの保持
- マップデータの管理


```34:115:VillageRush/config_manager.h
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
		double speed = 3;// プレイヤーのスピード
		double normal_attack_interval = 0.5;// 通常攻撃の間隔
		double normal_attack_damage = 0;// 通常攻撃のダメージ
		double skill_interval = 10;// スキルの間隔
		double skill_damage = 1;// スキルのダメージ
	};

	// 防御塔テンプレート
	struct TowerTemplate
	{
		double interval[10] = {1};// 設置間隔
		double damage[10] = {25};// ダメージ
		double view_range[10] = {5};// 視野範囲
		double cost[10] = {50};// 建設コスト
		double upgrade_cost[9] = {75};// アップグレードコスト
	};

	// 敵テンプレート
	struct EnemyTemplate
	{
		double hp = 100;// 体力
		double speed = 1;// 敵のスピード
		double damage = 1;// 防御ポイントへのダメージ
		double reward_ratio = 0.5;// 報酬確率
		double recover_interval = 10;// 回復間隔
		double recover_range = 0;// 回復範囲
		double recover_intensity = 25;// 回復強度
	};

public:
	Map map;// マップ
	std::vector<Wave> wave_list;// ウェーブデータ

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

```

#### 4. その他のマネージャー
- **EnemyManager**: 敵ユニットの生成と管理
- **TowerManager**: 防衛タワーの配置と管理
- **BulletManager**: 弾丸の発射と衝突判定
- **WaveManager**: 敵の波状攻撃の制御
- **CoinManager**: ゲーム内通貨の管理

### シングルトンパターンを採用した理由
1. **グローバルアクセスの提供**：
   - 各マネージャーへの一貫したアクセスポイント
   - コンポーネント間の依存関係の簡素化

2. **リソース管理の一元化**：
   - メモリ使用の最適化
   - リソースの重複生成防止

3. **状態管理の集中化**：
   - ゲーム状態の一貫性保持
   - デバッグの容易性

## 技術スタック
- SDL2 (描画、入力、音声)
- SDL2_image (画像読み込み)
- SDL2_mixer (音声再生)
- SDL2_ttf (フォント描画)
- cJSON (設定ファイルの解析)
- Tiled(tilemapの作成)
  
## 素材
- [Miniworld](https://merchant-shade.itch.io/16x16-mini-world-sprites)
- [イワシロ音楽素材](https://iwashiro-sounds.work/)
