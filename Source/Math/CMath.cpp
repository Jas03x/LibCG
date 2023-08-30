#include "CMath.hpp"

#include <cmath>

#include <algorithm>

#include <strsafe.h>

#include "Cg.hpp"
#include "CgDef.hpp"
#include "CgMath.hpp"

#define CMP(x0, x1)						\
	if ((x0) < (x1)) return true;		\
	else if ((x0) > (x1)) return false;	\

#define ADD2(v0, v1, v) { (v)[0] = (v0)[0] + (v1)[0]; (v)[1] = (v0)[1] + (v1)[1]; }
#define SUB2(v0, v1, v) { (v)[0] = (v0)[0] - (v1)[0]; (v)[1] = (v0)[1] - (v1)[1]; }
#define ADD3(v0, v1, v) { (v)[0] = (v0)[0] + (v1)[0]; (v)[1] = (v0)[1] + (v1)[1]; (v)[2] = (v0)[2] + (v1)[2]; }
#define SUB3(v0, v1, v) { (v)[0] = (v0)[0] - (v1)[0]; (v)[1] = (v0)[1] - (v1)[1]; (v)[2] = (v0)[2] - (v1)[2]; }
#define ADD4(v0, v1, v) { (v)[0] = (v0)[0] + (v1)[0]; (v)[1] = (v0)[1] + (v1)[1]; (v)[2] = (v0)[2] + (v1)[2]; (v)[3] = (v0)[3] + (v1)[3]; }
#define SUB4(v0, v1, v) { (v)[0] = (v0)[0] - (v1)[0]; (v)[1] = (v0)[1] - (v1)[1]; (v)[2] = (v0)[2] - (v1)[2]; (v)[3] = (v0)[3] - (v1)[3]; }

#define MUL2(a, v0, v) { (v)[0] = (a) * (v0)[0]; (v)[1] = (a) * (v0)[1]; }
#define MUL3(a, v0, v) { (v)[0] = (a) * (v0)[0]; (v)[1] = (a) * (v0)[1]; (v)[2] = (a) * (v0)[2]; }
#define MUL4(a, v0, v) { (v)[0] = (a) * (v0)[0]; (v)[1] = (a) * (v0)[1]; (v)[2] = (a) * (v0)[2]; (v)[3] = (a) * (v0)[3]; }

#define SQR(x) ((x) * (x))

#define __DOT2(x0, y0, x1, y1) (((x0) * (x1)) + ((y0) * (y1)))
#define __DOT3(x0, y0, z0, x1, y1, z1) (((x0) * (x1)) + ((y0) * (y1)) + ((z0) * (z1)))
#define __DOT4(x0, y0, z0, w0, x1, y1, z1, w1) (((x0) * (x1)) + ((y0) * (y1)) + ((z0) * (z1)) + ((w0) * (w1)))

#define DOT2(m, m0, m1, r, c) (m)[c][r] = __DOT2((m0)[0][r], (m0)[1][r], (m1)[c][0], (m1)[c][1])
#define DOT3(m, m0, m1, r, c) (m)[c][r] = __DOT3((m0)[0][r], (m0)[1][r], (m0)[2][r], (m1)[c][0], (m1)[c][1], (m1)[c][2])
#define DOT4(m, m0, m1, r, c) (m)[c][r] = __DOT4((m0)[0][r], (m0)[1][r], (m0)[2][r], (m0)[3][r], (m1)[c][0], (m1)[c][1], (m1)[c][2], (m1)[c][3])

#define DET2(a, b, c, d) ((a) * (d) - (b) * (c))
#define DET3(a, b, c, d, e, f, g, h, i) (((a) * DET2((e), (f), (h), (i))) - ((b) * DET2((d), (f), (g), (i))) + ((c) * DET2((d), (e), (g), (h))))

// ------------------------------------ Helper functions ------------------------------------------

template <typename T> void WriteToBuffer(wchar_t*& pBuffer, size_t& szBuffer, T t)
{
	if constexpr (std::is_arithmetic<T>::value)
	{
		if constexpr (std::is_floating_point<T>::value) { StringCchPrintfExW(pBuffer, szBuffer, &pBuffer, &szBuffer, 0, L"%f", t); }
		else if constexpr (std::is_unsigned<T>::value)  { StringCchPrintfExW(pBuffer, szBuffer, &pBuffer, &szBuffer, 0, L"%llu", static_cast<uint64_t>(t)); }
		else											{ StringCchPrintfExW(pBuffer, szBuffer, &pBuffer, &szBuffer, 0, L"%lli", static_cast<int64_t>(t));  }
	}
	else												{ StringCchPrintfExW(pBuffer, szBuffer, &pBuffer, &szBuffer, 0, L"%s", t); }
}

// ------------------------------------ Scalar functions ------------------------------------------

template <typename T>
T Step(T v0, T v1, T step)
{
	T d = v1 - v0;
	
	if constexpr (std::is_floating_point<T>::value) { return (std::fabs(d) > step) ? (v0 + std::copysign(step, d)) : v1; }
	else if constexpr (std::is_signed<T>::value)    { return static_cast<T>((std::abs(static_cast<int64_t>(d)) > step) ? (v0 + std::copysign(step, d)) : v1); }
	else                                            { return static_cast<T>((d > step) ? (v0 + step) : v1); }
}

template <typename T>
T Clamp(T val, T min, T max)
{
	return std::max(std::min(val, max), min);
}

// ----------------------------- Scalar function instantiations -----------------------------------

#define INSTANTIATE_SCALAR_FUNCTION_TEMPLATES(X) \
	template X Step(X v0, X v1, X step);		 \
    template X Clamp(X val, X min, X max);       \

INSTANTIATE_SCALAR_FUNCTION_TEMPLATES(int8_t)
INSTANTIATE_SCALAR_FUNCTION_TEMPLATES(int16_t)
INSTANTIATE_SCALAR_FUNCTION_TEMPLATES(int32_t)
INSTANTIATE_SCALAR_FUNCTION_TEMPLATES(int64_t)
INSTANTIATE_SCALAR_FUNCTION_TEMPLATES(uint8_t)
INSTANTIATE_SCALAR_FUNCTION_TEMPLATES(uint16_t)
INSTANTIATE_SCALAR_FUNCTION_TEMPLATES(uint32_t)
INSTANTIATE_SCALAR_FUNCTION_TEMPLATES(uint64_t)
INSTANTIATE_SCALAR_FUNCTION_TEMPLATES(float)
INSTANTIATE_SCALAR_FUNCTION_TEMPLATES(double)

// ----------------------------------------- Vector2 ----------------------------------------------

