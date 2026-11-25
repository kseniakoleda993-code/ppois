#include "DemoData.h"

namespace studis {

namespace {
constexpr int kDemoGroupSize = 25;
constexpr int kDemoBigRoom = 40;
constexpr int kDemoSmallRoom = 2;
constexpr int kDemoDormitoryRooms = 3;
constexpr int kDemoRoomCapacity = 2;
constexpr int kBachelorYears = 4;
constexpr int kBachelorCredits = 240;
constexpr int kProgrammingCredits = 6;
constexpr int kProgrammingHours = 216;
}  // namespace

namespace {

void addCatalog(StudentInformationSystem& system, int year) {
    system.addProgram(StudyProgram("CS", "Информатика", kBachelorYears, kBachelorCredits, kBachelorLevel));
    system.addProgram(StudyProgram("SE", "Программная инженерия", kBachelorYears, kBachelorCredits, kBachelorLevel));
    system.addCourse(Course("MATH", "Математический анализ", kProgrammingCredits, kProgrammingHours, "Кафедра математики"));
    system.addCourse(Course("PROG", "Программирование", kProgrammingCredits, kProgrammingHours, "Кафедра ПО"));
    system.createGroup("CS-101", "CS", year, kDemoGroupSize);
}

void addCampus(StudentInformationSystem& system) {
    const Building building("Главный корпус", Address("пр. Независимости 1", "Минск", "220000"), 5);
    system.addClassroom(Classroom(building, "101", kDemoBigRoom, true));
    system.addClassroom(Classroom(building, "102", kDemoSmallRoom, false));
    for (int room = 1; room <= kDemoDormitoryRooms; ++room) {
        system.addDormitoryRoom(room, kDemoRoomCapacity);
    }
}

void addPeople(StudentInformationSystem& system, int year) {
    const Person teacher("Пётр Сидоров", Date(1975, 3, 12), ContactInfo("+375 29 111-22-33", "petr@uni.by"),
                         Address("ул. Лесная 7", "Минск", "220002"));
    system.hireTeacher(teacher, "Кафедра ПО", Date(2005, 9, 1), "phd");
    const Person student("Анна Иванова", Date(2006, 5, 20), ContactInfo("+375 29 123-45-67", "anna@uni.by"),
                         Address("ул. Ленина 1", "Минск", "220000"));
    system.admitStudent(student, "CS", Date(year, 9, 1));
}

}  // namespace

StudentInformationSystem makeDemoSystem(const Date& today) {
    const int year = today.getYear();
    const Semester semester(year, Term::Autumn, Date(year, 9, 1), Date(year, 12, 20), Date(year + 1, 1, 20));
    StudentInformationSystem system("Информационная система студентов",
                                    Address("пр. Независимости 1", "Минск", "220000"), semester);
    addCatalog(system, year);
    addCampus(system);
    addPeople(system, year);
    return system;
}

}  // namespace studis
