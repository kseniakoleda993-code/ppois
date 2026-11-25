#include "StudentSystemMenu.h"

#include <istream>
#include <ostream>
#include <sstream>

namespace studis {

namespace {
constexpr int kDemoLessonHours = 2;
constexpr int kDefaultInstallments = 4;
}  // namespace

std::string toText(StudentStatus status) {
    switch (status) {
        case StudentStatus::Active: return "учится";
        case StudentStatus::OnLeave: return "в академотпуске";
        case StudentStatus::Expelled: return "отчислен";
        case StudentStatus::Graduated: return "выпускник";
    }
    return "неизвестно";
}

StudentSystemMenu::StudentSystemMenu(StudentInformationSystem& system, const Date& today, std::istream& in,
                                     std::ostream& out)
    : system_(system), today_(today), in_(in), out_(out) {}

void StudentSystemMenu::run() {
    Menu menu(system_.getName(), in_, out_, "Выход");
    buildMenu(menu);
    menu.run();
}

void StudentSystemMenu::buildMenu(Menu& menu) {
    menu.addItem("Показать студентов, группы и преподавателей", [this] { showStudents(); });
    menu.addItem("Принять студента", [this] { admitStudent(); });
    menu.addItem("Записать студента в группу", [this] { assignToGroup(); });
    menu.addItem("Показать дисциплины и аудитории", [this] { showCourses(); });
    buildAcademicMenu(menu);
    buildMoneyMenu(menu);
}

void StudentSystemMenu::buildAcademicMenu(Menu& menu) {
    menu.addItem("Открыть курс (дисциплина + преподаватель)", [this] { openOffering(); });
    menu.addItem("Записать студента на курс", [this] { enrollStudent(); });
    menu.addItem("Поставить занятие в расписание", [this] { scheduleLesson(); });
    menu.addItem("Выставить оценку", [this] { recordGrade(); });
    menu.addItem("Показать зачётку и средний балл", [this] { showRecordBook(); });
    menu.addItem("Показать должников", [this] { showDebtors(); });
    menu.addItem("Проверить, может ли студент выпуститься", [this] { checkGraduation(); });
}

void StudentSystemMenu::buildMoneyMenu(Menu& menu) {
    menu.addItem("Заключить договор на обучение", [this] { signContract(); });
    menu.addItem("Оплатить обучение", [this] { payTuition(); });
    menu.addItem("Заселить в общежитие", [this] { settleInDormitory(); });
}

int StudentSystemMenu::askInt(const std::string& message) { return promptValue<int>(in_, out_, message); }

Date StudentSystemMenu::askDate(const std::string& message) {
    std::istringstream text(promptLine(in_, out_, message));
    int year = 0;
    int month = 0;
    int day = 0;
    char firstDash = 0;
    char secondDash = 0;
    if (!(text >> year >> firstDash >> month >> secondDash >> day) || firstDash != '-' || secondDash != '-') {
        throw std::runtime_error("Дата должна быть в формате ГГГГ-ММ-ДД");
    }
    return Date(year, month, day);
}

void StudentSystemMenu::showStudents() {
    for (const Teacher& teacher : system_.getTeachers()) {
        out_ << "Преподаватель #" << teacher.getEmployeeId() << " " << teacher.getName() << ", нагрузка "
             << teacher.getWeeklyHours() << " ч/нед\n";
    }
    for (const Student& student : system_.getStudents()) {
        out_ << "Студент #" << student.getId() << " " << student.getName() << ", направление " << student.getProgramCode()
             << ", группа " << (student.getGroupCode().empty() ? "—" : student.getGroupCode()) << ", "
             << toText(student.getStatus()) << "\n";
    }
    for (const Group& group : system_.getGroups()) {
        out_ << "Группа " << group.getCode() << ": " << group.size() << "/" << group.getMaxSize() << "\n";
    }
}

void StudentSystemMenu::admitStudent() {
    const std::string name = promptLine(in_, out_, "ФИО: ");
    const Date birthDate = askDate("Дата рождения (ГГГГ-ММ-ДД): ");
    const std::string phone = promptLine(in_, out_, "Телефон: ");
    const std::string programCode = promptLine(in_, out_, "Код направления: ");
    const Person person(name, birthDate, ContactInfo(phone, ""), Address("не указан", "не указан", "000000"));
    out_ << "Студент принят, номер " << system_.admitStudent(person, programCode, today_) << "\n";
}

void StudentSystemMenu::assignToGroup() {
    const int studentId = askInt("Номер студента: ");
    system_.assignToGroup(studentId, promptLine(in_, out_, "Код группы: "));
    out_ << "Студент записан в группу\n";
}

void StudentSystemMenu::showCourses() {
    for (const Course& course : system_.getCourses()) {
        out_ << course.getCode() << ": " << course.getTitle() << ", " << course.getCredits() << " з.е.\n";
    }
    for (const Classroom& classroom : system_.getClassrooms()) {
        out_ << "Аудитория " << classroom.getNumber() << " (" << classroom.label() << "), мест: "
             << classroom.getCapacity() << "\n";
    }
    for (const CourseOffering& offering : system_.getOfferings()) {
        out_ << "Курс #" << offering.getId() << " " << offering.getCourseCode() << ", свободно мест: "
             << offering.seatsLeft() << "\n";
    }
}

void StudentSystemMenu::openOffering() {
    const std::string courseCode = promptLine(in_, out_, "Код дисциплины: ");
    const int teacherId = askInt("Номер преподавателя: ");
    const int capacity = askInt("Вместимость: ");
    out_ << "Курс открыт, номер " << system_.openOffering(courseCode, teacherId, capacity) << "\n";
}

void StudentSystemMenu::enrollStudent() {
    const int studentId = askInt("Номер студента: ");
    system_.enrollStudent(studentId, askInt("Номер курса: "));
    out_ << "Студент записан на курс\n";
}

void StudentSystemMenu::scheduleLesson() {
    const int offeringId = askInt("Номер курса: ");
    const std::string room = promptLine(in_, out_, "Номер аудитории: ");
    const int hour = askInt("Час начала (8-19), понедельник: ");
    const LessonTime time(Weekday::Monday, hour, kDemoLessonHours);
    out_ << "Занятие добавлено, номер " << system_.scheduleLesson(offeringId, room, time, kLectureKind) << "\n";
}

void StudentSystemMenu::recordGrade() {
    const int studentId = askInt("Номер студента: ");
    const std::string courseCode = promptLine(in_, out_, "Код дисциплины: ");
    system_.recordGrade(studentId, courseCode, askInt("Баллы (0-100): "), today_);
    out_ << "Оценка выставлена\n";
}

void StudentSystemMenu::showRecordBook() {
    const int studentId = askInt("Номер студента: ");
    for (const RecordEntry& entry : system_.findRecordBook(studentId).getEntries()) {
        out_ << entry.getCourseCode() << ": " << entry.getGrade().getPoints() << " (" << entry.getGrade().letter() << ")\n";
    }
    out_ << "Средний балл: " << system_.gpaOf(studentId) << "\n";
}

void StudentSystemMenu::signContract() {
    const int studentId = askInt("Номер студента: ");
    const int fee = askInt("Стоимость обучения (целых BYN): ");
    out_ << "Договор заключён, номер " << system_.signContract(studentId, Money::fromUnits(fee), kDefaultInstallments) << "\n";
}

void StudentSystemMenu::payTuition() {
    const int contractId = askInt("Номер договора: ");
    const int units = askInt("Сумма (целых BYN): ");
    const std::string method = promptLine(in_, out_, "Способ (online/cash/bank): ");
    system_.payTuition(contractId, Money::fromUnits(units), today_, method);
    out_ << "Платёж принят\n";
}

void StudentSystemMenu::settleInDormitory() {
    const int room = system_.settleInDormitory(askInt("Номер студента: "));
    out_ << "Студент заселён в комнату " << room << "\n";
}

void StudentSystemMenu::showDebtors() {
    const std::vector<int> debtors = system_.debtors();
    out_ << "Должников: " << debtors.size() << "\n";
    for (int id : debtors) {
        out_ << "  студент #" << id << " " << system_.findStudent(id).getName() << "\n";
    }
}

void StudentSystemMenu::checkGraduation() {
    const int studentId = askInt("Номер студента: ");
    out_ << (system_.canGraduate(studentId) ? "Студент набрал нужное число зачётных единиц\n"
                                            : "Зачётных единиц пока недостаточно\n");
}

}  // namespace studis