template <typename T> Vector2<T>::Vector2() { for (uint32_t i = 0; i < 2; i++) { elements[i] = 0; } }
template <typename T> Vector2<T>::Vector2(T t) : Vector2<T>(t, t) { }
template <typename T> Vector2<T>::Vector2(const Vector2<T>& v) : Vector2<T>(v.x, v.y) { }
template <typename T> Vector2<T>::Vector2(T _x, T _y) : Vector2<T>() { x = _x; y = _y; }

template <typename T> Vector2<T>& Vector2<T>::operator = (const Vector2<T>& v)
{
	this->x = v.x;
	this->y = v.y;
	return *this;
}

template <typename T> T& Vector2<T>::operator[] (uint32_t i) { return elements[i]; }
template <typename T> const T& Vector2<T>::operator[] (uint32_t i) const { return elements[i]; }

template <typename T> Vector2<T> Vector2<T>::operator + () const { return Vector2<T>(+x, +y); }
template <typename T> Vector2<T> Vector2<T>::operator - () const
{
	if constexpr (std::is_signed<T>::value) { return Vector2<T>(-x, -y); }
	else { CgAssert(false, __FUNCTION__ L": cannot use unary minus operator on unsigned type"); return *this; }
}

template <typename T> Vector2<T>  Vector2<T>::operator +  (const Vector2<T>& v) const { return Vector2<T>(x + v.x, y + v.y); }
template <typename T> Vector2<T>  Vector2<T>::operator -  (const Vector2<T>& v) const { return Vector2<T>(x - v.x, y - v.y); }
template <typename T> Vector2<T>& Vector2<T>::operator += (const Vector2<T>& v) { x += v.x; y += v.y; return *this; }
template <typename T> Vector2<T>& Vector2<T>::operator -= (const Vector2<T>& v) { x -= v.x; y -= v.y; return *this; }

template <typename T> Vector2<T>  Vector2<T>::operator *  (T t) const { return Vector2<T>(x * t, y * t); }
template <typename T> Vector2<T>& Vector2<T>::operator *= (T t) { x *= t; y *= t; return *this; }

template <typename T> Vector2<T> operator * (T t, Vector2<T>& v) { return v * t; }

template <typename T> bool Vector2<T>::operator < (const Vector2<T>& v) const
{
	CMP(this->x, v.x);
	CMP(this->y, v.y);
	return false;
}

template <typename T> bool Vector2<T>::operator == (const Vector2<T>& v) const { return (x == v.x) && (y == v.y); }
template <typename T> bool Vector2<T>::operator != (const Vector2<T>& v) const { return (x != v.x) || (y != v.y); }

// ----------------------------------------- Vector3 ----------------------------------------------

template <typename T> Vector3<T>::Vector3() { for (uint32_t i = 0; i < 3; i++) { elements[i] = 0; } }
template <typename T> Vector3<T>::Vector3(T t) : Vector3<T>(t, t, t) { }
template <typename T> Vector3<T>::Vector3(const Vector3<T>& v) : Vector3<T>(v.x, v.y, v.z) { }
template <typename T> Vector3<T>::Vector3(T _x, T _y, T _z) : Vector3<T>() { x = _x; y = _y; z = _z; }

template <typename T> Vector3<T>& Vector3<T>::operator = (const Vector3<T>& v)
{
	this->x = v.x;
	this->y = v.y;
	this->z = v.z;
	return *this;
}

template <typename T> T& Vector3<T>::operator[] (uint32_t i) { return elements[i]; }
template <typename T> const T& Vector3<T>::operator[] (uint32_t i) const { return elements[i]; }

template <typename T> Vector3<T> Vector3<T>::operator + () const { return Vector3<T>(+x, +y, +z); }
template <typename T> Vector3<T> Vector3<T>::operator - () const
{
	if constexpr (std::is_signed<T>::value) { return Vector3<T>(-x, -y, -z); }
	else { CgAssert(false, __FUNCTION__ L": cannot use unary minus operator on unsigned type"); return *this; }
}

template <typename T> Vector3<T>  Vector3<T>::operator +  (const Vector3<T>& v) const { return Vector3<T>(x + v.x, y + v.y, z + v.z); }
template <typename T> Vector3<T>  Vector3<T>::operator -  (const Vector3<T>& v) const { return Vector3<T>(x - v.x, y - v.y, z - v.z); }
template <typename T> Vector3<T>& Vector3<T>::operator += (const Vector3<T>& v) { x += v.x; y += v.y; z += v.z; return *this; }
template <typename T> Vector3<T>& Vector3<T>::operator -= (const Vector3<T>& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }

template <typename T> Vector3<T>  Vector3<T>::operator *  (T t) const { return Vector3<T>(x * t, y * t, z * t); }
template <typename T> Vector3<T>& Vector3<T>::operator *= (T t) { x *= t; y *= t; z *= t; return *this; }

template <typename T> Vector3<T> operator * (T t, Vector3<T>& v) { return v * t; }

template <typename T> bool Vector3<T>::operator < (const Vector3<T>& v) const
{
	CMP(this->x, v.x);
	CMP(this->y, v.y);
	CMP(this->z, v.z);
	return false;
}

template <typename T> bool Vector3<T>::operator == (const Vector3<T>& v) const { return (x == v.x) && (y == v.y) && (z == v.z); }
template <typename T> bool Vector3<T>::operator != (const Vector3<T>& v) const { return (x != v.x) || (y != v.y) || (z != v.z); }

// ----------------------------------------- Vector4 ----------------------------------------------

template <typename T> Vector4<T>::Vector4() { for (uint32_t i = 0; i < 4; i++) { elements[i] = 0; } }
template <typename T> Vector4<T>::Vector4(T t) : Vector4<T>(t, t, t, t) { }
template <typename T> Vector4<T>::Vector4(const Vector4<T>& v) : Vector4<T>(v.x, v.y, v.z, v.w) { }
template <typename T> Vector4<T>::Vector4(T _x, T _y, T _z, T _w) : Vector4<T>() { x = _x; y = _y; z = _z; w = _w; }

template <typename T> Vector4<T>& Vector4<T>::operator = (const Vector4<T>& v)
{
	this->x = v.x;
	this->y = v.y;
	this->z = v.z;
	this->w = v.w;
	return *this;
}

template <typename T> T& Vector4<T>::operator[] (uint32_t i) { return elements[i]; }
template <typename T> const T& Vector4<T>::operator[] (uint32_t i) const { return elements[i]; }

