#pragma once

#include <array>

namespace EventManager
{
	std::array<void(*)(), 500> OnMainLoopSuscribed;
	std::array<void(*)(), 500> OnExitAppSuscribed;
};

