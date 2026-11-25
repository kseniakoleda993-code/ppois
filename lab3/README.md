# Лабораторная работа №3. Информационная система студентов

Крупный ООП-проект на C++17 на новой предметной области (в лабораторной №2 был домен «Автомастерская»):
объектная модель университетской информационной системы — приём, студенты и группы, дисциплины и расписание,
экзамены и зачётные книжки, посещаемость, дипломы, оплата обучения, стипендии, общежитие, библиотека.

> Этот файл создан скриптом `tools/gen_readme.py` по заголовочным файлам. После изменения кода запустите
> `python3 tools/gen_readme.py`, чтобы таблицы остались точными.

## Сборка, запуск, тесты

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/lab3_app                          # консольное меню
cd build && ctest --output-on-failure     # модульные тесты
```

Покрытие (нужен `gcovr`):

```bash
cmake -S . -B build-cov -DCMAKE_BUILD_TYPE=Debug -DLAB3_ENABLE_COVERAGE=ON
cmake --build build-cov --parallel && (cd build-cov && ctest)
gcovr -r . --filter 'include/' --filter 'cli/' --exclude 'cli/main.cpp' --print-summary
```

## Структура

| Путь | Назначение |
| --- | --- |
| `include/studis/` | Предметная область: классы и исключения (по тематическим заголовкам) |
| `cli/` | Консольное меню, не зависящее от внутренней логики классов |
| `tests/` | Модульные тесты (GoogleTest) |
| `tools/gen_readme.py` | Генератор этого README |

## Где что находится (по заголовкам)

| Файл | Классы | Кол-во | Поля | Поведения | Ассоциации |
| --- | --- | --- | --- | --- | --- |
| `include/studis/Admission.h` | Applicant, AdmissionCampaign, Application, Enrollment, AcademicLeave, TransferRequest, AdministrativeOrder | 7 | 36 | 30 | 19 |
| `include/studis/Assessment.h` | Exam, ExamResult, AttendanceRecord, AttendanceJournal, Retake, Thesis, Diploma | 7 | 35 | 28 | 26 |
| `include/studis/Common.h` | Money, Date, LessonTime, Address, ContactInfo | 5 | 13 | 26 | 0 |
| `include/studis/Courses.h` | Course, CoursePrerequisite, CourseOffering, Lesson, Timetable, Assignment, Submission | 7 | 35 | 27 | 18 |
| `include/studis/Finance.h` | TuitionContract, TuitionPayment, Scholarship, ScholarshipAward, DormitoryRoom, Dormitory, Book, BookLoan, LibraryCard | 9 | 43 | 40 | 27 |
| `include/studis/Grades.h` | Grade, RecordEntry, RecordBook | 3 | 8 | 12 | 4 |
| `include/studis/Life.h` | Certificate, StudentCard, Internship | 3 | 17 | 13 | 8 |
| `include/studis/People.h` | Person, Student, Teacher, Professor, Dean, Methodist | 6 | 23 | 32 | 14 |
| `include/studis/Structure.h` | Building, Department, Faculty, StudyProgram, Semester, Group, Classroom, CurriculumEntry, Curriculum | 9 | 38 | 36 | 14 |
| `include/studis/StudentInformationSystem.h` | StudentInformationSystem | 1 | 15 | 26 | 18 |
| `include/studis/Exceptions.h` | исключения (см. ниже) | 20 | — | — | — |

## Итоговая статистика

| Показатель | Требование | Получено | Выполнено |
| --- | --- | --- | --- |
| Классы предметной области | не менее 50 | 57 | да |
| Поля | не менее 150 | 263 | да |
| Уникальные поведения | не менее 100 | 270 | да |
| Ассоциации | не менее 30 | 148 | да |
| Собственные классы исключений | не менее 12 | 20 | да |

«Поведения» — открытые методы с содержательной логикой (геттеры не считаются). «Ассоциация» — случай, когда другой класс предметной области используется как поле или как параметр метода/конструктора.

## Классы

Формат: `Класс | поля | методы | поведения | связанные классы`.

| Класс | Поля | Методы | Поведения | Ассоциации (связанные классы) |
| --- | --- | --- | --- | --- |
| AcademicLeave | 5 | 10 | 5 | Date, Dean, Student |
| Address | 3 | 6 | 3 | — |
| AdministrativeOrder | 5 | 9 | 4 | Date, Dean |
| AdmissionCampaign | 6 | 11 | 5 | Applicant, Date, StudyProgram |
| Applicant | 3 | 7 | 4 | Person |
| Application | 6 | 10 | 4 | AdmissionCampaign, Applicant, Date, StudyProgram |
| Assignment | 5 | 8 | 3 | CourseOffering, Date |
| AttendanceJournal | 2 | 7 | 5 | AttendanceRecord, CourseOffering, Student |
| AttendanceRecord | 4 | 7 | 3 | Date, Lesson, Student |
| Book | 5 | 9 | 4 | — |
| BookLoan | 6 | 11 | 5 | Book, Date, Money, Student |
| Building | 3 | 6 | 3 | Address |
| Certificate | 5 | 9 | 4 | Date, Student |
| Classroom | 4 | 7 | 3 | Building, Group |
| ContactInfo | 2 | 6 | 4 | — |
| Course | 5 | 8 | 3 | Department |
| CourseOffering | 9 | 14 | 6 | Course, CoursePrerequisite, RecordBook, Semester, Student, Teacher |
| CoursePrerequisite | 3 | 4 | 1 | RecordBook |
| Curriculum | 4 | 8 | 6 | CurriculumEntry, StudyProgram |
| CurriculumEntry | 4 | 6 | 2 | — |
| Date | 3 | 9 | 6 | — |
| Dean | 2 | 5 | 3 | Student, Teacher |
| Department | 4 | 7 | 4 | Teacher |
| Diploma | 7 | 10 | 3 | Date, RecordBook, Student, StudyProgram, Thesis |
| Dormitory | 4 | 10 | 6 | Address, DormitoryRoom, Money, Student |
| DormitoryRoom | 3 | 8 | 5 | Student |
| Enrollment | 4 | 8 | 4 | CourseOffering, Date, Student |
| Exam | 7 | 10 | 5 | Classroom, CourseOffering, Date, Student |
| ExamResult | 4 | 7 | 3 | Course, Date, Exam, Grade, Student |
| Faculty | 3 | 6 | 3 | Dean, Department |
| Grade | 1 | 4 | 3 | — |
| Group | 6 | 13 | 7 | Date, Student, StudyProgram, Teacher |
| Internship | 7 | 12 | 5 | Date, Grade, Student |
| Lesson | 6 | 11 | 5 | Classroom, CourseOffering, LessonTime |
| LessonTime | 3 | 7 | 4 | — |
| LibraryCard | 3 | 8 | 5 | Book, BookLoan, Date, Money, Student |
| Methodist | 3 | 6 | 3 | Person, Student |
| Money | 2 | 11 | 9 | — |
| Person | 4 | 7 | 3 | Address, ContactInfo, Date |
| Professor | 3 | 9 | 6 | Money, Student, Teacher |
| RecordBook | 2 | 9 | 7 | RecordEntry, Student |
| RecordEntry | 5 | 7 | 2 | Date, Grade |
| Retake | 4 | 8 | 4 | Date, ExamResult |
| Scholarship | 4 | 7 | 3 | Money, RecordBook |
| ScholarshipAward | 7 | 10 | 4 | Date, Money, RecordBook, Scholarship, Student |
| Semester | 5 | 9 | 4 | Date |
| Student | 6 | 17 | 11 | Date, Person |
| StudentCard | 5 | 9 | 4 | Date, Money, Student |
| StudentInformationSystem | 15 | 39 | 26 | Address, Classroom, Course, CourseOffering, Date, Dormitory, DormitoryRoom, Group, LessonTime, Money, Person, RecordBook, Semester, Student, StudyProgram, Teacher, Timetable, TuitionContract |
| StudyProgram | 5 | 9 | 4 | Date |
| Submission | 5 | 8 | 3 | Assignment, Date, Student |
| Teacher | 5 | 11 | 6 | Date, Person |
| Thesis | 7 | 12 | 5 | Date, Grade, Professor, Student |
| Timetable | 2 | 8 | 6 | Lesson, Semester |
| TransferRequest | 7 | 11 | 4 | Dean, Student, StudyProgram |
| TuitionContract | 6 | 11 | 5 | Money, Student, StudyProgram |
| TuitionPayment | 5 | 8 | 3 | Date, Money, TuitionContract |

## Исключения

- `StudentSystemException` — Базовое исключение информационной системы студентов. (наследует `std::runtime_error`)
- `InvalidStudentDataException` — Некорректные данные студента или сотрудника. (наследует `StudentSystemException`)
- `InvalidGradeException` — Оценка вне допустимого диапазона. (наследует `StudentSystemException`)
- `InvalidCourseDataException` — Некорректные данные дисциплины или учебного плана. (наследует `StudentSystemException`)
- `CourseFullException` — В группе дисциплины нет свободных мест. (наследует `StudentSystemException`)
- `PrerequisiteNotMetException` — Не выполнены пререквизиты дисциплины. (наследует `StudentSystemException`)
- `ScheduleConflictException` — Конфликт расписания или перегрузка преподавателя. (наследует `StudentSystemException`)
- `InvalidDateException` — Некорректная дата или время. (наследует `StudentSystemException`)
- `InsufficientCreditsException` — Недостаточно зачётных единиц. (наследует `StudentSystemException`)
- `DuplicateRecordException` — Повторная запись (зачисление, отметка, заселение). (наследует `StudentSystemException`)
- `TuitionPaymentException` — Некорректный платёж за обучение. (наследует `StudentSystemException`)
- `InvalidMoneyOperationException` — Недопустимая денежная операция. (наследует `StudentSystemException`)
- `ScholarshipEligibilityException` — Студент не имеет права на стипендию. (наследует `StudentSystemException`)
- `InvalidExamStateException` — Операция недопустима в текущем состоянии экзамена или работы. (наследует `StudentSystemException`)
- `AcademicProbationException` — Действие запрещено из-за академической задолженности. (наследует `StudentSystemException`)
- `RoomCapacityException` — Недостаточно мест в аудитории, группе или комнате. (наследует `StudentSystemException`)
- `DocumentException` — Документ недействителен или не принадлежит студенту. (наследует `StudentSystemException`)
- `LibraryException` — Операция с библиотекой невозможна. (наследует `StudentSystemException`)
- `AdmissionException` — Поступление невозможно. (наследует `StudentSystemException`)
- `NotFoundException` — Запрошенный объект не найден. (наследует `StudentSystemException`)

## Подробное описание классов

#### AcademicLeave

Академический отпуск. Файл: `include/studis/Admission.h`.

- Поля: `studentId_` (int), `startDate_` (Date), `endDate_` (Date), `reason_` (std::string), `approvedById_` (int).
- Поведения: `approve`, `extend`, `isActiveOn`, `isApproved`, `lengthInDays`.
- Методы доступа: `getApprovedById`, `getEndDate`, `getReason`, `getStartDate`, `getStudentId`.
- Ассоциации: Date, Dean, Student.

#### Address

Почтовый адрес. Файл: `include/studis/Common.h`.

- Поля: `street_` (std::string), `city_` (std::string), `postalCode_` (std::string).
- Поведения: `fullAddress`, `hasValidPostalCode`, `isInCity`.
- Методы доступа: `getCity`, `getPostalCode`, `getStreet`.
- Ассоциации: —.

#### AdministrativeOrder

Приказ по студентам. Файл: `include/studis/Admission.h`.

- Поля: `number_` (int), `type_` (std::string), `issuedOn_` (Date), `text_` (std::string), `signedById_` (int).
- Поведения: `describe`, `isRecentOn`, `isSigned`, `sign`.
- Методы доступа: `getIssuedOn`, `getNumber`, `getSignedById`, `getText`, `getType`.
- Ассоциации: Date, Dean.

#### AdmissionCampaign

Приёмная кампания на направление. Файл: `include/studis/Admission.h`.

- Поля: `year_` (int), `programCode_` (std::string), `quota_` (int), `minScore_` (int), `deadline_` (Date), `acceptedIds_` (std::vector<int>).
- Поведения: `accept`, `fillPercent`, `isAccepted`, `isOpenOn`, `remainingPlaces`.
- Методы доступа: `getAcceptedIds`, `getDeadline`, `getMinScore`, `getProgramCode`, `getQuota`, `getYear`.
- Ассоциации: Applicant, Date, StudyProgram.

#### Applicant (наследует Person)

Абитуриент. Файл: `include/studis/Admission.h`.

- Поля: `id_` (int), `examScores_` (std::vector<int>), `schoolAverage_` (double).
- Поведения: `averageScore`, `bestScore`, `meetsMinimum`, `totalScore`.
- Методы доступа: `getExamScores`, `getId`, `getSchoolAverage`.
- Ассоциации: Person.

#### Application

Заявление абитуриента на направление. Файл: `include/studis/Admission.h`.

- Поля: `id_` (int), `applicantId_` (int), `programCode_` (std::string), `priority_` (int), `submittedOn_` (Date), `status_` (ApplicationStatus).
- Поведения: `approve`, `evaluate`, `isHigherPriorityThan`, `reject`.
- Методы доступа: `getApplicantId`, `getId`, `getPriority`, `getProgramCode`, `getStatus`, `getSubmittedOn`.
- Ассоциации: AdmissionCampaign, Applicant, Date, StudyProgram.

#### Assignment

Задание по дисциплине. Файл: `include/studis/Courses.h`.

- Поля: `id_` (int), `offeringId_` (int), `title_` (std::string), `dueDate_` (Date), `maxScore_` (int).
- Поведения: `acceptsSubmissionOn`, `isOverdueOn`, `latePenaltyPercent`.
- Методы доступа: `getDueDate`, `getId`, `getMaxScore`, `getOfferingId`, `getTitle`.
- Ассоциации: CourseOffering, Date.

#### AttendanceJournal

Журнал посещаемости реализации дисциплины. Файл: `include/studis/Assessment.h`.

- Поля: `offeringId_` (int), `records_` (std::vector<AttendanceRecord>).
- Поведения: `absencesOf`, `attendancePercent`, `isAllowedToExam`, `mark`, `studentsBelow`.
- Методы доступа: `getOfferingId`, `getRecords`.
- Ассоциации: AttendanceRecord, CourseOffering, Student.

#### AttendanceRecord

Отметка о посещении занятия. Файл: `include/studis/Assessment.h`.

- Поля: `lessonId_` (int), `studentId_` (int), `status_` (AttendanceStatus), `date_` (Date).
- Поведения: `countsAsPresent`, `isExcused`, `weight`.
- Методы доступа: `getDate`, `getLessonId`, `getStatus`, `getStudentId`.
- Ассоциации: Date, Lesson, Student.

#### Book

Книга библиотеки. Файл: `include/studis/Finance.h`.

- Поля: `isbn_` (std::string), `title_` (std::string), `author_` (std::string), `totalCopies_` (int), `availableCopies_` (int).
- Поведения: `borrowedCount`, `giveBack`, `isAvailable`, `lend`.
- Методы доступа: `getAuthor`, `getAvailableCopies`, `getIsbn`, `getTitle`, `getTotalCopies`.
- Ассоциации: —.

#### BookLoan

Выдача книги студенту. Файл: `include/studis/Finance.h`.

- Поля: `isbn_` (std::string), `studentId_` (int), `borrowedOn_` (Date), `dueOn_` (Date), `returned_` (bool), `extensions_` (int).
- Поведения: `daysOverdue`, `extend`, `fineOn`, `isOverdueOn`, `markReturned`.
- Методы доступа: `getBorrowedOn`, `getDueOn`, `getExtensions`, `getIsbn`, `getStudentId`, `isReturned`.
- Ассоциации: Book, Date, Money, Student.

#### Building

Учебный корпус. Файл: `include/studis/Structure.h`.

- Поля: `name_` (std::string), `address_` (Address), `floors_` (int).
- Поведения: `describe`, `hasFloor`, `isInCity`.
- Методы доступа: `getAddress`, `getFloors`, `getName`.
- Ассоциации: Address.

#### Certificate

Справка для студента (с места учёбы и т. п.). Файл: `include/studis/Life.h`.

- Поля: `number_` (int), `studentId_` (int), `type_` (std::string), `issuedOn_` (Date), `validUntil_` (Date).
- Поведения: `daysLeft`, `extend`, `isValidOn`, `verify`.
- Методы доступа: `getIssuedOn`, `getNumber`, `getStudentId`, `getType`, `getValidUntil`.
- Ассоциации: Date, Student.

#### Classroom

Аудитория. Файл: `include/studis/Structure.h`.

- Поля: `buildingName_` (std::string), `number_` (std::string), `capacity_` (int), `hasProjector_` (bool).
- Поведения: `canFit`, `isSuitableFor`, `label`.
- Методы доступа: `getBuildingName`, `getCapacity`, `getNumber`, `hasProjector`.
- Ассоциации: Building, Group.

#### ContactInfo

Контактные данные: телефон и электронная почта. Файл: `include/studis/Common.h`.

- Поля: `phone_` (std::string), `email_` (std::string).
- Поведения: `hasValidEmail`, `hasValidPhone`, `maskedPhone`, `phoneDigits`.
- Методы доступа: `getEmail`, `getPhone`.
- Ассоциации: —.

#### Course

Учебная дисциплина. Файл: `include/studis/Courses.h`.

- Поля: `code_` (std::string), `title_` (std::string), `credits_` (int), `hours_` (int), `departmentName_` (std::string).
- Поведения: `belongsTo`, `hoursPerCredit`, `isIntensive`.
- Методы доступа: `getCode`, `getCredits`, `getDepartmentName`, `getHours`, `getTitle`.
- Ассоциации: Department.

#### CourseOffering

Реализация дисциплины в семестре: преподаватель, места и записавшиеся студенты. Файл: `include/studis/Courses.h`.

- Поля: `id_` (int), `courseCode_` (std::string), `credits_` (int), `semesterYear_` (int), `term_` (Term), `teacherId_` (int), `capacity_` (int), `enrolledIds_` (std::vector<int>), `prerequisites_` (std::vector<CoursePrerequisite>).
- Поведения: `addPrerequisite`, `drop`, `enroll`, `isEnrolled`, `isFull`, `seatsLeft`.
- Методы доступа: `getCapacity`, `getCourseCode`, `getCredits`, `getEnrolledIds`, `getId`, `getSemesterYear`, `getTeacherId`, `getTerm`.
- Ассоциации: Course, CoursePrerequisite, RecordBook, Semester, Student, Teacher.

#### CoursePrerequisite

Пререквизит: чтобы изучать дисциплину, нужно сдать другую не ниже заданного балла. Файл: `include/studis/Courses.h`.

- Поля: `courseCode_` (std::string), `requiredCourseCode_` (std::string), `minPoints_` (int).
- Поведения: `isSatisfiedBy`.
- Методы доступа: `getCourseCode`, `getMinPoints`, `getRequiredCourseCode`.
- Ассоциации: RecordBook.

#### Curriculum

Учебный план направления. Файл: `include/studis/Structure.h`.

- Поля: `programCode_` (std::string), `requiredCredits_` (int), `totalSemesters_` (int), `entries_` (std::vector<CurriculumEntry>).
- Поведения: `addEntry`, `coversProgram`, `creditsInSemester`, `hasCourse`, `mandatoryCourses`, `totalCredits`.
- Методы доступа: `getEntries`, `getProgramCode`.
- Ассоциации: CurriculumEntry, StudyProgram.

#### CurriculumEntry

Строка учебного плана: дисциплина в определённом семестре. Файл: `include/studis/Structure.h`.

- Поля: `courseCode_` (std::string), `semesterNumber_` (int), `credits_` (int), `mandatory_` (bool).
- Поведения: `isElective`, `isInSemester`.
- Методы доступа: `getCourseCode`, `getCredits`, `getSemesterNumber`, `isMandatory`.
- Ассоциации: —.

#### Date

Календарная дата (григорианский календарь). Файл: `include/studis/Common.h`.

- Поля: `year_` (int), `month_` (int), `day_` (int).
- Поведения: `daysInMonth`, `daysUntil`, `isBefore`, `isLeapYear`, `plusDays`, `toString`.
- Методы доступа: `getDay`, `getMonth`, `getYear`.
- Ассоциации: —.

#### Dean (наследует Teacher)

Декан факультета. Файл: `include/studis/People.h`.

- Поля: `facultyName_` (std::string), `signedDocuments_` (int).
- Поведения: `canApproveLeaveFor`, `reviewExpulsion`, `signDocument`.
- Методы доступа: `getFacultyName`, `getSignedDocuments`.
- Ассоциации: Student, Teacher.

#### Department

Кафедра. Файл: `include/studis/Structure.h`.

- Поля: `name_` (std::string), `facultyName_` (std::string), `headTeacherId_` (int), `teacherIds_` (std::vector<int>).
- Поведения: `addTeacher`, `hasTeacher`, `isHeadedBy`, `teacherCount`.
- Методы доступа: `getFacultyName`, `getHeadTeacherId`, `getName`.
- Ассоциации: Teacher.

#### Diploma

Диплом о высшем образовании. Файл: `include/studis/Assessment.h`.

- Поля: `studentId_` (int), `programCode_` (std::string), `issuedOn_` (Date), `gpa_` (double), `withHonors_` (bool), `thesisTitle_` (std::string), `number_` (std::string).
- Поведения: `describe`, `honorsLabel`, `isIssuedIn`.
- Методы доступа: `getGpa`, `getIssuedOn`, `getNumber`, `getProgramCode`, `getStudentId`, `getThesisTitle`, `isWithHonors`.
- Ассоциации: Date, RecordBook, Student, StudyProgram, Thesis.

#### Dormitory

Студенческое общежитие. Файл: `include/studis/Finance.h`.

- Поля: `name_` (std::string), `address_` (Address), `monthlyFee_` (Money), `rooms_` (std::vector<DormitoryRoom>).
- Поведения: `addRoom`, `evict`, `feeForMonths`, `occupancyPercent`, `settle`, `totalFreePlaces`.
- Методы доступа: `getAddress`, `getMonthlyFee`, `getName`, `getRooms`.
- Ассоциации: Address, DormitoryRoom, Money, Student.

#### DormitoryRoom

Комната общежития. Файл: `include/studis/Finance.h`.

- Поля: `number_` (int), `capacity_` (int), `residentIds_` (std::vector<int>).
- Поведения: `assign`, `freePlaces`, `hosts`, `isFull`, `vacate`.
- Методы доступа: `getCapacity`, `getNumber`, `getResidentIds`.
- Ассоциации: Student.

#### Enrollment

Запись студента на реализацию дисциплины. Файл: `include/studis/Admission.h`.

- Поля: `studentId_` (int), `offeringId_` (int), `enrolledOn_` (Date), `status_` (EnrollmentStatus).
- Поведения: `complete`, `daysEnrolled`, `drop`, `isActive`.
- Методы доступа: `getEnrolledOn`, `getOfferingId`, `getStatus`, `getStudentId`.
- Ассоциации: CourseOffering, Date, Student.

#### Exam

Экзамен или зачёт по реализации дисциплины. Файл: `include/studis/Assessment.h`.

- Поля: `id_` (int), `offeringId_` (int), `date_` (Date), `classroomLabel_` (std::string), `roomCapacity_` (int), `status_` (ExamStatus), `registeredIds_` (std::vector<int>).
- Поведения: `attendeeCount`, `finish`, `isRegistered`, `registerStudent`, `start`.
- Методы доступа: `getClassroomLabel`, `getDate`, `getId`, `getOfferingId`, `getStatus`.
- Ассоциации: Classroom, CourseOffering, Date, Student.

#### ExamResult

Результат студента на экзамене. Файл: `include/studis/Assessment.h`.

- Поля: `examId_` (int), `studentId_` (int), `grade_` (Grade), `takenOn_` (Date).
- Поведения: `isPassed`, `toRecordEntry`, `updateStanding`.
- Методы доступа: `getExamId`, `getGrade`, `getStudentId`, `getTakenOn`.
- Ассоциации: Course, Date, Exam, Grade, Student.

#### Faculty

Факультет. Файл: `include/studis/Structure.h`.

- Поля: `name_` (std::string), `deanId_` (int), `departments_` (std::vector<Department>).
- Поведения: `addDepartment`, `findDepartment`, `totalTeachers`.
- Методы доступа: `getDeanId`, `getDepartments`, `getName`.
- Ассоциации: Dean, Department.

#### Grade

Оценка по 100-балльной шкале. Файл: `include/studis/Grades.h`.

- Поля: `points_` (int).
- Поведения: `isPassing`, `letter`, `toFivePoint`.
- Методы доступа: `getPoints`.
- Ассоциации: —.

#### Group

Учебная группа. Файл: `include/studis/Structure.h`.

- Поля: `code_` (std::string), `programCode_` (std::string), `admissionYear_` (int), `maxSize_` (int), `studentIds_` (std::vector<int>), `curatorId_` (int).
- Поведения: `addStudent`, `assignCurator`, `courseNumberOn`, `hasStudent`, `isFull`, `removeStudent`, `size`.
- Методы доступа: `getAdmissionYear`, `getCode`, `getCuratorId`, `getMaxSize`, `getProgramCode`, `getStudentIds`.
- Ассоциации: Date, Student, StudyProgram, Teacher.

#### Internship

Производственная практика студента. Файл: `include/studis/Life.h`.

- Поля: `id_` (int), `studentId_` (int), `company_` (std::string), `startDate_` (Date), `endDate_` (Date), `supervisorName_` (std::string), `reportScore_` (int).
- Поведения: `hasReport`, `isActiveOn`, `isCompletedOn`, `lengthInDays`, `submitReport`.
- Методы доступа: `getCompany`, `getEndDate`, `getId`, `getReportScore`, `getStartDate`, `getStudentId`, `getSupervisorName`.
- Ассоциации: Date, Grade, Student.

#### Lesson

Учебное занятие (лекция, практика, лабораторная). Файл: `include/studis/Courses.h`.

- Поля: `id_` (int), `offeringId_` (int), `teacherId_` (int), `classroomLabel_` (std::string), `time_` (LessonTime), `kind_` (std::string).
- Поведения: `conflictsWith`, `durationHours`, `isLecture`, `isOnDay`, `reschedule`.
- Методы доступа: `getClassroomLabel`, `getId`, `getKind`, `getOfferingId`, `getTeacherId`, `getTime`.
- Ассоциации: Classroom, CourseOffering, LessonTime.

#### LessonTime

Время учебного занятия: день недели, час начала и длительность в часах. Файл: `include/studis/Common.h`.

- Поля: `day_` (Weekday), `startHour_` (int), `durationHours_` (int).
- Поведения: `contains`, `endHour`, `isMorning`, `overlaps`.
- Методы доступа: `getDay`, `getDurationHours`, `getStartHour`.
- Ассоциации: —.

#### LibraryCard

Читательский билет студента. Файл: `include/studis/Finance.h`.

- Поля: `studentId_` (int), `issuedOn_` (Date), `loans_` (std::vector<BookLoan>).
- Поведения: `activeLoans`, `borrow`, `giveBack`, `hasOverdue`, `totalFineOn`.
- Методы доступа: `getIssuedOn`, `getLoans`, `getStudentId`.
- Ассоциации: Book, BookLoan, Date, Money, Student.

#### Methodist (наследует Person)

Сотрудник деканата (методист). Файл: `include/studis/People.h`.

- Поля: `employeeId_` (int), `office_` (std::string), `processedDocuments_` (int).
- Поведения: `canIssueCertificateTo`, `registerDocument`, `verifyStudentData`.
- Методы доступа: `getEmployeeId`, `getOffice`, `getProcessedDocuments`.
- Ассоциации: Person, Student.

#### Money

Денежная сумма, хранится в копейках, чтобы избежать ошибок округления. Файл: `include/studis/Common.h`.

- Поля: `cents_` (long long), `currency_` (std::string).
- Поведения: `applyPercent`, `fromUnits`, `isGreaterOrEqual`, `isNegative`, `isZero`, `minus`, `multiply`, `plus`, `toString`.
- Методы доступа: `getCents`, `getCurrency`.
- Ассоциации: —.

#### Person

Человек: общие данные студентов, преподавателей и сотрудников. Файл: `include/studis/People.h`.

- Поля: `name_` (std::string), `birthDate_` (Date), `contact_` (ContactInfo), `address_` (Address).
- Поведения: `ageOn`, `moveTo`, `updateContact`.
- Методы доступа: `getAddress`, `getBirthDate`, `getContact`, `getName`.
- Ассоциации: Address, ContactInfo, Date.

#### Professor (наследует Teacher)

Профессор: руководит дипломниками и ведёт научную работу. Файл: `include/studis/People.h`.

- Поля: `publications_` (int), `supervisedIds_` (std::vector<int>), `grantFunds_` (Money).
- Поведения: `canSupervise`, `publishPaper`, `receiveGrant`, `spendGrant`, `supervise`, `supervises`.
- Методы доступа: `getGrantFunds`, `getPublications`, `getSupervisedIds`.
- Ассоциации: Money, Student, Teacher.

#### RecordBook

Зачётная книжка студента. Файл: `include/studis/Grades.h`.

- Поля: `studentId_` (int), `entries_` (std::vector<RecordEntry>).
- Поведения: `addEntry`, `bestScore`, `earnedCredits`, `failedCourses`, `gpa`, `hasHonors`, `hasPassed`.
- Методы доступа: `getEntries`, `getStudentId`.
- Ассоциации: RecordEntry, Student.

#### RecordEntry

Запись в зачётной книжке: дисциплина, оценка и попытка. Файл: `include/studis/Grades.h`.

- Поля: `courseCode_` (std::string), `credits_` (int), `grade_` (Grade), `takenOn_` (Date), `attempt_` (int).
- Поведения: `isPassed`, `weightedPoints`.
- Методы доступа: `getAttempt`, `getCourseCode`, `getCredits`, `getGrade`, `getTakenOn`.
- Ассоциации: Date, Grade.

#### Retake

Пересдача экзамена. Файл: `include/studis/Assessment.h`.

- Поля: `studentId_` (int), `examId_` (int), `attempt_` (int), `scheduledOn_` (Date).
- Поведения: `daysUntil`, `isLastAttempt`, `nextAttempt`, `reschedule`.
- Методы доступа: `getAttempt`, `getExamId`, `getScheduledOn`, `getStudentId`.
- Ассоциации: Date, ExamResult.

#### Scholarship

Вид стипендии. Файл: `include/studis/Finance.h`.

- Поля: `name_` (std::string), `monthlyAmount_` (Money), `minGpa_` (double), `durationMonths_` (int).
- Поведения: `amountFor`, `isEligible`, `totalValue`.
- Методы доступа: `getDurationMonths`, `getMinGpa`, `getMonthlyAmount`, `getName`.
- Ассоциации: Money, RecordBook.

#### ScholarshipAward

Назначение стипендии студенту. Файл: `include/studis/Finance.h`.

- Поля: `studentId_` (int), `scholarshipName_` (std::string), `monthlyAmount_` (Money), `startDate_` (Date), `endDate_` (Date), `months_` (int), `revoked_` (bool).
- Поведения: `isActiveOn`, `monthsLeft`, `revoke`, `totalPaidUntil`.
- Методы доступа: `getEndDate`, `getMonthlyAmount`, `getScholarshipName`, `getStartDate`, `getStudentId`, `isRevoked`.
- Ассоциации: Date, Money, RecordBook, Scholarship, Student.

#### Semester

Учебный семестр. Файл: `include/studis/Structure.h`.

- Поля: `year_` (int), `term_` (Term), `startDate_` (Date), `examStartDate_` (Date), `endDate_` (Date).
- Поведения: `daysUntilExams`, `isActiveOn`, `isExamPeriodOn`, `lengthInDays`.
- Методы доступа: `getEndDate`, `getExamStartDate`, `getStartDate`, `getTerm`, `getYear`.
- Ассоциации: Date.

#### Student (наследует Person)

Студент вуза. Файл: `include/studis/People.h`.

- Поля: `id_` (int), `programCode_` (std::string), `groupCode_` (std::string), `admissionDate_` (Date), `status_` (StudentStatus), `failedExams_` (int).
- Поведения: `assignGroup`, `changeProgram`, `clearFailedExam`, `expel`, `goOnLeave`, `graduate`, `isActive`, `isOnProbation`, `registerFailedExam`, `returnFromLeave`, `yearOfStudyOn`.
- Методы доступа: `getAdmissionDate`, `getFailedExams`, `getGroupCode`, `getId`, `getProgramCode`, `getStatus`.
- Ассоциации: Date, Person.

#### StudentCard

Студенческий билет с электронным кошельком. Файл: `include/studis/Life.h`.

- Поля: `studentId_` (int), `number_` (std::string), `issuedOn_` (Date), `validUntil_` (Date), `balance_` (Money).
- Поведения: `extend`, `isValidOn`, `pay`, `topUp`.
- Методы доступа: `getBalance`, `getIssuedOn`, `getNumber`, `getStudentId`, `getValidUntil`.
- Ассоциации: Date, Money, Student.

#### StudentInformationSystem

Информационная система университета: объединяет студентов, преподавателей, дисциплины, расписание и финансы. Файл: `include/studis/StudentInformationSystem.h`.

- Поля: `name_` (std::string), `address_` (Address), `semester_` (Semester), `nextId_` (int), `students_` (std::vector<Student>), `teachers_` (std::vector<Teacher>), `programs_` (std::vector<StudyProgram>), `groups_` (std::vector<Group>), `courses_` (std::vector<Course>), `offerings_` (std::vector<CourseOffering>), `classrooms_` (std::vector<Classroom>), `recordBooks_` (std::vector<RecordBook>), `contracts_` (std::vector<TuitionContract>), `timetable_` (Timetable), `dormitory_` (Dormitory).
- Поведения: `addClassroom`, `addCourse`, `addDormitoryRoom`, `addProgram`, `admitStudent`, `assignToGroup`, `canGraduate`, `createGroup`, `debtors`, `enrollStudent`, `findClassroom`, `findCourse`, `findGroup`, `findOffering`, `findProgram`, `findRecordBook`, `findStudent`, `findTeacher`, `gpaOf`, `hireTeacher`, `openOffering`, `payTuition`, `recordGrade`, `scheduleLesson`, `settleInDormitory`, `signContract`.
- Методы доступа: `getAddress`, `getClassrooms`, `getContracts`, `getCourses`, `getDormitory`, `getGroups`, `getName`, `getOfferings`, `getPrograms`, `getSemester`, `getStudents`, `getTeachers`, `getTimetable`.
- Ассоциации: Address, Classroom, Course, CourseOffering, Date, Dormitory, DormitoryRoom, Group, LessonTime, Money, Person, RecordBook, Semester, Student, StudyProgram, Teacher, Timetable, TuitionContract.

#### StudyProgram

Направление подготовки. Файл: `include/studis/Structure.h`.

- Поля: `code_` (std::string), `name_` (std::string), `durationYears_` (int), `requiredCredits_` (int), `level_` (std::string).
- Поведения: `creditsPerSemester`, `graduationYear`, `isBachelor`, `totalSemesters`.
- Методы доступа: `getCode`, `getDurationYears`, `getLevel`, `getName`, `getRequiredCredits`.
- Ассоциации: Date.

#### Submission

Работа студента по заданию. Файл: `include/studis/Courses.h`.

- Поля: `assignmentId_` (int), `studentId_` (int), `submittedOn_` (Date), `score_` (int), `penaltyPercent_` (int).
- Поведения: `grade`, `isGraded`, `percentOfMax`.
- Методы доступа: `getAssignmentId`, `getPenaltyPercent`, `getScore`, `getStudentId`, `getSubmittedOn`.
- Ассоциации: Assignment, Date, Student.

#### Teacher (наследует Person)

Преподаватель. Файл: `include/studis/People.h`.

- Поля: `employeeId_` (int), `department_` (std::string), `hiredOn_` (Date), `degree_` (std::string), `weeklyHours_` (int).
- Поведения: `addWorkload`, `freeHours`, `hasDoctorateDegree`, `isSeniorOn`, `releaseWorkload`, `yearsOfServiceOn`.
- Методы доступа: `getDegree`, `getDepartment`, `getEmployeeId`, `getHiredOn`, `getWeeklyHours`.
- Ассоциации: Date, Person.

#### Thesis

Дипломная работа. Файл: `include/studis/Assessment.h`.

- Поля: `id_` (int), `studentId_` (int), `supervisorId_` (int), `title_` (std::string), `deadline_` (Date), `status_` (ThesisStatus), `score_` (int).
- Поведения: `defend`, `isDefended`, `isLate`, `reject`, `submit`.
- Методы доступа: `getDeadline`, `getId`, `getScore`, `getStatus`, `getStudentId`, `getSupervisorId`, `getTitle`.
- Ассоциации: Date, Grade, Professor, Student.

#### Timetable

Расписание занятий на семестр. Файл: `include/studis/Courses.h`.

- Поля: `semesterYear_` (int), `lessons_` (std::vector<Lesson>).
- Поведения: `addLesson`, `hasConflict`, `lessonsOfTeacher`, `lessonsOn`, `removeLesson`, `totalHoursFor`.
- Методы доступа: `getLessons`, `getSemesterYear`.
- Ассоциации: Lesson, Semester.

#### TransferRequest

Заявление на перевод на другое направление. Файл: `include/studis/Admission.h`.

- Поля: `id_` (int), `studentId_` (int), `fromProgramCode_` (std::string), `toProgramCode_` (std::string), `reason_` (std::string), `status_` (RequestStatus), `approvedById_` (int).
- Поведения: `applyTo`, `approve`, `isPending`, `reject`.
- Методы доступа: `getApprovedById`, `getFromProgramCode`, `getId`, `getReason`, `getStatus`, `getStudentId`, `getToProgramCode`.
- Ассоциации: Dean, Student, StudyProgram.

#### TuitionContract

Договор об оплате обучения. Файл: `include/studis/Finance.h`.

- Поля: `id_` (int), `studentId_` (int), `programCode_` (std::string), `totalFee_` (Money), `paid_` (Money), `installments_` (int).
- Поведения: `installmentAmount`, `isFullyPaid`, `outstanding`, `paidInstallments`, `registerPayment`.
- Методы доступа: `getId`, `getInstallments`, `getPaid`, `getProgramCode`, `getStudentId`, `getTotalFee`.
- Ассоциации: Money, Student, StudyProgram.

#### TuitionPayment

Платёж за обучение. Файл: `include/studis/Finance.h`.

- Поля: `id_` (int), `contractId_` (int), `amount_` (Money), `paidOn_` (Date), `method_` (std::string).
- Поведения: `isLate`, `isOnline`, `receiptText`.
- Методы доступа: `getAmount`, `getContractId`, `getId`, `getMethod`, `getPaidOn`.
- Ассоциации: Date, Money, TuitionContract.