template <typename T> Vector4<T> Vector4<T>::operator + () const { return Vector4<T>(+x, +y, +z, +w); }
template <typename T> Vector4<T> Vector4<T>::operator - () const
{
	if constexpr (std::is_signed<T>::value) { return Vector4<T>(-x, -y, -z, -w); }
	else { CgAssert(false, __FUNCTION__ L": cannot use unary minus operator on unsigned type"); return *this; }
}

template <typename T> Vector4<T>  Vector4<T>::operator +  (const Vector4<T>& v) const { return Vector4<T>(x + v.x, y + v.y, z + v.z, w + v.w); }
template <typename T> Vector4<T>  Vector4<T>::operator -  (const Vector4<T>& v) const { return Vector4<T>(x - v.x, y - v.y, z - v.z, w - v.w); }
template <typename T> Vector4<T>& Vector4<T>::operator += (const Vector4<T>& v) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
template <typename T> Vector4<T>& Vector4<T>::operator -= (const Vector4<T>& v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }

template <typename T> Vector4<T>  Vector4<T>::operator *  (T t) const { return Vector4<T>(x * t, y * t, z * t, w * t); }
template <typename T> Vector4<T>& Vector4<T>::operator *= (T t) { x *= t; y *= t; z *= t; w *= t; return *this; }

template <typename T> Vector4<T> operator * (T t, Vector4<T>& v) { return v * t; }

template <typename T> bool Vector4<T>::operator < (const Vector4<T>& v) const
{
	CMP(this->x, v.x);
	CMP(this->y, v.y);
	CMP(this->z, v.z);
	CMP(this->w, v.w);
	return false;
}

template <typename T> bool Vector4<T>::operator == (const Vector4<T>& v) const { return (x == v.x) && (y == v.y) && (z == v.z) && (w == v.w); }
template <typename T> bool Vector4<T>::operator != (const Vector4<T>& v) const { return (x != v.x) || (y != v.y) || (z != v.z) || (w != v.w); }

// ------------------------------------ Vector functions ------------------------------------------

template <typename T> T Vector::Length(const Vector2<T>& v) { return std::sqrt(SQR(v.x) + SQR(v.y)); }
template <typename T> T Vector::Length(const Vector3<T>& v) { return std::sqrt(SQR(v.x) + SQR(v.y) + SQR(v.z)); }
template <typename T> T Vector::Length(const Vector4<T>& v) { return std::sqrt(SQR(v.x) + SQR(v.y) + SQR(v.z) + SQR(v.w)); }

template <typename T> T Vector::Dot(const Vector2<T>& v0, const Vector2<T>& v1) { return (v0.x * v1.x) + (v0.y + v1.y); }
template <typename T> T Vector::Dot(const Vector3<T>& v0, const Vector3<T>& v1) { return (v0.x * v1.x) + (v0.y + v1.y) + (v0.z * v1.z); }
template <typename T> T Vector::Dot(const Vector4<T>& v0, const Vector4<T>& v1) { return (v0.x * v1.x) + (v0.y + v1.y) + (v0.z * v1.z) + (v0.w * v1.w); }

template <typename T> Vector2<T> Vector::Normalize(const Vector2<T>& v) { return v * (1.0f / Vector::Length(v)); }
template <typename T> Vector3<T> Vector::Normalize(const Vector3<T>& v) { return v * (1.0f / Vector::Length(v)); }
template <typename T> Vector4<T> Vector::Normalize(const Vector4<T>& v) { return v * (1.0f / Vector::Length(v)); }

template <typename T> Vector3<T> Vector::Cross(const Vector3<T>& v0, const Vector3<T>& v1)
{
	T x = (v0.y * v1.z) - (v0.z * v1.y);
	T y = (v0.x * v1.z) - (v0.z * v1.x);
	T z = (v0.x * v1.y) - (v0.y * v1.x);
	return Vector3<T>(x, -y, z);
}

template <typename T> T Vector::Distance(const Vector2<T>& v0, const Vector2<T>& v1) { return std::sqrt(SQR(v1.x - v0.x) + SQR(v1.y - v0.y)); }
template <typename T> T Vector::Distance(const Vector3<T>& v0, const Vector3<T>& v1) { return std::sqrt(SQR(v1.x - v0.x) + SQR(v1.y - v0.y) + SQR(v1.z - v0.z)); }
template <typename T> T Vector::Distance(const Vector4<T>& v0, const Vector4<T>& v1) { return std::sqrt(SQR(v1.x - v0.x) + SQR(v1.y - v0.y) + SQR(v1.z - v0.z) + SQR(v1.w - v0.w)); }

template <typename T> std::wstring VectorToString(T* pElements, size_t nElements)
{
	wchar_t buffer[2048] = {};

	wchar_t* pBuffer = buffer;
	size_t szBuffer = _countof(buffer);

	WriteToBuffer(pBuffer, szBuffer, L"<");
	for (uint32_t i = 0; i < nElements; i++)
	{
		WriteToBuffer(pBuffer, szBuffer, pElements[i]);
		if (i != (nElements - 1))
		{
			WriteToBuffer(pBuffer, szBuffer, L", ");
		}
	}
	WriteToBuffer(pBuffer, szBuffer, L">");
	
	*pBuffer = 0; // null-terminate

	return std::wstring(buffer);
}

template <typename T> std::wstring Vector::ToString(const Vector2<T>& v) { return VectorToString(v.elements, _countof(v.elements)); }
template <typename T> std::wstring Vector::ToString(const Vector3<T>& v) { return VectorToString(v.elements, _countof(v.elements)); }
template <typename T> std::wstring Vector::ToString(const Vector4<T>& v) { return VectorToString(v.elements, _countof(v.elements)); }

// ------------------------- Vector template/function instantiations ------------------------------

#define INSTANTIATE_VECTOR_TEMPLATES_FOR_TYPE(X)                                        \
    template struct Vector2<X>;                                                         \
    template struct Vector3<X>;                                                         \
    template struct Vector4<X>;                                                         \
	template std::wstring Vector::ToString(const Vector2<X>& v);						\
	template std::wstring Vector::ToString(const Vector3<X>& v);						\
	template std::wstring Vector::ToString(const Vector4<X>& v);						\

