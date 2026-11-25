#pragma once

#include <string>

#include "studis/StudentInformationSystem.h"

namespace testdata {

using namespace studis;

inline Date today() { return Date(2026, 11, 10); }

inline Person makePerson(const std::string& name = "Анна Иванова", int birthYear = 2005) {
    return Person(name, Date(birthYear, 5, 20), ContactInfo("+375 29 123-45-67", "anna@uni.by"),
                  Address("ул. Ленина 1", "Минск", "220000"));
}

inline Student makeStudent(int id = 1, const std::string& programCode = "CS") {
    return Student(id, makePerson(), programCode, Date(2023, 9, 1));
}

inline Teacher makeTeacher(int id = 100, const std::string& department = "Кафедра ПО", const std::string& degree = "phd") {
    return Teacher(id, makePerson("Пётр Сидоров", 1975), department, Date(2005, 9, 1), degree);
}

inline Professor makeProfessor(int id = 101) {
    return Professor(makeTeacher(id, "Кафедра ПО", "doctor"), Money::fromUnits(1000));
}

inline Dean makeDean(int id = 102) { return Dean(makeTeacher(id), "ФКСиС"); }

inline StudyProgram makeProgram(const std::string& code = "CS") {
    return StudyProgram(code, "Информатика", 4, 240, "bachelor");
}

inline Semester makeSemester() {
    return Semester(2026, Term::Autumn, Date(2026, 9, 1), Date(2026, 12, 20), Date(2027, 1, 20));
}

inline Course makeCourse(const std::string& code = "MATH", int credits = 6) {
    return Course(code, "Математика", credits, credits * 36, "Кафедра ПО");
}

inline Building makeBuilding() { return Building("Корпус 1", Address("ул. Мира 5", "Минск", "220001"), 5); }

inline Classroom makeClassroom(const std::string& number = "101", int capacity = 30) {
    return Classroom(makeBuilding(), number, capacity, true);
}

inline CourseOffering makeOffering(int id = 10, int capacity = 2, const std::string& courseCode = "MATH") {
    return CourseOffering(id, makeCourse(courseCode), makeSemester(), makeTeacher(), capacity);
}

inline Lesson makeLesson(int id = 1, const std::string& room = "101", Weekday day = Weekday::Monday, int hour = 9) {
    return Lesson(id, makeOffering(), makeClassroom(room), LessonTime(day, hour, 2), "lecture");
}

inline RecordBook makeRecordBook(const Student& student) { return RecordBook(student); }

inline StudentInformationSystem makeSystem() {
    StudentInformationSystem system("Университет", Address("пр. Независимости 1", "Минск", "220000"), makeSemester());
    system.addProgram(makeProgram());
    system.addProgram(makeProgram("SE"));
    system.addProgram(StudyProgram("MINI", "Мини-курс", 1, 6, "bachelor"));
    system.addCourse(makeCourse("MATH", 6));
    system.addCourse(makeCourse("PHYS", 6));
    system.addClassroom(makeClassroom("101", 30));
    system.addClassroom(makeClassroom("102", 1));
    system.addDormitoryRoom(1, 1);
    system.createGroup("CS-101", "CS", 2023, 2);
    return system;
}

}  // namespace testdata
