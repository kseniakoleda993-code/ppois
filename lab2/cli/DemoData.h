#pragma once

#include "autoshop/AutoShop.h"

namespace autoshop {

/// Создаёт мастерскую с каталогом услуг, запчастями, одним механиком и одним клиентом с автомобилем.
AutoShop makeDemoShop(const Date& today);

}  // namespace autoshop
