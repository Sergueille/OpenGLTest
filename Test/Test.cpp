#include "pch.h"
#include "CppUnitTest.h"

#include <math.h>  
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "../OpenGLTest/Utility.cpp"
#include "../OpenGLTest/Collider.h"
#include "../OpenGLTest/Collider.cpp"
#include "../OpenGLTest/RectCollider.cpp"
#include "../OpenGLTest/CircleCollider.cpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace glm;

namespace Test
{
	TEST_CLASS(UtilityTest)
	{
	public:
		TEST_METHOD(Project_1)
		{
			vec3 res = Utility::Project(vec2(0, 0), vec2(5, 0), vec2(3, 3));
			vec3 expect = vec3(3, 0, 3);
			Assert::AreEqual(expect.x, res.x);
			Assert::AreEqual(expect.y, res.y);
			Assert::AreEqual(expect.z, res.z);
		}

		TEST_METHOD(Project_2)
		{
			vec3 res = Utility::Project(vec2(-1, -1), vec2(3, 1), vec2(6, 0));
			Assert::AreEqual(5.0f, res.x);
			Assert::AreEqual(2.0f, res.y);
			Assert::AreEqual((float)std::sqrt(45), res.z);
		}

		TEST_METHOD(Project_3)
		{
			vec3 res = Utility::Project(vec2(0, 0), vec2(0, -2), vec2(1, 1));
			Assert::AreEqual(0.0f, res.x);
			Assert::AreEqual(1.0f, res.y);
			Assert::AreEqual(-1.0f, res.z);
		}
	};

	TEST_CLASS(CollisionTest)
	{
	public:
		TEST_METHOD(CircleCircle_1)
		{
			CircleCollider c1 = CircleCollider(vec2(0, 0), 1);
			CircleCollider c2 = CircleCollider(vec2(3, 0), 1);
			vec3 res = c1.CollideWith(&c2);
			Assert::AreEqual(0.f, res.x);
			Assert::AreEqual(0.f, res.y);
			Assert::AreEqual(0.f, res.z);
			Assert::AreEqual(to_string(res), to_string(c2.CollideWith(&c1)));
		}

		TEST_METHOD(CircleCircle_2)
		{
			CircleCollider c1 = CircleCollider(vec2(0, 0), 1);
			CircleCollider c2 = CircleCollider(vec2(0, 0.5), 1);
			vec3 res = c1.CollideWith(&c2);
			Assert::AreEqual(0.f, res.x);
			Assert::AreEqual(-1.f, res.y);
			Assert::AreEqual(0.5f, res.z);
		}

		TEST_METHOD(CircleRect_1)
		{
			RectCollider c1 = RectCollider(vec2(0, 0), vec2(2, 2), 0);
			CircleCollider c2 = CircleCollider(vec2(3, 3), 1);
			vec3 res = c2.CollideWith(&c1);
			Assert::AreEqual(0.f, res.x);
			Assert::AreEqual(0.f, res.y);
			Assert::AreEqual(0.f, res.z);
		}

		TEST_METHOD(CircleRect_2)
		{
			RectCollider c1 = RectCollider(vec2(0, 0), vec2(2, 2), 45);
			CircleCollider c2 = CircleCollider(vec2(3, 3), 1);
			vec3 res = c2.CollideWith(&c1);
			Assert::AreEqual(0.f, res.x);
			Assert::AreEqual(0.f, res.y);
			Assert::AreEqual(0.f, res.z);
		}

		TEST_METHOD(CircleRect_3)
		{
			RectCollider c1 = RectCollider(vec2(0, 0), vec2(2, 2), 0);
			CircleCollider c2 = CircleCollider(vec2(1.5, 0), 1);
			vec3 res = c2.CollideWith(&c1);
			Assert::AreEqual(1.f, res.x);
			Assert::AreEqual(0.f, res.y);
			Assert::AreEqual(0.f, res.z);
		}

		TEST_METHOD(CircleRect_4)
		{
			RectCollider c1 = RectCollider(vec2(0, 0), vec2(4, 4), 0);
			CircleCollider c2 = CircleCollider(vec2(0, 1), 1);
			vec3 res = c2.CollideWith(&c1);
			Assert::AreEqual(0.f, res.x);
			Assert::AreEqual(1.f, res.y);
			Assert::AreEqual(0.5f, res.z);
		}
	};
}
