#include "Utility.h"

#include "Camera.h"

#include <iostream>
#include <string>
#include <math.h>
#include <algorithm>
#include "Editor.h"
#include "RessourceManager.h"

namespace Utility
{
	int screenX;
	int screenY;

	GLFWwindow* window = NULL;
	float time = 0;
	float lastTime = 0;

	SoLoud::Soloud* soloud = nullptr;

	bool isAzerty = true;

	int FPSvalues[FPS_NB_VALUES];
	int FPSvaluePos;

	glm::vec4 overlayColor = glm::vec4(0);
	float overlayZ = 100;

	int testCount = 0; 
	
	float corruptionAmount = 0;

	float gameSoundsVolume = 1;
	float musicVolume = 1;
	float globalVolume = 1;
	float globalVolumeOverride = 1;

	float GetDeltaTime()
	{
		return time - lastTime;
	}

	glm::vec3 Utility::Project(glm::vec2 a, glm::vec2 b, glm::vec2 c)
	{
		glm::vec2 AB = b - a;
		glm::vec2 AC = c - a;

		float AH = ((AB.x * AC.x) + (AB.y * AC.y)) / length(AB);
		glm::vec2 H = a + (glm::normalize(AB) * AH);

		return glm::vec3(H.x, H.y, AH);
	}

	glm::vec2 Rotate(glm::vec2 vec, float beta)
	{
		double rad = beta * 3.14159265 / 180.0;
		return glm::vec2(
			(cos(rad) * vec.x) - (sin(rad) * vec.y),
			(sin(rad) * vec.x) + (cos(rad) * vec.y)
		);
	}

	float AngleBetween(glm::vec2 a, glm::vec2 b)
	{
		return acos(glm::dot(a, b) / glm::length(a) / glm::length(b)) * Utility::RadToDeg;
	}

