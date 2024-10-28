#ifndef _ANIMATION_H_
#define _ANIMATION_H_

/**
 * @brief アニメーションクラス
 *
 * このクラスは、ゲーム内のスプライトアニメーションを管理します。
 * フレームの更新、レンダリング、ループ制御、コールバック機能などを提供します。
 *
 * 主な機能:
 * - フレームごとのアニメーション更新
 * - アニメーションのレンダリング
 * - ループ設定
 * - アニメーション終了時のコールバック
 * - タイマーを使用したフレーム制御
 *
 * このクラスは、ゲーム内の様々なオブジェクト（キャラクター、エフェクトなど）の
 * アニメーション制御に使用されます。
 */

#include "timer.h"

#include <vector>
#include <functional>
#include <SDL.h>

/*アニメーションのフレーム更新とレンダリングを処理する*/
class Animation
{
public:
	// コールバックタイプの定義、アニメーション再生終了時にコールバックをトリガーできる
	typedef std::function<void()> PlayCallback;

public:
	Animation()
	{
		// タイマーをワンショットではなく設定し、継続的に更新されることを示す
		timer.set_one_shot(false);

		// タイマーのタイムアウト後のコールバック関数を設定し、アニメーションフレームの更新を担当する
		timer.set_on_timeout(
			[&]()
			{
				// 現在のフレームインデックスをインクリメント
				idx_frame++;

				// 現在のフレームがフレームリストの長さを超えた場合、ループが必要かどうかをチェック
				if (idx_frame >= rect_src_list.size())
				{
					// ループする場合は最初のフレームに戻り、そうでない場合は最後のフレームを維持
					idx_frame = is_loop ? 0 : rect_src_list.size() - 1;

					// ループしない場合で終了コールバックが設定されている場合、コールバックをトリガー
					if (!is_loop && on_finished)
						on_finished();
				};
			});
	}

	~Animation() = default;

	// アニメーションをリセットし、現在のフレームを0に設定してタイマーを再起動
	void reset()
	{
		timer.restart();

		idx_frame = 0;
	}

	// フレームデータを設定し、テクスチャ内の各フレームのソース矩形を計算
	// num_h; num_vはそれぞれフレームアニメーションセット画像の幅と高さを表す
	// idx_listは各フレームのインデックスを表す
	void set_frame_data(SDL_Texture *texture, int num_h, int num_v, const std::vector<int> &idx_list)
	{

		int width_tex, height_tex;

		// アニメーションセットの実際の幅と高さを問い合わせる
		this->texture = texture;
		SDL_QueryTexture(texture, nullptr, nullptr, &width_tex, &height_tex);

		// 水平および垂直フレーム数に基づいて各フレームの幅と高さを計算
		width_frame = width_tex / num_h, height_frame = height_tex / num_v;

		// ソース矩形リストのサイズを調整し、各フレームの矩形を計算
		rect_src_list.resize(idx_list.size()); // idx_list.sizeは各フレームのアニメーションを表す
		for (size_t i = 0; i < idx_list.size(); i++)
		{
			int idx = idx_list[i];				   // idxは現在のフレームインデックスを表す
			SDL_Rect &rect_src = rect_src_list[i]; // 現在のフレームの画像を切り取る

			// アニメーションセット内の各フレームのxとy位置を計算
			rect_src.x = (idx % num_h) * width_frame;
			rect_src.y = (idx / num_h) * height_frame;
			rect_src.w = width_frame, rect_src.h = height_frame;
		}
	}

	// アニメーションをループ再生するかどうかを設定
	void set_loop(bool is_loop)
	{
		this->is_loop = is_loop;
	}

	// フレーム更新の時間間隔を設定
	void set_interval(double interval)
	{
		timer.set_wait_time(interval);
	}

	// アニメーション終了時のコールバック関数を設定
	void set_on_finished(PlayCallback on_finished)
	{
		this->on_finished = on_finished;
	}

	// アップデート関数、時間増分を渡してタイマーを更新
	void on_update(double delta)
	{
		timer.on_update(delta);
	}

	// レンダリング関数、現在のフレームを画面の指定位置にレンダリング、回転角度をサポート
	void on_render(SDL_Renderer *renderer, const SDL_Point &pos_dst, double angle = 0) const
	{
		static SDL_Rect rect_dst;
		rect_dst.x = pos_dst.x, rect_dst.y = pos_dst.y;
		rect_dst.w = width_frame, rect_dst.h = height_frame;

		// 現在のフレームのテクスチャを目標矩形位置にレンダリング
		SDL_RenderCopyEx(renderer, texture, &rect_src_list[idx_frame], &rect_dst, angle, nullptr, SDL_RendererFlip::SDL_FLIP_NONE);
		// rect_src_list: クリッピング矩形
		// rect_dst: 目標矩形
		// SDL_RendererFlip: 画像を水平または垂直方向に反転するかどうか
	}

private:
	Timer timer;
	bool is_loop = true;
	size_t idx_frame = 0;
	PlayCallback on_finished;

	SDL_Texture *texture = nullptr;
	std::vector<SDL_Rect> rect_src_list; // 各フレームのソース矩形リストを格納

	int width_frame = 0, height_frame = 0; // 各フレームの幅と高さ
};

#endif // !_ANIMATION_H_
