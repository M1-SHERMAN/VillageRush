#ifndef _BULLET_MANAGER_H_
#define _BULLET_MANAGER_H_

/**
 * @brief 弾丸管理クラス
 *
 * このクラスは、ゲーム内の弾丸の生成、管理、更新を担当するシングルトンクラスです。
 *
 * 主な機能:
 * - 異なる種類の弾丸（矢、斧、砲弾）の生成と管理
 * - 弾丸のリストの維持と更新
 * - 弾丸の移動、衝突検出、削除の処理
 * - 弾丸のレンダリング
 *
 * 使用方法:
 * - BulletManager::instance()->create_bullet() を使用して新しい弾丸を生成
 * - on_update() メソッドを毎フレーム呼び出して弾丸の状態を更新
 * - on_render() メソッドを使用して弾丸をレンダリング
 *
 */

#include "manager.h"
#include "bullet.h"
#include "bullet_type.h"
#include "arrow_bullet.h"
#include "axe_bullet.h"
#include "shell_bullet.h"

#include <vector>

class BulletManager : public Manager<BulletManager>
{
	friend class Manager<BulletManager>;

public:
	// 弾のリストの型定義
	typedef std::vector<Bullet *> BulletList;

public:
	/* データの更新 */
	void on_update(double delta)
	{
		// すべての弾をループし、on_updateメソッドを呼び出す
		for (Bullet *bullet : bullet_list)
		{
			bullet->on_update(delta);
		}

		// 削除可能な弾を取り除く
		bullet_list.erase(std::remove_if(
							  bullet_list.begin(), bullet_list.end(),
							  [](const Bullet *bullet)
							  {
								  // 弾が削除可能かチェック
								  bool deletable = bullet->can_remove();
								  if (deletable)
								  {
									  delete bullet; // （可能な場合）弾を削除してメモリを解放
								  }
								  return deletable; // 削除可能かどうかのブール値を返す
							  }),
						  bullet_list.end());
	}

	/* レンダリングの更新 */
	void on_render(SDL_Renderer *renderer)
	{
		// すべての弾をループし、on_renderメソッドを呼び出してレンダリング
		for (Bullet *bullet : bullet_list)
		{
			bullet->on_render(renderer);
		}
	}

	BulletList &get_bullet_list()
	{
		return bullet_list;
	}

	/* 弾のスポーン */
	void spawn_bullet(BulletType type, const Vector2 &position, const Vector2 &velocity, double damage)
	{
		// 新しくスポーンされた弾のポインタを保存
		Bullet *bullet = nullptr;

		// タイプに応じて異なる弾をスポーン
		switch (type)
		{
		case Arrow:
			bullet = new ArrowBullet();
			break;
		case Axe:
			bullet = new AxeBullet();
			break;
		case Shell:
			bullet = new ShellBullet();
			break;
		default:
			bullet = new ArrowBullet();
			break;
		}

		bullet->set_position(position);						// 弾の初期位置を設定
		bullet->set_velocity_and_rotation(velocity);	// 弾のスピードを設定
		bullet->set_damage(damage);						// 弾のダメージを設定

		bullet_list.push_back(bullet); // 弾をリストに追加
	};

private:
	BulletManager() = default;

	/* デストラクタ、弾のインスタンスを削除するために使用 */
	~BulletManager()
	{
		for (Bullet *bullet : bullet_list)
		{
			delete bullet;
		}
	};

private:
	// 生成されたすべての弾のインスタンスを格納
	BulletList bullet_list;
};

#endif // !_BULLET_MANAGER_H_