	glm::vec2 GetMousePos()
	{
		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);
		return glm::vec2((float)mouseX, (float)mouseY);
	}

	glm::vec2 ScreenToWorld(glm::vec2 screenPos)
	{
		screenPos.y *= -1;
		screenPos.y += (float)screenY / 2;
		screenPos.x -= (float)screenX / 2;

		screenPos /= screenY;

		glm::vec2 res = Camera::position + (screenPos * Camera::size);
		return res;
	}

	glm::vec3 WorldToScreen(glm::vec3 worldPos)
	{
		vec2 delta = vec2(worldPos) - Camera::position;
		delta /= Camera::size;
		delta *= screenY;
		delta += vec2(screenX / 2, screenY / 2);

		return vec3(delta.x, delta.y, worldPos.z);
	}

	float GetVectorAngle(glm::vec2 vector)
	{
		float angleRight = AngleBetween(glm::vec2(1, 0), vector);
		float angleDown = AngleBetween(glm::vec2(0, -1), vector);

		if (angleDown < 90)
			return 360 - angleRight;
		else
			return angleRight;
	}

	glm::vec2 LineItersection(float a1, float b1, float a2, float b2)
	{
		if (a1 == a2) throw "The lines are not crossing. Please check that a1 is not equal to a2 before calling the function";

		if (std::isnan(a1))
		{
			return LineItersectionVertical(a2, b2, b1);
		}
		if (std::isnan(a2))
		{
			return LineItersectionVertical(a1, b1, b2);
		}

		float sx = (b2 - b1) / (a1 - a2);
		float sy = (a1 * sx) + b1;

		return glm::vec2(sx, sy);
	}

	glm::vec2 LineItersectionVertical(float a1, float b1, float b2)
	{
		float sy = (a1 * b2) + b1;
		float sx = b2;

		return glm::vec2(sx, sy);
	}

	bool SegementIntersection(glm::vec2 A1, glm::vec2 B1, glm::vec2 A2, glm::vec2 B2, glm::vec2* res)
	{
		float a1, b1;
		GetLineEquationFromPoints(A1, B1, &a1, &b1);

		float a2, b2;
		GetLineEquationFromPoints(A2, B2, &a2, &b2);

		// Parallel segemnts
		if (a1 == a2 || (std::isnan(a1) && std::isnan(a2)))
		{
			return false;
		}

		glm::vec2 intersection = LineItersection(a1, b1, a2, b2);

		bool inFirst = false;
		bool inSecond = false;

		// Check if the intesection point is inside first segement
		if (A1.x == B1.x) // Vectical lines
		{
			if ((A1.y < B1.y && intersection.y > A1.y && intersection.y < B1.y)
				|| (A1.y > B1.y && intersection.y < A1.y && intersection.y > B1.y))
			{
				inFirst = true;
			}
		}
		else // Other lines
		{
			if ((A1.x < B1.x && intersection.x > A1.x && intersection.x < B1.x)
				|| (A1.x > B1.x && intersection.x < A1.x && intersection.x > B1.x))
			{
				inFirst = true;
			}
		}

		// Check if the intesection point is inside second segement
		if (A2.x == B2.x) // Vectical lines
		{
			if ((A2.y < B2.y && intersection.y > A2.y && intersection.y < B2.y)
				|| (A2.y > B2.y && intersection.y < A2.y && intersection.y > B2.y))
			{
				inSecond = true;
			}
		}
		else // Other lines
		{
			if ((A2.x < B2.x && intersection.x > A2.x && intersection.x < B2.x)
				|| (A2.x > B2.x && intersection.x < A2.x && intersection.x > B2.x))
			{
				inSecond = true;
			}
		}

		if (inFirst && inSecond)
		{
			*res = intersection;
			return true;
		}
		else
		{
			*res = intersection; // Set the result anyway, can be useful
			return false;
		}
	}

	void GetLineEquationFromPoints(glm::vec2 p1, glm::vec2 p2, float* a, float* b)
	{
		if (p1.x == p2.x) // Vertical
		{
			*a = NAN;
			*b = p1.x;
		}
		else // Not vertical
		{
			*a = (p2.y - p1.y) / (p2.x - p1.x);
			*b = p1.y - ((*a) * p1.x);
		}
	}

	float GetTimeSine(int period)
	{
		return (float)cos(Utility::time * Utility::PI / (period / 1000.f)) * 0.5f + 0.5f; return 0.0f;
	}

	int GetFPS()
	{
		int sum = 0;

		for (int i = 0; i < FPS_NB_VALUES; i++)
		{
			sum += FPSvalues[i];
		}

		return sum / FPS_NB_VALUES;
	}

	std::string GetKeyDesc(int glfwKey)
	{
		if (glfwKey == GLFW_KEY_DELETE) return "Del";

		if (isAzerty)
		{
			if (glfwKey == GLFW_KEY_A)
				return "Q";
			if (glfwKey == GLFW_KEY_Q)
				return "A";
			if (glfwKey == GLFW_KEY_W)
				return "Z";
			if (glfwKey == GLFW_KEY_Z)
				return "W";
		}
			
		return std::string(1, glfwKey);
	}

	std::string ToLower(std::string value)
	{
		std::string res = value;
		std::transform(value.begin(), value.end(), res.begin(),
			[](unsigned char c) { return std::tolower(c); });

		return res;
	}

	std::string ToUpper(std::string value)
	{
		std::string res = value;
		std::transform(value.begin(), value.end(), res.begin(),
			[](unsigned char c) { return std::toupper(c); });

		return res;
	}

	glm::vec2 Abs(glm::vec2 vector)
	{
		return glm::vec2(
			abs(vector.x),
			abs(vector.y)
		);
	}

	glm::vec3 Abs(glm::vec3 vector)
	{
		return glm::vec3(
			abs(vector.x),
			abs(vector.y),
			abs(vector.z)
		);
	}

	unsigned int currentVAO = 0;
	void GlBindVtexArrayOptimised(unsigned int VAO)
	{
		if (currentVAO == VAO) return;

		currentVAO = VAO;
		glBindVertexArray(VAO);
	}

	void GetLevelAABB(glm::vec2* resMin, glm::vec2* resMax, bool inEditor)
	{
		auto list = inEditor ? &Editor::editorObjects : &EditorSaveManager::levelObjectList;

		(*list->begin())->GetAABB(resMin, resMax);

		for (auto it = list->begin(); it != list->end(); it++)
		{
			vec2 min; vec2 max;
			(*it)->GetAABB(&min, &max);

			if (min.x < resMin->x)
				resMin->x = min.x;
			if (max.x > resMax->x)
				resMax->x = max.x;
			if (min.y < resMin->y)
				resMin->y = min.y;
			if (max.y > resMax->y)
				resMax->y = max.y;
		}
	}

	glm::vec2 RoundVector(glm::vec2 v)
	{
		return glm::vec2(std::round(v.x), std::round(v.y));
	}

	glm::vec3 RoundVector(glm::vec3 v)
	{
		return glm::vec3(std::round(v.x), std::round(v.y), std::round(v.z));
	}

	float SqrLength(glm::vec2 vec)
	{
		return vec.x * vec.x + vec.y * vec.y;
	}

	float SqrDist(glm::vec2 a, glm::vec2 b)
	{
		return SqrLength(a - b);
	}

	SoLoud::handle PlaySound(std::string path, float volume)
	{
		SoLoud::Wav* sound = RessourceManager::GetSound(path);
		SoLoud::handle handle = soloud->play(*sound, volume);
		return handle;
	}

	unsigned int lastBoundTexture = -1;
	int lastTextureAttachement = -1;
	void BindTexture2D(unsigned int texture, int attachement)
	{
		if (lastBoundTexture == texture && lastTextureAttachement == attachement) return;

		lastBoundTexture = texture;
		lastTextureAttachement = attachement;

		glActiveTexture(GL_TEXTURE0 + attachement);
		glBindTexture(GL_TEXTURE_2D, texture);
	}
}