#define INSTANTIATE_VECTOR_TEMPLATES_FOR_FLOATING_POINT_TYPE(X)							\
	INSTANTIATE_VECTOR_TEMPLATES_FOR_TYPE(X)											\
    template X Vector::Length(const Vector2<X>& v);                                     \
    template X Vector::Length(const Vector3<X>& v);                                     \
    template X Vector::Length(const Vector4<X>& v);                                     \
    template X Vector::Dot(const Vector2<X>& v0, const Vector2<X>& v1);                 \
    template X Vector::Dot(const Vector3<X>& v0, const Vector3<X>& v1);                 \
    template X Vector::Dot(const Vector4<X>& v0, const Vector4<X>& v1);                 \
    template Vector2<X> Vector::Normalize(const Vector2<X>& v0);                        \
    template Vector3<X> Vector::Normalize(const Vector3<X>& v0);                        \
    template Vector4<X> Vector::Normalize(const Vector4<X>& v0);                        \
    template Vector3<X> Vector::Cross(const Vector3<X>& v0, const Vector3<X>& v1);      \
	template X Vector::Distance(const Vector2<X>& v0, const Vector2<X>& v1);            \
    template X Vector::Distance(const Vector3<X>& v0, const Vector3<X>& v1);            \
    template X Vector::Distance(const Vector4<X>& v0, const Vector4<X>& v1);            \

INSTANTIATE_VECTOR_TEMPLATES_FOR_TYPE(int8_t)
INSTANTIATE_VECTOR_TEMPLATES_FOR_TYPE(int16_t)
INSTANTIATE_VECTOR_TEMPLATES_FOR_TYPE(int32_t)
INSTANTIATE_VECTOR_TEMPLATES_FOR_TYPE(int64_t)
INSTANTIATE_VECTOR_TEMPLATES_FOR_TYPE(uint8_t)
INSTANTIATE_VECTOR_TEMPLATES_FOR_TYPE(uint16_t)
INSTANTIATE_VECTOR_TEMPLATES_FOR_TYPE(uint32_t)
INSTANTIATE_VECTOR_TEMPLATES_FOR_TYPE(uint64_t)

INSTANTIATE_VECTOR_TEMPLATES_FOR_FLOATING_POINT_TYPE(float)
INSTANTIATE_VECTOR_TEMPLATES_FOR_FLOATING_POINT_TYPE(double)

// --------------------------------------- Quaternion ---------------------------------------------

template <typename T> Quaternion<T>::Quaternion() { for (uint32_t i = 0; i < 4; i++) { elements[i] = 0; } }
template <typename T> Quaternion<T>::Quaternion(const Vector3<T>& v) : Quaternion<T>(v.x, v.y, v.z) { }
template <typename T> Quaternion<T>::Quaternion(const Vector4<T>& v) : Quaternion<T>(v.x, v.y, v.z, v.w) { }
template <typename T> Quaternion<T>::Quaternion(T _x, T _y, T _z, T _w) : Quaternion<T>() { x = _x; y = _y; z = _z; w = _w; }

template <typename T> Quaternion<T>::Quaternion(T _x, T _y, T _z) : Quaternion<T>()
{
	const Vector3F h = Vector3<T>(_x, _y, _z) * 0.5f; // half-rotation vector
	const Vector3F c(cosf(h.x), cosf(h.y), cosf(h.z));
	const Vector3F s(sinf(h.x), sinf(h.y), sinf(h.z));

	w = c.x * c.y * c.z + s.x * s.y * s.z;
	x = s.x * c.y * c.z - c.x * s.y * s.z;
	y = c.x * s.y * c.z + s.x * c.y * s.z;
	z = c.x * c.y * s.z - s.x * s.y * c.z;
}

template <typename T> Quaternion<T>::Quaternion(const struct Matrix3<T>& m) : Quaternion<T>()
{
	if (m[0][0] + m[1][1] + m[2][2] >= static_cast<T>(0)) // q_w >= 1/2
	{
		float f = static_cast<T>(2) * sqrt(1 + m[0][0] + m[1][1] + m[2][2]);
		w = static_cast<T>(0.25) * f;
		x = (m[1][2] - m[2][1])  / f;
		y = (m[2][0] - m[0][2])  / f;
		z = (m[0][1] - m[1][0])  / f;
	}
	else if (m[0][0] - m[1][1] - m[2][2] >= static_cast<T>(0)) // q_x >= 1/2
	{
		float f = static_cast<T>(2) * sqrt(1 + m[0][0] - m[1][1] - m[2][2]);
		w = (m[1][2] - m[2][1])  / f;
		x = static_cast<T>(0.25) * f;
		y = (m[1][0] + m[0][1])  / f;
		z = (m[2][0] + m[0][2])  / f;
	}
	else if (-m[0][0] + m[1][1] - m[2][2] >= static_cast<T>(0)) // q_y >= 1/2
	{
		float f = static_cast<T>(2) * sqrt(1 - m[0][0] + m[1][1] - m[2][2]);
		w = (m[2][0] - m[0][2])  / f;
		x = (m[1][0] + m[0][1])  / f;
		y = static_cast<T>(0.25) * f;
		z = (m[1][2] + m[2][1])  / f;
	}
	else // (-m[0][0] - m[1][1] + m[2][2] >= 0) => q_z >= 1/2
	{
		float f = static_cast<T>(2) * sqrt(1 - m[0][0] - m[1][1] + m[2][2]);
		w = (m[0][1] - m[1][0])  / f;
		x = (m[2][0] + m[0][2])  / f;
		y = (m[1][2] + m[2][1])  / f;
		z = static_cast<T>(0.25) * f;
	}
}

template <typename T> Quaternion<T> Quaternion<T>::operator * (const Quaternion<T> & q) const
{
	Quaternion<T> r;
	r.w = w*q.w - x*q.x - y*q.y - z*q.z;
	r.x = w*q.x + x*q.w + y*q.z - z*q.y;
	r.y = w*q.y - x*q.z + y*q.w + z*q.x;
	r.z = w*q.z + x*q.y - y*q.x + z*q.w;
	return r;
}

template <typename T> Quaternion<T>& Quaternion<T>::operator *= (const Quaternion<T>& q)
{
	Quaternion<T> r = *this;
	w = r.w*q.w - r.x*q.x - r.y*q.y - r.z*q.z;
	x = r.w*q.x + r.x*q.w + r.y*q.z - r.z*q.y;
	y = r.w*q.y - r.x*q.z + r.y*q.w + r.z*q.x;
	z = r.w*q.z + r.x*q.y - r.y*q.x + r.z*q.w;
	return *this;
}

template <typename T> Quaternion<T>& Quaternion<T>::operator = (const Vector4<T>& v)
{
	w = v.w;
	x = v.x;
	y = v.y;
	z = v.z;
	return *this;
}

template <typename T> Quaternion<T>& Quaternion<T>::operator = (const Quaternion& q)
{
	w = q.w;
	x = q.x;
	y = q.y;
	z = q.z;
	return *this;
}

