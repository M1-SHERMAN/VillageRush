#ifndef _VECTOR2_H_
#define _VECTOR2_H_

#include <cmath>

class Vector2
{
public:
	// ゼロベクトルで初期化
	double x = 0;
	double y = 0;

public:
	Vector2() = default;
	~Vector2() = default;

	// xとy成分を初期化するためのパラメータ付きコンストラクタ
	Vector2(double x, double y) : x(x), y(y) {}

	// +演算子のオーバーロード、ベクトルの加算を実現
	Vector2 operator+(const Vector2 &vec) const
	{
		return Vector2(x + vec.x, y + vec.y);
	}

	// +=演算子のオーバーロード、ベクトルの加算代入を実現
	void operator+=(const Vector2 &vec)
	{
		x += vec.x;
		y += vec.y;
	}

	// -演算子のオーバーロード、ベクトルの減算を実現
	Vector2 operator-(const Vector2 &vec) const
	{
		return Vector2(x - vec.x, y - vec.y);
	}

	// -=演算子のオーバーロード、ベクトルの減算代入を実現
	void operator-=(const Vector2 &vec)
	{
		x -= vec.x;
		y -= vec.y;
	}

	// *演算子のオーバーロード、ベクトルの内積を実現、スカラーを返す
	double operator*(const Vector2 &vec) const
	{
		return x * vec.x + y * vec.y;
	}

	// *演算子のオーバーロード、ベクトルとスカラーの乗算を実現
	Vector2 operator*(double val) const
	{
		return Vector2(x * val, y * val);
	}

	// *=演算子のオーバーロード、スカラー乗算の代入を実現
	void operator*=(double val)
	{
		x *= val, y *= val;
	}

	// ==演算子のオーバーロード、2つのベクトルが等しいかを比較
	bool operator==(const Vector2 &vec) const
	{
		return x == vec.x && y == vec.y;
	}

	// >演算子のオーバーロード、現在のベクトルが他のベクトルより大きいかを比較（長さによる）
	bool operator>(const Vector2 &vec) const
	{
		return length() > vec.length();
	}

	// <演算子のオーバーロード、現在のベクトルが他のベクトルより小さいかを比較（長さによる）
	bool operator<(const Vector2 &vec) const
	{
		return length() < vec.length();
	}
	// ベクトルの長さ（大きさ）を計算
	double length() const
	{
		return sqrt(x * x + y * y);
	}

	// ベクトルを正規化し、単位ベクトルを返す
	Vector2 normalize() const
	{
		double len = length();
		if (len == 0)
			return Vector2(0, 0);
		return Vector2(x / len, y / len);
	}

	// ベクトルがゼロに近いかを判定
	bool approx_zero() const
	{
		return length() < 0.0001;
	}

private:
};

#endif // !_VECTOR_H_
