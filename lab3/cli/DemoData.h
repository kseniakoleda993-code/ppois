#pragma once

#include "studis/StudentInformationSystem.h"

namespace studis {

/// Создаёт систему с направлениями, дисциплинами, аудиториями, группой, одним преподавателем и одним студентом.
StudentInformationSystem makeDemoSystem(const Date& today);

}  // namespace studis