#define INSTANTIATE_QUATERNION_TEMPLATES_FOR_FLOATING_POINT_TYPE(X) \
    template struct Quaternion<X>;

INSTANTIATE_QUATERNION_TEMPLATES_FOR_FLOATING_POINT_TYPE(float)

// ----------------------------------------- Matrix2 ----------------------------------------------

template <typename T> Matrix2<T>::Matrix2() : Matrix2<T>(static_cast<T>(1)) { }
template <typename T> Matrix2<T>::Matrix2(T t)
{
	rows[0] = Vector2<T>(t, 0);
	rows[1] = Vector2<T>(0, t);
}
template <typename T> Matrix2<T>::Matrix2(const Vector2<T>& v0, const Vector2<T>& v1)
{
	rows[0] = v0;
	rows[1] = v1;
}
template <typename T> Matrix2<T>::Matrix2(const Matrix2<T>& m) : Matrix2<T>(m[0], m[1]) { }

template <typename T> Matrix2<T>& Matrix2<T>::operator = (const Matrix2<T>& m)
{
	this->rows[0] = m.rows[0];
	this->rows[1] = m.rows[1];
	return *this;
}

template <typename T> Vector2<T>& Matrix2<T>::operator[] (uint32_t i) { return rows[i]; }
template <typename T> const Vector2<T>& Matrix2<T>::operator[] (uint32_t i) const { return rows[i]; }

template <typename T> Matrix2<T> Matrix2<T>::operator * (T t) const
{
	Matrix2<T> r;
	MUL2(t, this->elements[0], r.elements[0]);
	MUL2(t, this->elements[1], r.elements[1]);
	return r;
}

template <typename T> Matrix2<T>& Matrix2<T>::operator *= (T t)
{
	MUL2(t, this->elements[0], this->elements[0]);
	MUL2(t, this->elements[1], this->elements[1]);
	return *this;
}

template <typename T> Matrix2<T> Matrix2<T>::operator + (const Matrix2<T>& m) const
{
	Matrix2<T> r;
	ADD2(this->elements[0], m.elements[0], r.elements[0]);
	ADD2(this->elements[1], m.elements[1], r.elements[1]);
	return r;
}

template <typename T> Matrix2<T> Matrix2<T>::operator - (const Matrix2<T>& m) const
{
	Matrix2<T> r;
	SUB2(this->elements[0], m.elements[0], r.elements[0]);
	SUB2(this->elements[1], m.elements[1], r.elements[1]);
	return r;
}

template <typename T> Matrix2<T>& Matrix2<T>::operator += (const Matrix2<T>& m)
{
	ADD2(this->elements[0], m.elements[0], this->elements[0]);
	ADD2(this->elements[1], m.elements[1], this->elements[1]);
	return *this;
}

template <typename T> Matrix2<T>& Matrix2<T>::operator -= (const Matrix2<T>& m)
{
	SUB2(this->elements[0], m.elements[0], this->elements[0]);
	SUB2(this->elements[1], m.elements[1], this->elements[1]);
	return *this;
}

template <typename T> Matrix2<T> Matrix2<T>::operator * (const Matrix2<T>& m) const
{
	Matrix2<T> r;
	DOT2(r.rows, this->rows, m.rows, 0, 0);
	DOT2(r.rows, this->rows, m.rows, 0, 1);
	DOT2(r.rows, this->rows, m.rows, 1, 0);
	DOT2(r.rows, this->rows, m.rows, 1, 1);
	return r;
}

template <typename T> Matrix2<T>& Matrix2<T>::operator *= (const Matrix2<T>& m)
{
	Matrix2<T> c(*this);
	DOT2(this->rows, c.rows, m.rows, 0, 0);
	DOT2(this->rows, c.rows, m.rows, 0, 1);
	DOT2(this->rows, c.rows, m.rows, 1, 0);
	DOT2(this->rows, c.rows, m.rows, 1, 1);
	return *this;
}

template <typename T> Vector2<T> Matrix2<T>::operator * (const Vector2<T>& v) const
{
	return Vector2<T>(
		Vector::Dot<T>(v, rows[0]),
		Vector::Dot<T>(v, rows[1])
	);
}

// ----------------------------------------- Matrix3 ----------------------------------------------

template <typename T> Matrix3<T>::Matrix3() : Matrix3<T>(static_cast<T>(1)) { }
template <typename T> Matrix3<T>::Matrix3(T t)
{
	rows[0] = Vector3<T>(t, 0, 0);
	rows[1] = Vector3<T>(0, t, 0);
	rows[2] = Vector3<T>(0, 0, t);
}
template <typename T> Matrix3<T>::Matrix3(const Vector3<T>& v0, const Vector3<T>& v1, const Vector3<T>& v2)
{
	rows[0] = v0;
	rows[1] = v1;
	rows[2] = v2;
}
template <typename T> Matrix3<T>::Matrix3(const Matrix3<T>& m) : Matrix3<T>(m[0], m[1], m[2]) { }

template <typename T> Matrix3<T>& Matrix3<T>::operator = (const Matrix3<T>& m)
{
	rows[0] = m.rows[0];
	rows[1] = m.rows[1];
	rows[2] = m.rows[2];
	return *this;
}

template <typename T> Vector3<T>& Matrix3<T>::operator[] (uint32_t i) { return rows[i]; }
template <typename T> const Vector3<T>& Matrix3<T>::operator[] (uint32_t i) const { return rows[i]; }

template <typename T> Matrix3<T> Matrix3<T>::operator * (T t) const
{
	Matrix3<T> r;
	MUL3(t, this->elements[0], r.elements[0]);
	MUL3(t, this->elements[1], r.elements[1]);
	MUL3(t, this->elements[2], r.elements[2]);
	return r;
}

template <typename T> Matrix3<T>& Matrix3<T>::operator *= (T t)
{
	MUL3(t, this->elements[0], this->elements[0]);
	MUL3(t, this->elements[1], this->elements[1]);
	MUL3(t, this->elements[2], this->elements[2]);
	return *this;
}

template <typename T> Matrix3<T> Matrix3<T>::operator + (const Matrix3<T>& m) const
{
	Matrix3<T> r;
	ADD3(this->elements[0], m.elements[0], r.elements[0]);
	ADD3(this->elements[1], m.elements[1], r.elements[1]);
	ADD3(this->elements[2], m.elements[2], r.elements[2]);
	return r;
}

template <typename T> Matrix3<T> Matrix3<T>::operator - (const Matrix3<T>& m) const
{
	Matrix3<T> r;
	SUB3(this->elements[0], m.elements[0], r.elements[0]);
	SUB3(this->elements[1], m.elements[1], r.elements[1]);
	SUB3(this->elements[2], m.elements[2], r.elements[2]);
	return r;
}

