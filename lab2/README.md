# Лабораторная работа №2. Автомастерская

Крупный ООП-проект на C++17: объектная модель автомастерской (клиенты, автомобили, записи на обслуживание,
заказ-наряды, склад запчастей, счета и оплата, зарплата, отчётность).

> Этот файл создан скриптом `tools/gen_readme.py` по заголовочным файлам. После изменения кода запустите
> `python3 tools/gen_readme.py`, чтобы таблицы остались точными.

## Сборка, запуск, тесты

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/lab2_app                          # консольное меню
cd build && ctest --output-on-failure     # модульные тесты
```

Покрытие (нужен `gcovr`):

```bash
cmake -S . -B build-cov -DCMAKE_BUILD_TYPE=Debug -DLAB2_ENABLE_COVERAGE=ON
cmake --build build-cov --parallel && (cd build-cov && ctest)
gcovr -r . --filter 'include/' --filter 'cli/' --exclude 'cli/main.cpp' --print-summary
```

## Структура

| Путь | Назначение |
| --- | --- |
| `include/autoshop/` | Предметная область: классы и исключения (по тематическим заголовкам) |
| `cli/` | Консольное меню, не зависящее от внутренней логики классов |
| `tests/` | Модульные тесты (GoogleTest) |
| `tools/gen_readme.py` | Генератор этого README |

Заголовки по темам: `Exceptions.h` (исключения), `Common.h` (деньги, даты, время, адрес, контакты),
`Services.h` (услуги, скидки, промокоды), `Vehicle.h` (автомобиль и его узлы), `Parts.h` (запчасти и склад),
`People.h` (клиенты и сотрудники), `Operations.h` (записи, заказ-наряды, контроль качества, жалобы),
`Billing.h` (счета, оплата, зарплата, отчёты, смены, лояльность, уведомления), `AutoShop.h` (фасад).

## Принципы проектирования

- **Инкапсуляция:** все поля приватные, доступ через методы; инварианты проверяются в конструкторах.
- **Деньги в копейках** (`Money`), чтобы не накапливать ошибку округления; разные валюты не складываются.
- **Связи по идентификаторам:** заказ-наряд хранит `clientId`/`vin`, а не копию клиента. Сами объекты передаются в
  конструкторы и методы как параметры (это и есть ассоциации), а владеет ими фасад `AutoShop`.
- **Свои исключения** наследуются от `AutoShopException`; бизнес-ошибки не маскируются кодами возврата.
- **Состояния** (заказ-наряд, запись, заказ поставщику, жалоба) меняются только допустимыми переходами.
- **Консоль отделена от логики:** меню получает потоки ввода/вывода и тестируется без реальной консоли.

## Итоговая статистика

| Показатель | Требование | Получено | Выполнено |
| --- | --- | --- | --- |
| Классы предметной области | не менее 50 | 55 | да |
| Поля | не менее 150 | 230 | да |
| Уникальные поведения | не менее 100 | 241 | да |
| Ассоциации | не менее 30 | 159 | да |
| Собственные классы исключений | не менее 12 | 16 | да |

«Поведения» — открытые методы с содержательной логикой (геттеры не считаются). «Ассоциация» — случай, когда другой класс предметной области используется как поле или как параметр метода/конструктора.

## Классы

Формат: `Класс | поля | методы | поведения | связанные классы`.

| Класс | Поля | Методы | Поведения | Ассоциации (связанные классы) |
| --- | --- | --- | --- | --- |
| Accountant | 2 | 4 | 2 | Date, Employee, ExpenseRecord, Invoice, Money |
| Address | 3 | 6 | 3 | — |
| Appointment | 6 | 12 | 6 | Client, ServiceType, TimeSlot, Vehicle |
| AppointmentBook | 2 | 10 | 8 | Appointment, Date, TimeSlot |
| AutoShop | 16 | 35 | 22 | Address, AppointmentBook, Client, Date, ExpenseRecord, Invoice, LoyaltyCard, Mechanic, Money, NotificationService, Part, PartsInventory, Person, ServiceType, TimeSlot, Vehicle, WorkOrder, WorkshopBay |
| Cashier | 2 | 6 | 4 | Employee, Money |
| Certificate | 3 | 5 | 2 | Date |
| Client | 4 | 9 | 5 | Date, Money, Person, Vehicle |
| Complaint | 8 | 11 | 4 | Client, Date, Manager, WorkOrder |
| ContactInfo | 2 | 6 | 4 | — |
| CustomerReview | 4 | 6 | 2 | Client, WorkOrder |
| Date | 3 | 9 | 6 | — |
| DiagnosticResult | 4 | 6 | 2 | Money |
| Discount | 4 | 8 | 4 | Date, Money |
| Employee | 4 | 7 | 3 | Date, Money, Person |
| Engine | 3 | 6 | 3 | — |
| Estimate | 7 | 13 | 6 | Client, Date, Money, OrderLine, ServiceType |
| ExpenseRecord | 4 | 6 | 2 | Date, Money |
| FinancialReport | 4 | 10 | 6 | Date, ExpenseRecord, Invoice, Money |
| InspectionReport | 4 | 8 | 4 | Date, DiagnosticResult, Mechanic, Vehicle |
| Insurance | 4 | 7 | 3 | Date, Money |
| Invoice | 7 | 13 | 6 | Date, Discount, Money, WorkOrder |
| LoyaltyCard | 3 | 7 | 4 | Client, Date, Money |
| Manager | 3 | 7 | 4 | Discount, Employee, Money |
| Mechanic | 4 | 8 | 6 | Certificate, Date, Employee, Money, ServiceType, TimeSlot |
| Money | 2 | 11 | 9 | — |
| Notification | 4 | 5 | 1 | Date |
| NotificationService | 1 | 5 | 4 | Appointment, Client, Date, Invoice, Notification, WorkOrder |
| OrderLine | 2 | 4 | 2 | Part |
| Part | 5 | 7 | 2 | Money, Vehicle |
| PartsInventory | 2 | 9 | 8 | OrderLine, Part, StockRecord |
| Payment | 5 | 8 | 3 | Date, Invoice, Money |
| PayrollRecord | 5 | 7 | 2 | Employee, Money |
| Person | 4 | 8 | 4 | Address, ContactInfo, Date |
| PromoCode | 4 | 6 | 3 | Date, Discount |
| PurchaseOrder | 5 | 11 | 6 | Date, OrderLine, Supplier |
| QualityCheck | 4 | 8 | 4 | Date, Mechanic, WorkOrder |
| Receipt | 5 | 6 | 1 | Cashier, Date, Money, Payment |
| Refund | 4 | 6 | 2 | Money, Payment |
| ServiceAdvisor | 2 | 5 | 3 | Client, Employee, Money, ServicePlan, Vehicle |
| ServicePlan | 3 | 6 | 3 | Vehicle |
| ServiceType | 5 | 8 | 3 | Money |
| Shift | 4 | 7 | 3 | Date, Employee |
| StockRecord | 4 | 8 | 4 | Part |
| Supplier | 4 | 7 | 3 | ContactInfo, Date |
| TimeSlot | 3 | 7 | 4 | Date |
| TireSet | 4 | 7 | 3 | — |
| Tool | 4 | 7 | 3 | Date |
| Vehicle | 9 | 17 | 8 | Date, Engine, Insurance, TireSet, Warranty |
| WarehouseKeeper | 2 | 5 | 3 | Employee, OrderLine, PartsInventory, PurchaseOrder |
| Warranty | 3 | 6 | 3 | Date |
| WarrantyClaim | 5 | 8 | 3 | Date, Vehicle, Warranty, WorkOrder |
| WorkOrder | 10 | 22 | 12 | Client, Date, Discount, Mechanic, Money, OrderLine, ServiceType, Vehicle |
| WorkSchedule | 2 | 7 | 5 | Date, Employee, Shift |
| WorkshopBay | 4 | 10 | 6 | Date, Tool, WorkOrder |

## Исключения

- `InvalidClientDataException` — Некорректные данные клиента. (наследует `AutoShopException`)
- `InvalidVehicleDataException` — Некорректные данные автомобиля. (наследует `AutoShopException`)
- `InvalidMoneyOperationException` — Недопустимая операция с деньгами (разные валюты, переплата, неизвестный способ оплаты). (наследует `AutoShopException`)
- `InsufficientFundsException` — Недостаточно средств (или баллов, или лимита). (наследует `AutoShopException`)
- `OutOfStockException` — Запчасти нет в нужном количестве на складе. (наследует `AutoShopException`)
- `InvalidPartDataException` — Некорректные данные запчасти, поставщика или позиции заказа. (наследует `AutoShopException`)
- `InvalidServiceDataException` — Некорректные данные услуги. (наследует `AutoShopException`)
- `MechanicNotAvailableException` — Механик занят или не имеет нужной квалификации. (наследует `AutoShopException`)
- `BayNotAvailableException` — Нет свободного рабочего поста (бокса). (наследует `AutoShopException`)
- `AppointmentConflictException` — Конфликт записи на обслуживание или смены. (наследует `AutoShopException`)
- `InvalidOrderStateException` — Операция недопустима в текущем состоянии заказа. (наследует `AutoShopException`)
- `InvalidDateException` — Некорректная дата или временной интервал. (наследует `AutoShopException`)
- `PromoCodeException` — Промокод недействителен или исчерпан. (наследует `AutoShopException`)
- `WarrantyExpiredException` — Гарантия не действует. (наследует `AutoShopException`)
- `DiscountLimitExceededException` — Превышен лимит скидки. (наследует `AutoShopException`)
- `NotFoundException` — Запрошенный объект не найден. (наследует `AutoShopException`)

## Подробное описание классов

#### Accountant (наследует Employee)

Бухгалтер: считает зарплату и составляет отчёты. Файл: `include/autoshop/Billing.h`.

- Поля: `taxPercent_` (double), `reportsPrepared_` (int).
- Поведения: `calculatePayroll`, `compileReport`.
- Методы доступа: `getReportsPrepared`, `getTaxPercent`.
- Ассоциации: Date, Employee, ExpenseRecord, Invoice, Money.

#### Address

Почтовый адрес. Файл: `include/autoshop/Common.h`.

- Поля: `street_` (std::string), `city_` (std::string), `postalCode_` (std::string).
- Поведения: `fullAddress`, `hasValidPostalCode`, `isInCity`.
- Методы доступа: `getCity`, `getPostalCode`, `getStreet`.
- Ассоциации: —.

#### Appointment

Запись клиента на обслуживание. Файл: `include/autoshop/Operations.h`.

- Поля: `id_` (int), `clientId_` (int), `vin_` (std::string), `slot_` (TimeSlot), `service_` (ServiceType), `status_` (AppointmentStatus).
- Поведения: `cancel`, `complete`, `confirm`, `conflictsWith`, `isActive`, `reschedule`.
- Методы доступа: `getClientId`, `getId`, `getService`, `getSlot`, `getStatus`, `getVin`.
- Ассоциации: Client, ServiceType, TimeSlot, Vehicle.

#### AppointmentBook

Журнал записей: следит, чтобы не было больше записей, чем постов. Файл: `include/autoshop/Operations.h`.

- Поля: `capacity_` (int), `appointments_` (std::vector<Appointment>).
- Поведения: `appointmentsOn`, `book`, `cancel`, `confirm`, `countActiveAt`, `findById`, `findFreeSlot`, `isSlotFree`.
- Методы доступа: `getAppointments`, `getCapacity`.
- Ассоциации: Appointment, Date, TimeSlot.

#### AutoShop

Автомастерская: объединяет клиентов, автомобили, сотрудников, склад, записи, заказы и счета. Файл: `include/autoshop/AutoShop.h`.

- Поля: `name_` (std::string), `address_` (Address), `hourlyRate_` (Money), `nextId_` (int), `clients_` (std::vector<Client>), `vehicles_` (std::vector<Vehicle>), `mechanics_` (std::vector<Mechanic>), `orders_` (std::vector<WorkOrder>), `invoices_` (std::vector<Invoice>), `expenses_` (std::vector<ExpenseRecord>), `catalog_` (std::vector<ServiceType>), `loyaltyCards_` (std::vector<LoyaltyCard>), `bays_` (std::vector<WorkshopBay>), `appointments_` (AppointmentBook), `inventory_` (PartsInventory), `notifier_` (NotificationService).
- Поведения: `addPart`, `addPartToOrder`, `addServiceToOrder`, `addServiceType`, `bookAppointment`, `buildReport`, `completeWorkOrder`, `findClient`, `findInvoice`, `findMechanic`, `findOrder`, `findService`, `findVehicle`, `hasVehicle`, `hireMechanic`, `issueInvoice`, `openWorkOrder`, `payInvoice`, `recordExpense`, `registerClient`, `registerVehicle`, `startWorkOrder`.
- Методы доступа: `getAddress`, `getAppointments`, `getBays`, `getCatalog`, `getClients`, `getHourlyRate`, `getInventory`, `getInvoices`, `getMechanics`, `getName`, `getNotifications`, `getOrders`, `getVehicles`.
- Ассоциации: Address, AppointmentBook, Client, Date, ExpenseRecord, Invoice, LoyaltyCard, Mechanic, Money, NotificationService, Part, PartsInventory, Person, ServiceType, TimeSlot, Vehicle, WorkOrder, WorkshopBay.

#### Cashier (наследует Employee)

Кассир: принимает оплату и работает с кассой. Файл: `include/autoshop/People.h`.

- Поля: `cashDrawer_` (Money), `receiptsIssued_` (int).
- Поведения: `acceptCash`, `giveChange`, `registerReceipt`, `withdraw`.
- Методы доступа: `getCashDrawer`, `getReceiptsIssued`.
- Ассоциации: Employee, Money.

#### Certificate

Квалификационный сертификат механика. Файл: `include/autoshop/People.h`.

- Поля: `name_` (std::string), `issuedOn_` (Date), `validUntil_` (Date).
- Поведения: `daysLeft`, `isValidOn`.
- Методы доступа: `getIssuedOn`, `getName`, `getValidUntil`.
- Ассоциации: Date.

#### Client (наследует Person)

Клиент автомастерской. Файл: `include/autoshop/People.h`.

- Поля: `id_` (int), `registeredOn_` (Date), `vehicleVins_` (std::vector<std::string>), `totalSpent_` (Money).
- Поведения: `isRegularCustomer`, `ownsVehicle`, `recordSpending`, `registerVehicle`, `releaseVehicle`.
- Методы доступа: `getId`, `getRegisteredOn`, `getTotalSpent`, `getVehicleVins`.
- Ассоциации: Date, Money, Person, Vehicle.

#### Complaint

Жалоба клиента. Файл: `include/autoshop/Operations.h`.

- Поля: `id_` (int), `clientId_` (int), `workOrderId_` (int), `text_` (std::string), `status_` (ComplaintStatus), `openedOn_` (Date), `resolvedById_` (int), `resolution_` (std::string).
- Поведения: `ageInDaysOn`, `escalate`, `isOverdueOn`, `resolve`.
- Методы доступа: `getClientId`, `getId`, `getOpenedOn`, `getResolvedById`, `getStatus`, `getText`, `getWorkOrderId`.
- Ассоциации: Client, Date, Manager, WorkOrder.

#### ContactInfo

Контактные данные: телефон и электронная почта. Файл: `include/autoshop/Common.h`.

- Поля: `phone_` (std::string), `email_` (std::string).
- Поведения: `hasValidEmail`, `hasValidPhone`, `maskedPhone`, `phoneDigits`.
- Методы доступа: `getEmail`, `getPhone`.
- Ассоциации: —.

#### CustomerReview

Отзыв клиента о выполненном заказе. Файл: `include/autoshop/Operations.h`.

- Поля: `clientId_` (int), `workOrderId_` (int), `rating_` (int), `comment_` (std::string).
- Поведения: `isPositive`, `summary`.
- Методы доступа: `getClientId`, `getComment`, `getRating`, `getWorkOrderId`.
- Ассоциации: Client, WorkOrder.

#### Date

Календарная дата (григорианский календарь). Файл: `include/autoshop/Common.h`.

- Поля: `year_` (int), `month_` (int), `day_` (int).
- Поведения: `daysInMonth`, `daysUntil`, `isBefore`, `isLeapYear`, `plusDays`, `toString`.
- Методы доступа: `getDay`, `getMonth`, `getYear`.
- Ассоциации: —.

#### DiagnosticResult

Результат диагностики одного узла автомобиля. Файл: `include/autoshop/Vehicle.h`.

- Поля: `code_` (std::string), `description_` (std::string), `severity_` (int), `estimatedCost_` (Money).
- Поведения: `isCritical`, `priority`.
- Методы доступа: `getCode`, `getDescription`, `getEstimatedCost`, `getSeverity`.
- Ассоциации: Money.

#### Discount

Скидка с ограниченным сроком действия. Файл: `include/autoshop/Services.h`.

- Поля: `percent_` (double), `reason_` (std::string), `validFrom_` (Date), `validTo_` (Date).
- Поведения: `applyTo`, `extendTo`, `isValidOn`, `savedAmount`.
- Методы доступа: `getPercent`, `getReason`, `getValidFrom`, `getValidTo`.
- Ассоциации: Date, Money.

#### Employee (наследует Person)

Сотрудник автомастерской. Файл: `include/autoshop/People.h`.

- Поля: `employeeId_` (int), `position_` (std::string), `hireDate_` (Date), `monthlySalary_` (Money).
- Поведения: `isEligibleForBonusOn`, `raiseSalary`, `yearsOfServiceOn`.
- Методы доступа: `getEmployeeId`, `getHireDate`, `getMonthlySalary`, `getPosition`.
- Ассоциации: Date, Money, Person.

#### Engine

Двигатель автомобиля. Файл: `include/autoshop/Vehicle.h`.

- Поля: `type_` (std::string), `volumeLiters_` (double), `powerHp_` (int).
- Поведения: `isDiesel`, `isHighPerformance`, `oilChangeIntervalKm`.
- Методы доступа: `getPowerHp`, `getType`, `getVolumeLiters`.
- Ассоциации: —.

#### Estimate

Предварительная смета на ремонт. Файл: `include/autoshop/Operations.h`.

- Поля: `id_` (int), `clientId_` (int), `services_` (std::vector<ServiceType>), `parts_` (std::vector<OrderLine>), `validUntil_` (Date), `markupPercent_` (double), `approved_` (bool).
- Поведения: `addPart`, `addService`, `approve`, `isExpiredOn`, `subtotal`, `total`.
- Методы доступа: `getClientId`, `getId`, `getMarkupPercent`, `getParts`, `getServices`, `getValidUntil`, `isApproved`.
- Ассоциации: Client, Date, Money, OrderLine, ServiceType.

#### ExpenseRecord

Расход мастерской. Файл: `include/autoshop/Billing.h`.

- Поля: `category_` (std::string), `amount_` (Money), `date_` (Date), `description_` (std::string).
- Поведения: `isInMonth`, `isLarge`.
- Методы доступа: `getAmount`, `getCategory`, `getDate`, `getDescription`.
- Ассоциации: Date, Money.

#### FinancialReport

Финансовый отчёт за период. Файл: `include/autoshop/Billing.h`.

- Поля: `periodStart_` (Date), `periodEnd_` (Date), `revenue_` (Money), `expenses_` (Money).
- Поведения: `addExpense`, `addInvoice`, `includes`, `isProfitable`, `profit`, `profitMarginPercent`.
- Методы доступа: `getExpenses`, `getPeriodEnd`, `getPeriodStart`, `getRevenue`.
- Ассоциации: Date, ExpenseRecord, Invoice, Money.

#### InspectionReport

Отчёт о диагностике автомобиля. Файл: `include/autoshop/Operations.h`.

- Поля: `vin_` (std::string), `mechanicId_` (int), `date_` (Date), `results_` (std::vector<DiagnosticResult>).
- Поведения: `addResult`, `hasCriticalIssues`, `totalEstimatedCost`, `worstSeverity`.
- Методы доступа: `getDate`, `getMechanicId`, `getResults`, `getVin`.
- Ассоциации: Date, DiagnosticResult, Mechanic, Vehicle.

#### Insurance

Страховой полис. Файл: `include/autoshop/Vehicle.h`.

- Поля: `policyNumber_` (std::string), `provider_` (std::string), `validUntil_` (Date), `coverageLimit_` (Money).
- Поведения: `covers`, `extend`, `isValidOn`.
- Методы доступа: `getCoverageLimit`, `getPolicyNumber`, `getProvider`, `getValidUntil`.
- Ассоциации: Date, Money.

#### Invoice

Счёт на оплату выполненного заказа. Файл: `include/autoshop/Billing.h`.

- Поля: `id_` (int), `workOrderId_` (int), `clientId_` (int), `issuedOn_` (Date), `dueOn_` (Date), `amount_` (Money), `paid_` (Money).
- Поведения: `applyDiscount`, `daysOverdue`, `isOverdueOn`, `isPaid`, `outstanding`, `registerPayment`.
- Методы доступа: `getAmount`, `getClientId`, `getDueOn`, `getId`, `getIssuedOn`, `getPaid`, `getWorkOrderId`.
- Ассоциации: Date, Discount, Money, WorkOrder.

#### LoyaltyCard

Карта лояльности клиента. Файл: `include/autoshop/Billing.h`.

- Поля: `clientId_` (int), `points_` (int), `issuedOn_` (Date).
- Поведения: `discountPercent`, `earn`, `redeem`, `tier`.
- Методы доступа: `getClientId`, `getIssuedOn`, `getPoints`.
- Ассоциации: Client, Date, Money.

#### Manager (наследует Employee)

Руководитель мастерской: утверждает скидки и возвраты. Файл: `include/autoshop/People.h`.

- Поля: `discountLimitPercent_` (double), `refundLimit_` (Money), `approvedCount_` (int).
- Поведения: `approveDiscount`, `authorizeRefund`, `canApprove`, `raiseLimit`.
- Методы доступа: `getApprovedCount`, `getDiscountLimitPercent`, `getRefundLimit`.
- Ассоциации: Discount, Employee, Money.

#### Mechanic (наследует Employee)

Механик, выполняющий ремонт. Файл: `include/autoshop/People.h`.

- Поля: `specialization_` (std::string), `hourlyRate_` (Money), `certificates_` (std::vector<Certificate>), `busySlots_` (std::vector<TimeSlot>).
- Поведения: `addCertificate`, `canHandle`, `hasValidCertificate`, `isAvailable`, `laborCost`, `reserve`.
- Методы доступа: `getHourlyRate`, `getSpecialization`.
- Ассоциации: Certificate, Date, Employee, Money, ServiceType, TimeSlot.

#### Money

Денежная сумма, хранится в копейках, чтобы избежать ошибок округления. Файл: `include/autoshop/Common.h`.

- Поля: `cents_` (long long), `currency_` (std::string).
- Поведения: `applyPercent`, `fromUnits`, `isGreaterOrEqual`, `isNegative`, `isZero`, `minus`, `multiply`, `plus`, `toString`.
- Методы доступа: `getCents`, `getCurrency`.
- Ассоциации: —.

#### Notification

Уведомление клиенту. Файл: `include/autoshop/Billing.h`.

- Поля: `recipientId_` (int), `message_` (std::string), `channel_` (std::string), `createdOn_` (Date).
- Поведения: `isEmail`.
- Методы доступа: `getChannel`, `getCreatedOn`, `getMessage`, `getRecipientId`.
- Ассоциации: Date.

#### NotificationService

Служба уведомлений (сообщения накапливаются в журнале отправленных). Файл: `include/autoshop/Billing.h`.

- Поля: `sent_` (std::vector<Notification>).
- Поведения: `countFor`, `notifyAppointment`, `notifyInvoice`, `notifyWorkReady`.
- Методы доступа: `getSent`.
- Ассоциации: Appointment, Client, Date, Invoice, Notification, WorkOrder.

#### OrderLine

Строка заказа: запчасть и количество. Файл: `include/autoshop/Parts.h`.

- Поля: `part_` (Part), `quantity_` (int).
- Поведения: `increase`, `lineTotal`.
- Методы доступа: `getPart`, `getQuantity`.
- Ассоциации: Part.

#### Part

Запчасть или расходный материал. Файл: `include/autoshop/Parts.h`.

- Поля: `sku_` (std::string), `name_` (std::string), `category_` (std::string), `price_` (Money), `compatibleMakes_` (std::vector<std::string>).
- Поведения: `isCompatibleWith`, `priceWithMarkup`.
- Методы доступа: `getCategory`, `getCompatibleMakes`, `getName`, `getPrice`, `getSku`.
- Ассоциации: Money, Vehicle.

#### PartsInventory

Склад запчастей автомастерской. Файл: `include/autoshop/Parts.h`.

- Поля: `records_` (std::map<std::string, StockRecord>), `location_` (std::string).
- Поведения: `find`, `isAvailable`, `issue`, `lowStockSkus`, `quantityOf`, `receive`, `registerPart`, `totalValue`.
- Методы доступа: `getLocation`.
- Ассоциации: OrderLine, Part, StockRecord.

#### Payment

Платёж по счёту. Файл: `include/autoshop/Billing.h`.

- Поля: `id_` (int), `invoiceId_` (int), `amount_` (Money), `method_` (std::string), `paidOn_` (Date).
- Поведения: `isCash`, `netAmount`, `processingFee`.
- Методы доступа: `getAmount`, `getId`, `getInvoiceId`, `getMethod`, `getPaidOn`.
- Ассоциации: Date, Invoice, Money.

#### PayrollRecord

Расчётный листок сотрудника за период. Файл: `include/autoshop/Billing.h`.

- Поля: `employeeId_` (int), `period_` (std::string), `bonus_` (Money), `gross_` (Money), `tax_` (Money).
- Поведения: `effectiveTaxPercent`, `net`.
- Методы доступа: `getBonus`, `getEmployeeId`, `getGross`, `getPeriod`, `getTax`.
- Ассоциации: Employee, Money.

#### Person

Человек: общие данные клиентов и сотрудников. Файл: `include/autoshop/People.h`.

- Поля: `name_` (std::string), `birthDate_` (Date), `contact_` (ContactInfo), `address_` (Address).
- Поведения: `ageOn`, `isAdultOn`, `moveTo`, `updateContact`.
- Методы доступа: `getAddress`, `getBirthDate`, `getContact`, `getName`.
- Ассоциации: Address, ContactInfo, Date.

#### PromoCode

Промокод, дающий скидку ограниченное число раз. Файл: `include/autoshop/Services.h`.

- Поля: `code_` (std::string), `discount_` (Discount), `usageLimit_` (int), `timesUsed_` (int).
- Поведения: `canBeUsedOn`, `redeem`, `remainingUses`.
- Методы доступа: `getCode`, `getDiscount`, `getTimesUsed`.
- Ассоциации: Date, Discount.

#### PurchaseOrder

Заказ поставщику на пополнение склада. Файл: `include/autoshop/Parts.h`.

- Поля: `id_` (int), `supplier_` (Supplier), `lines_` (std::vector<OrderLine>), `orderedOn_` (Date), `status_` (PurchaseStatus).
- Поведения: `addLine`, `cancel`, `expectedDelivery`, `markReceived`, `send`, `total`.
- Методы доступа: `getId`, `getLines`, `getOrderedOn`, `getStatus`, `getSupplier`.
- Ассоциации: Date, OrderLine, Supplier.

#### QualityCheck

Контроль качества выполненных работ. Файл: `include/autoshop/Operations.h`.

- Поля: `workOrderId_` (int), `inspectorId_` (int), `checkedOn_` (Date), `items_` (std::vector<std::pair<std::string, bool>>).
- Поведения: `failedItems`, `hasPassed`, `passRatePercent`, `recordItem`.
- Методы доступа: `getCheckedOn`, `getInspectorId`, `getItems`, `getWorkOrderId`.
- Ассоциации: Date, Mechanic, WorkOrder.

#### Receipt

Кассовый чек. Файл: `include/autoshop/Billing.h`.

- Поля: `number_` (int), `paymentId_` (int), `total_` (Money), `cashierName_` (std::string), `issuedOn_` (Date).
- Поведения: `render`.
- Методы доступа: `getCashierName`, `getIssuedOn`, `getNumber`, `getPaymentId`, `getTotal`.
- Ассоциации: Cashier, Date, Money, Payment.

#### Refund

Возврат денег клиенту. Файл: `include/autoshop/Billing.h`.

- Поля: `id_` (int), `paymentId_` (int), `amount_` (Money), `reason_` (std::string).
- Поведения: `isFullRefundOf`, `refundedSharePercent`.
- Методы доступа: `getAmount`, `getId`, `getPaymentId`, `getReason`.
- Ассоциации: Money, Payment.

#### ServiceAdvisor (наследует Employee)

Приёмщик: общается с клиентами и рекомендует услуги. Файл: `include/autoshop/People.h`.

- Поля: `customersServed_` (int), `commissionPercent_` (double).
- Поведения: `calculateCommission`, `recommendServices`, `recordServedClient`.
- Методы доступа: `getCommissionPercent`, `getCustomersServed`.
- Ассоциации: Client, Employee, Money, ServicePlan, Vehicle.

#### ServicePlan

План регулярного обслуживания: через сколько километров нужна услуга. Файл: `include/autoshop/Vehicle.h`.

- Поля: `serviceName_` (std::string), `intervalKm_` (int), `lastServiceKm_` (int).
- Поведения: `isDue`, `kmUntilDue`, `recordService`.
- Методы доступа: `getIntervalKm`, `getLastServiceKm`, `getServiceName`.
- Ассоциации: Vehicle.

#### ServiceType

Вид услуги, которую оказывает автомастерская (замена масла, диагностика и т. д.). Файл: `include/autoshop/Services.h`.

- Поля: `code_` (std::string), `name_` (std::string), `basePrice_` (Money), `standardHours_` (double), `requiredSkill_` (std::string).
- Поведения: `isLongRunning`, `requiresSkill`, `totalPrice`.
- Методы доступа: `getBasePrice`, `getCode`, `getName`, `getRequiredSkill`, `getStandardHours`.
- Ассоциации: Money.

#### Shift

Рабочая смена сотрудника. Файл: `include/autoshop/Billing.h`.

- Поля: `employeeId_` (int), `date_` (Date), `startHour_` (int), `hours_` (int).
- Поведения: `endHour`, `isNightShift`, `overlaps`.
- Методы доступа: `getDate`, `getEmployeeId`, `getHours`, `getStartHour`.
- Ассоциации: Date, Employee.

#### StockRecord

Складская запись об одной запчасти. Файл: `include/autoshop/Parts.h`.

- Поля: `part_` (Part), `quantity_` (int), `minLevel_` (int), `shelf_` (std::string).
- Поведения: `add`, `isLow`, `remove`, `restockAmount`.
- Методы доступа: `getMinLevel`, `getPart`, `getQuantity`, `getShelf`.
- Ассоциации: Part.

#### Supplier

Поставщик запчастей. Файл: `include/autoshop/Parts.h`.

- Поля: `name_` (std::string), `contact_` (ContactInfo), `leadTimeDays_` (int), `rating_` (double).
- Поведения: `estimateDelivery`, `isReliable`, `rate`.
- Методы доступа: `getContact`, `getLeadTimeDays`, `getName`, `getRating`.
- Ассоциации: ContactInfo, Date.

#### TimeSlot

Временной интервал в пределах одного рабочего дня. Файл: `include/autoshop/Common.h`.

- Поля: `date_` (Date), `startHour_` (int), `durationHours_` (int).
- Поведения: `contains`, `endHour`, `isOn`, `overlaps`.
- Методы доступа: `getDate`, `getDurationHours`, `getStartHour`.
- Ассоциации: Date.

#### TireSet

Комплект шин. Файл: `include/autoshop/Vehicle.h`.

- Поля: `size_` (std::string), `season_` (std::string), `wearPercent_` (double), `productionYear_` (int).
- Поведения: `addWear`, `ageInYears`, `needsReplacement`.
- Методы доступа: `getProductionYear`, `getSeason`, `getSize`, `getWearPercent`.
- Ассоциации: —.

#### Tool

Инструмент, требующий периодической калибровки. Файл: `include/autoshop/Parts.h`.

- Поля: `name_` (std::string), `serialNumber_` (std::string), `lastCalibration_` (Date), `intervalDays_` (int).
- Поведения: `calibrate`, `needsCalibrationOn`, `nextCalibrationDate`.
- Методы доступа: `getIntervalDays`, `getLastCalibration`, `getName`, `getSerialNumber`.
- Ассоциации: Date.

#### Vehicle

Автомобиль клиента. Файл: `include/autoshop/Vehicle.h`.

- Поля: `vin_` (std::string), `plate_` (std::string), `make_` (std::string), `model_` (std::string), `year_` (int), `mileage_` (int), `ownerId_` (int), `engine_` (Engine), `tires_` (TireSet).
- Поведения: `ageInYears`, `assignOwnerId`, `driveKm`, `isInsuredOn`, `isUnderWarranty`, `needsTireReplacement`, `replaceTires`, `updateMileage`.
- Методы доступа: `getEngine`, `getMake`, `getMileage`, `getModel`, `getOwnerId`, `getPlate`, `getTires`, `getVin`, `getYear`.
- Ассоциации: Date, Engine, Insurance, TireSet, Warranty.

#### WarehouseKeeper (наследует Employee)

Кладовщик: принимает поставки и выдаёт запчасти. Файл: `include/autoshop/People.h`.

- Поля: `zone_` (std::string), `operationsCount_` (int).
- Поведения: `buildReorderList`, `issueParts`, `receiveDelivery`.
- Методы доступа: `getOperationsCount`, `getZone`.
- Ассоциации: Employee, OrderLine, PartsInventory, PurchaseOrder.

#### Warranty

Гарантия на автомобиль или выполненные работы. Файл: `include/autoshop/Vehicle.h`.

- Поля: `description_` (std::string), `coveredUntil_` (Date), `mileageLimitKm_` (int).
- Поведения: `isActive`, `prolong`, `remainingKm`.
- Методы доступа: `getCoveredUntil`, `getDescription`, `getMileageLimitKm`.
- Ассоциации: Date.

#### WarrantyClaim

Гарантийное обращение по выполненному заказу. Файл: `include/autoshop/Operations.h`.

- Поля: `id_` (int), `workOrderId_` (int), `warranty_` (Warranty), `reason_` (std::string), `status_` (ClaimStatus).
- Поведения: `approve`, `isResolved`, `reject`.
- Методы доступа: `getId`, `getReason`, `getStatus`, `getWarranty`, `getWorkOrderId`.
- Ассоциации: Date, Vehicle, Warranty, WorkOrder.

#### WorkOrder

Заказ-наряд: основной документ ремонта. Файл: `include/autoshop/Operations.h`.

- Поля: `id_` (int), `clientId_` (int), `vin_` (std::string), `mechanicId_` (int), `services_` (std::vector<ServiceType>), `parts_` (std::vector<OrderLine>), `status_` (WorkOrderStatus), `openedOn_` (Date), `laborHours_` (double), `discountPercent_` (double).
- Поведения: `addPart`, `addService`, `applyDiscount`, `assignMechanic`, `cancel`, `complete`, `laborTotal`, `partsTotal`, `plannedHours`, `servicesTotal`, `start`, `total`.
- Методы доступа: `getClientId`, `getDiscountPercent`, `getId`, `getLaborHours`, `getMechanicId`, `getOpenedOn`, `getParts`, `getServices`, `getStatus`, `getVin`.
- Ассоциации: Client, Date, Discount, Mechanic, Money, OrderLine, ServiceType, Vehicle.

#### WorkSchedule

График смен сотрудников. Файл: `include/autoshop/Billing.h`.

- Поля: `shifts_` (std::vector<Shift>), `maxTotalHours_` (int).
- Поведения: `addShift`, `canAddHours`, `hoursFor`, `isOnDuty`, `shiftsOn`.
- Методы доступа: `getMaxTotalHours`, `getShifts`.
- Ассоциации: Date, Employee, Shift.

#### WorkshopBay

Рабочий пост (подъёмник, яма) с набором инструментов. Файл: `include/autoshop/Operations.h`.

- Поля: `number_` (int), `type_` (std::string), `occupiedByOrderId_` (int), `tools_` (std::vector<Tool>).
- Поведения: `addTool`, `hasTool`, `isFree`, `occupy`, `release`, `toolsNeedingCalibration`.
- Методы доступа: `getNumber`, `getOccupiedByOrderId`, `getTools`, `getType`.
- Ассоциации: Date, Tool, WorkOrder.
