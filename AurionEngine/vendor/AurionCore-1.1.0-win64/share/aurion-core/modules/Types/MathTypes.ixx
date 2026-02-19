module;

#include <AurionExport.h>

export module Aurion.Types:Math;

import :Primitives;

export namespace Aurion
{

	/* This is a helper macro for generating each math type with operator overloads */
#define DEFINE_TYPE(NAME, TYPE, SIZE, ...)								\
	struct AURION_API NAME												\
	{																	\
		union															\
		{																\
			struct { TYPE __VA_ARGS__; };								\
			TYPE data[SIZE];											\
		};																\
																		\
		/* Access */													\
		TYPE& operator[](size_t i);										\
		const TYPE& operator[](size_t i) const;							\
																		\
		/* Unary */														\
		NAME operator-() const;											\
																		\
		/* Compound assignments */										\
		NAME& operator+=(const NAME& rhs);								\
		NAME& operator-=(const NAME& rhs);								\
		NAME& operator*=(TYPE scalar);									\
		NAME& operator/=(TYPE scalar);									\
	};																	\
																		\
	/* Binary operators */												\
	AURION_API NAME operator+(const NAME& lhs, const NAME& rhs);		\
	AURION_API NAME operator-(const NAME& lhs, const NAME& rhs);		\
	AURION_API NAME operator*(const NAME& val, TYPE scalar);			\
	AURION_API NAME operator*(TYPE scalar, const NAME& vec);			\
	AURION_API NAME operator/(const NAME& val, TYPE scalar);			\
																		\
	/* Comparisons */													\
	AURION_API bool operator==(const NAME& lhs, const NAME& rhs);		\
	AURION_API bool operator!=(const NAME& lhs, const NAME& rhs);		\
	AURION_API bool operator<(const NAME& lhs, const NAME& rhs);		\
	AURION_API bool operator<=(const NAME& lhs, const NAME& rhs);		\
	AURION_API bool operator>(const NAME& lhs, const NAME& rhs);		\
	AURION_API bool operator>=(const NAME& lhs, const NAME& rhs);

	// ----------------------------
	// ------- Vector Types -------
	// ----------------------------

	DEFINE_TYPE(Vec2f32, f32, 2, x, y);
	DEFINE_TYPE(Vec3f32, f32, 3, x, y, z);
	DEFINE_TYPE(Vec4f32, f32, 4, x, y, z, w);
	DEFINE_TYPE(Vec2f64, f64, 2, x, y);
	DEFINE_TYPE(Vec3f64, f64, 3, x, y, z);
	DEFINE_TYPE(Vec4f64, f64, 4, x, y, z, w);

	// ----------------------------
	// ------- Matrix Types -------
	// ----------------------------

	DEFINE_TYPE(Mat2f32, f32, 4,
		m11, m12,
		m21, m22
	);
	DEFINE_TYPE(Mat3f32, f32, 9,
		m11, m12, m13,
		m21, m22, m23,
		m31, m32, m33
	);
	DEFINE_TYPE(Mat4f32, f32, 16,
		m11, m12, m13, m14,
		m21, m22, m23, m24,
		m31, m32, m33, m34,
		m41, m42, m43, m44
	);
	DEFINE_TYPE(Mat2f64, f64, 4,
		m11, m12,
		m21, m22
	);
	DEFINE_TYPE(Mat3f64, f64, 9,
		m11, m12, m13,
		m21, m22, m23,
		m31, m32, m33
	);
	DEFINE_TYPE(Mat4f64, f64, 16,
		m11, m12, m13, m14,
		m21, m22, m23, m24,
		m31, m32, m33, m34,
		m41, m42, m43, m44
	);

	// ----------------------------
	// ----- Quaternion Types -----
	// ----------------------------

	DEFINE_TYPE(Quatf32, f32, 4, x, y, z, w);
	DEFINE_TYPE(Quatf64, f64, 4, x, y, z, w);

	// ----------------------------
	// ------- Type Aliases -------
	// ----------------------------

	typedef AURION_API Vec2f32 fVec2;
	typedef AURION_API Vec3f32 fVec3;
	typedef AURION_API Vec4f32 fVec4;

	typedef AURION_API Mat2f32 fMat2;
	typedef AURION_API Mat3f32 fMat3;
	typedef AURION_API Mat4f32 fMat4;

	typedef AURION_API Quatf32 fQuat;

	typedef AURION_API Vec2f64 dVec2;
	typedef AURION_API Vec3f64 dVec3;
	typedef AURION_API Vec4f64 dVec4;

	typedef AURION_API Mat2f64 dMat2;
	typedef AURION_API Mat3f64 dMat3;
	typedef AURION_API Mat4f64 dMat4;

	typedef AURION_API Quatf64 dQuat;
}