template <typename T> Matrix3<T>& Matrix3<T>::operator += (const Matrix3<T>& m)
{
	ADD3(this->elements[0], m.elements[0], this->elements[0]);
	ADD3(this->elements[1], m.elements[1], this->elements[1]);
	ADD3(this->elements[2], m.elements[2], this->elements[2]);
	return *this;
}

template <typename T> Matrix3<T>& Matrix3<T>::operator -= (const Matrix3<T>& m)
{
	SUB3(this->elements[0], m.elements[0], this->elements[0]);
	SUB3(this->elements[1], m.elements[1], this->elements[1]);
	SUB3(this->elements[2], m.elements[2], this->elements[2]);
	return *this;
}

template <typename T> Matrix3<T> Matrix3<T>::operator * (const Matrix3<T>& m) const
{
	Matrix3<T> r;
	DOT3(r.rows, rows, m.rows, 0, 0);
	DOT3(r.rows, rows, m.rows, 0, 1);
	DOT3(r.rows, rows, m.rows, 0, 2);
	DOT3(r.rows, rows, m.rows, 1, 0);
	DOT3(r.rows, rows, m.rows, 1, 1);
	DOT3(r.rows, rows, m.rows, 1, 2);
	DOT3(r.rows, rows, m.rows, 2, 0);
	DOT3(r.rows, rows, m.rows, 2, 1);
	DOT3(r.rows, rows, m.rows, 2, 2);
	return r;
}

template <typename T> Matrix3<T>& Matrix3<T>::operator *= (const Matrix3<T>& m)
{
	Matrix3<T> c(*this);
	DOT3(rows, c.rows, m.rows, 0, 0);
	DOT3(rows, c.rows, m.rows, 0, 1);
	DOT3(rows, c.rows, m.rows, 0, 2);
	DOT3(rows, c.rows, m.rows, 1, 0);
	DOT3(rows, c.rows, m.rows, 1, 1);
	DOT3(rows, c.rows, m.rows, 1, 2);
	DOT3(rows, c.rows, m.rows, 2, 0);
	DOT3(rows, c.rows, m.rows, 2, 1);
	DOT3(rows, c.rows, m.rows, 2, 2);
	return *this;
}

template <typename T> Vector3<T> Matrix3<T>::operator * (const Vector3<T>& v) const
{
	return Vector3<T>(
		Vector::Dot<T>(v, rows[0]),
		Vector::Dot<T>(v, rows[1]),
		Vector::Dot<T>(v, rows[2])
	);
}

// ----------------------------------------- Matrix4 ----------------------------------------------

template <typename T> Matrix4<T>::Matrix4() : Matrix4<T>(static_cast<T>(1)) { }
template <typename T> Matrix4<T>::Matrix4(T t)
{
	rows[0] = Vector4<T>(t, 0, 0, 0);
	rows[1] = Vector4<T>(0, t, 0, 0);
	rows[2] = Vector4<T>(0, 0, t, 0);
	rows[3] = Vector4<T>(0, 0, 0, t);
}
template <typename T> Matrix4<T>::Matrix4(const Vector4<T>& v0, const Vector4<T>& v1, const Vector4<T>& v2, const Vector4<T>& v3)
{
	rows[0] = v0;
	rows[1] = v1;
	rows[2] = v2;
	rows[3] = v3;
}
template <typename T> Matrix4<T>::Matrix4(const Matrix4<T>& m) : Matrix4<T>(m[0], m[1], m[2], m[3]) { }

template <typename T> Matrix4<T>::Matrix4(const Quaternion<T>& q) : Matrix4<T>(1)
{
	float qxx = q.x * q.x;
	float qxy = q.x * q.y;
	float qxz = q.x * q.z;
	float qxw = q.w * q.x;
	float qyy = q.y * q.y;
	float qyz = q.y * q.z;
	float qyw = q.w * q.y;
	float qzz = q.z * q.z;
	float qzw = q.w * q.z;

	elements[0][0] = 1.0f - 2.0f * (qyy + qzz);
	elements[0][1] = 2.0f * (qxy + qzw);
	elements[0][2] = 2.0f * (qxz - qyw);

	elements[1][0] = 2.0f * (qxy - qzw);
	elements[1][1] = 1.0f - 2.0f * (qxx + qzz);
	elements[1][2] = 2.0f * (qyz + qxw);

	elements[2][0] = 2.0f * (qxz + qyw);
	elements[2][1] = 2.0f * (qyz - qxw);
	elements[2][2] = 1.0f - 2.0f * (qxx + qyy);
}

template <typename T> Matrix4<T>& Matrix4<T>::operator = (const Matrix4<T>& m)
{
	rows[0] = m.rows[0];
	rows[1] = m.rows[1];
	rows[2] = m.rows[2];
	rows[3] = m.rows[3];
	return *this;
}

template <typename T> Vector4<T>& Matrix4<T>::operator[] (uint32_t i) { return rows[i]; }
template <typename T> const Vector4<T>& Matrix4<T>::operator[] (uint32_t i) const { return rows[i]; }

template <typename T> Matrix4<T> Matrix4<T>::operator * (T t) const
{
	Matrix4<T> r;
	MUL4(t, this->elements[0], r.elements[0]);
	MUL4(t, this->elements[1], r.elements[1]);
	MUL4(t, this->elements[2], r.elements[2]);
	MUL4(t, this->elements[3], r.elements[3]);
	return r;
}

template <typename T> Matrix4<T>& Matrix4<T>::operator *= (T t)
{
	MUL4(t, this->elements[0], this->elements[0]);
	MUL4(t, this->elements[1], this->elements[1]);
	MUL4(t, this->elements[2], this->elements[2]);
	MUL4(t, this->elements[3], this->elements[3]);
	return *this;
}

template <typename T> Matrix4<T> Matrix4<T>::operator + (const Matrix4<T>& m) const
{
	Matrix4<T> r;
	ADD4(this->elements[0], m.elements[0], r.elements[0]);
	ADD4(this->elements[1], m.elements[1], r.elements[1]);
	ADD4(this->elements[2], m.elements[2], r.elements[2]);
	ADD4(this->elements[3], m.elements[3], r.elements[3]);
	return r;
}

template <typename T> Matrix4<T> Matrix4<T>::operator - (const Matrix4<T>& m) const
{
	Matrix4<T> r;
	SUB4(this->elements[0], m.elements[0], r.elements[0]);
	SUB4(this->elements[1], m.elements[1], r.elements[1]);
	SUB4(this->elements[2], m.elements[2], r.elements[2]);
	SUB4(this->elements[3], m.elements[3], r.elements[3]);
	return r;
}

template <typename T> Matrix4<T>& Matrix4<T>::operator += (const Matrix4<T>& m)
{
	ADD4(this->elements[0], m.elements[0], this->elements[0]);
	ADD4(this->elements[1], m.elements[1], this->elements[1]);
	ADD4(this->elements[2], m.elements[2], this->elements[2]);
	ADD4(this->elements[3], m.elements[3], this->elements[3]);
	return *this;
}

template <typename T> Matrix4<T>& Matrix4<T>::operator -= (const Matrix4<T>& m)
{
	SUB4(this->elements[0], m.elements[0], this->elements[0]);
	SUB4(this->elements[1], m.elements[1], this->elements[1]);
	SUB4(this->elements[2], m.elements[2], this->elements[2]);
	SUB4(this->elements[3], m.elements[3], this->elements[3]);
	return *this;
}

template <typename T> Matrix4<T> Matrix4<T>::operator * (const Matrix4<T>& m) const
{
	Matrix4<T> r;
	DOT4(r.rows, rows, m.rows, 0, 0);
	DOT4(r.rows, rows, m.rows, 0, 1);
	DOT4(r.rows, rows, m.rows, 0, 2);
	DOT4(r.rows, rows, m.rows, 0, 3);
	DOT4(r.rows, rows, m.rows, 1, 0);
	DOT4(r.rows, rows, m.rows, 1, 1);
	DOT4(r.rows, rows, m.rows, 1, 2);
	DOT4(r.rows, rows, m.rows, 1, 3);
	DOT4(r.rows, rows, m.rows, 2, 0);
	DOT4(r.rows, rows, m.rows, 2, 1);
	DOT4(r.rows, rows, m.rows, 2, 2);
	DOT4(r.rows, rows, m.rows, 2, 3);
	DOT4(r.rows, rows, m.rows, 3, 0);
	DOT4(r.rows, rows, m.rows, 3, 1);
	DOT4(r.rows, rows, m.rows, 3, 2);
	DOT4(r.rows, rows, m.rows, 3, 3);
	return r;
}

template <typename T> Matrix4<T>& Matrix4<T>::operator *= (const Matrix4<T>& m)
{
	Matrix4<T> c(*this);
	DOT4(rows, c.rows, m.rows, 0, 0);
	DOT4(rows, c.rows, m.rows, 0, 1);
	DOT4(rows, c.rows, m.rows, 0, 2);
	DOT4(rows, c.rows, m.rows, 0, 3);
	DOT4(rows, c.rows, m.rows, 1, 0);
	DOT4(rows, c.rows, m.rows, 1, 1);
	DOT4(rows, c.rows, m.rows, 1, 2);
	DOT4(rows, c.rows, m.rows, 1, 3);
	DOT4(rows, c.rows, m.rows, 2, 0);
	DOT4(rows, c.rows, m.rows, 2, 1);
	DOT4(rows, c.rows, m.rows, 2, 2);
	DOT4(rows, c.rows, m.rows, 2, 3);
	DOT4(rows, c.rows, m.rows, 3, 0);
	DOT4(rows, c.rows, m.rows, 3, 1);
	DOT4(rows, c.rows, m.rows, 3, 2);
	DOT4(rows, c.rows, m.rows, 3, 3);
	return *this;
}

template <typename T> Vector4<T> Matrix4<T>::operator * (const Vector4<T>& v) const
{
	return Vector4<T>(
		Vector::Dot<T>(v, rows[0]),
		Vector::Dot<T>(v, rows[1]),
		Vector::Dot<T>(v, rows[2]),
		Vector::Dot<T>(v, rows[3])
	);
}

// ------------------------------------ Matrix functions ------------------------------------------

template <typename T> Matrix2<T> Matrix::Inverse(const Matrix2<T>& m)
{
	Matrix2<T> r;
	T det = static_cast<T>(1) / DET2(m[0][0], m[0][1], m[1][0], m[1][1]);
	
	r[0][0] = m[1][1];
	r[1][1] = m[0][0];
	r[0][1] = -m[0][1];
	r[1][0] = -m[1][0];

	return (r * det);
}

template <typename T> Matrix3<T> Matrix::Inverse(const Matrix3<T>& m)
{
	Matrix3<T> r;
	T det = static_cast<T>(1) / DET3(m[0][0], m[0][1], m[0][2], m[1][0], m[1][1], m[1][2], m[2][0], m[2][1], m[2][2]);
	
	r[0][0] = +DET2(m[1][1], m[1][2], m[2][1], m[2][2]);
	r[1][0] = -DET2(m[1][0], m[1][2], m[2][0], m[2][2]);
	r[2][0] = +DET2(m[1][0], m[1][1], m[2][0], m[2][1]);
	r[0][1] = -DET2(m[0][1], m[0][2], m[2][1], m[2][2]);
	r[1][1] = +DET2(m[0][0], m[0][2], m[2][0], m[2][2]);
	r[2][1] = -DET2(m[0][0], m[0][1], m[2][0], m[2][1]);
	r[0][2] = +DET2(m[0][1], m[0][2], m[1][1], m[1][2]);
	r[1][2] = -DET2(m[0][0], m[0][2], m[1][0], m[1][2]);
	r[2][2] = +DET2(m[0][0], m[0][1], m[1][0], m[1][1]);

	return (r * det);
}

template <typename T> Matrix4<T> Matrix::Inverse(const Matrix4<T>& m)
{
	Matrix4F r;

	float m0 = m[0][0] * DET3(m[1][1], m[1][2], m[1][3], m[2][1], m[2][2], m[2][3], m[3][1], m[3][2], m[3][3]);
	float m1 = m[0][1] * DET3(m[1][0], m[1][2], m[1][3], m[2][0], m[2][2], m[2][3], m[3][0], m[3][2], m[3][3]);
	float m2 = m[0][2] * DET3(m[1][0], m[1][1], m[1][3], m[2][0], m[2][1], m[2][3], m[3][0], m[3][1], m[3][3]);
	float m3 = m[0][3] * DET3(m[1][0], m[1][1], m[1][2], m[2][0], m[2][1], m[2][2], m[3][0], m[3][1], m[3][2]);
	T det = static_cast<T>(1) / (m0 - m1 + m2 - m3);

	r[0][0] = +DET3(m[1][1], m[1][2], m[1][3], m[2][1], m[2][2], m[2][3], m[3][1], m[3][2], m[3][3]);
	r[1][0] = -DET3(m[1][0], m[1][2], m[1][3], m[2][0], m[2][2], m[2][3], m[3][0], m[3][2], m[3][3]);
	r[2][0] = +DET3(m[1][0], m[1][1], m[1][3], m[2][0], m[2][1], m[2][3], m[3][0], m[3][1], m[3][3]);
	r[3][0] = -DET3(m[1][0], m[1][1], m[1][2], m[2][0], m[2][1], m[2][2], m[3][0], m[3][1], m[3][2]);

	r[0][1] = -DET3(m[0][1], m[0][2], m[0][3], m[2][1], m[2][2], m[2][3], m[3][1], m[3][2], m[3][3]);
	r[1][1] = +DET3(m[0][0], m[0][2], m[0][3], m[2][0], m[2][2], m[2][3], m[3][0], m[3][2], m[3][3]);
	r[2][1] = -DET3(m[0][0], m[0][1], m[0][3], m[2][0], m[2][1], m[2][3], m[3][0], m[3][1], m[3][3]);
	r[3][1] = +DET3(m[0][0], m[0][1], m[0][2], m[2][0], m[2][1], m[2][2], m[3][0], m[3][1], m[3][2]);

	r[0][2] = +DET3(m[0][1], m[0][2], m[0][3], m[1][1], m[1][2], m[1][3], m[3][1], m[3][2], m[3][3]);
	r[1][2] = -DET3(m[0][0], m[0][2], m[0][3], m[1][0], m[1][2], m[1][3], m[3][0], m[3][2], m[3][3]);
	r[2][2] = +DET3(m[0][0], m[0][1], m[0][3], m[1][0], m[1][1], m[1][3], m[3][0], m[3][1], m[3][3]);
	r[3][2] = -DET3(m[0][0], m[0][1], m[0][2], m[1][0], m[1][1], m[1][2], m[3][0], m[3][1], m[3][2]);

	r[0][3] = -DET3(m[0][1], m[0][2], m[0][3], m[1][1], m[1][2], m[1][3], m[2][1], m[2][2], m[2][3]);
	r[1][3] = +DET3(m[0][0], m[0][2], m[0][3], m[1][0], m[1][2], m[1][3], m[2][0], m[2][2], m[2][3]);
	r[2][3] = -DET3(m[0][0], m[0][1], m[0][3], m[1][0], m[1][1], m[1][3], m[2][0], m[2][1], m[2][3]);
	r[3][3] = +DET3(m[0][0], m[0][1], m[0][2], m[1][0], m[1][1], m[1][2], m[2][0], m[2][1], m[2][2]);

	return (r * det);
}

template <typename T> Matrix2<T> Matrix::Transpose(const Matrix2<T>& m)
{
	return Matrix2<T> {
		{ m[0][0], m[1][0] },
		{ m[0][1], m[1][1] }
	};
}

template <typename T> Matrix3<T> Matrix::Transpose(const Matrix3<T>& m)
{
	return Matrix3<T> {
		{ m[0][0], m[1][0], m[2][0] },
		{ m[0][1], m[1][1], m[2][1] },
		{ m[0][2], m[1][2], m[2][2] }
	};
}

template <typename T> Matrix4<T> Matrix::Transpose(const Matrix4<T>& m)
{
	return Matrix4<T> {
		{ m[0][0], m[1][0], m[2][0], m[3][0] },
		{ m[0][1], m[1][1], m[2][1], m[3][1] },
		{ m[0][2], m[1][2], m[2][2], m[3][2] },
		{ m[0][3], m[1][3], m[2][3], m[3][3] }
	};
}

template <typename T> Matrix4<T> Matrix::Translate(const Vector3<T>& v)
{
	Matrix4F m(1.0f);
	m[3] = Vector4F(v, 1.0f);
	return m;
}

template <typename T> Matrix4<T> Matrix::Scale(const Vector3<T>& v)
{
	Matrix4F m(1.0f);
	m[0][0] = v.x;
	m[1][1] = v.y;
	m[2][2] = v.z;
	return m;
}

template <typename T> Matrix4<T> Matrix::Rotate(const Vector3<T>& v)
{
	return Matrix4F(Quaternion(v));
}

template <typename T> std::wstring MatrixToString(T* pElements, size_t nElements)
{
	wchar_t buffer[2048] = {};

	wchar_t* pBuffer = buffer;
	size_t szBuffer = _countof(buffer);

	for (uint32_t i = 0; i < nElements; i++)
	{
		WriteToBuffer(pBuffer, szBuffer, L"[");
		for (uint32_t j = 0; j < nElements; j++)
		{
			WriteToBuffer(pBuffer, szBuffer, *(pElements + (i * nElements) + j));
			if (j != (nElements - 1))
			{
				WriteToBuffer(pBuffer, szBuffer, L", ");
			}
		}
		WriteToBuffer(pBuffer, szBuffer, L"]\n");
	}

	*pBuffer = 0; // null-terminate

	return std::wstring(buffer);
}

template <typename T> std::wstring Matrix::ToString(const Matrix2<T>& m) { return MatrixToString(&m.elements[0][0], 2); }
template <typename T> std::wstring Matrix::ToString(const Matrix3<T>& m) { return MatrixToString(&m.elements[0][0], 3); }
template <typename T> std::wstring Matrix::ToString(const Matrix4<T>& m) { return MatrixToString(&m.elements[0][0], 4); }

// ------------------------- Matrix template/function instantiations ------------------------------

#define INSTANTIATE_MATRIX_TEMPLATES_FOR_FLOATING_POINT_TYPE(X)							\
    template struct Matrix2<X>;                                                         \
    template struct Matrix3<X>;                                                         \
    template struct Matrix4<X>;                                                         \
	template Matrix2<X> Matrix::Inverse(const Matrix2<X>& m);							\
	template Matrix3<X> Matrix::Inverse(const Matrix3<X>& m);							\
	template Matrix4<X> Matrix::Inverse(const Matrix4<X>& m);							\
	template Matrix2<X> Matrix::Transpose(const Matrix2<X>& m);							\
	template Matrix3<X> Matrix::Transpose(const Matrix3<X>& m);							\
	template Matrix4<X> Matrix::Transpose(const Matrix4<X>& m);							\
	template std::wstring Matrix::ToString(const Matrix2<X>& m);						\
	template std::wstring Matrix::ToString(const Matrix3<X>& m);						\
	template std::wstring Matrix::ToString(const Matrix4<X>& m);						\

INSTANTIATE_MATRIX_TEMPLATES_FOR_FLOATING_POINT_TYPE(float)
