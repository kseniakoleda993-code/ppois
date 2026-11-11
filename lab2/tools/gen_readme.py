#!/usr/bin/env python3
"""Генерирует README.md из заголовочных файлов include/autoshop/*.h.

Правила разбора (код написан в строгом формате, поэтому достаточно регулярных выражений):
  * поле - строка с отступом в 4 пробела вида "Тип имя_;" (имя оканчивается подчёркиванием);
  * метод - строка с отступом в 4 пробела, содержащая "(", в секции public;
  * геттер - метод, тело которого целиком "{ return поле_; }" (в «поведения» не входит);
  * ассоциация - другой доменный класс, встречающийся в типе поля или в параметрах public-метода/конструктора.
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
HEADERS = ROOT / "include" / "autoshop"
MIN_CLASSES, MIN_FIELDS, MIN_BEHAVIORS, MIN_ASSOCIATIONS, MIN_EXCEPTIONS = 50, 150, 100, 30, 12

CLASS_RE = re.compile(r"^class (\w+)(?: : public (\w+))? \{$")
ACCESS_RE = re.compile(r"^(public|private|protected):$")
FIELD_RE = re.compile(r"^((?:[\w:<>,&*\s]+?)\s+)(\w+_)(?:\s*=\s*[^;]+)?;$")
METHOD_RE = re.compile(r"^(?:(?:explicit|static|virtual|inline)\s+)*(.*?)(~?\w+|operator\S+?)\s*\(")
ACCESSOR_RE = re.compile(r"\{ return \w+_; \}\s*$")


class ClassInfo:
    def __init__(self, name, base, header, brief):
        self.name, self.base, self.header, self.brief = name, base, header, brief
        self.fields = []        # (тип, имя)
        self.methods = []       # (имя, подпись, это_геттер)
        self.association_text = []

    @property
    def behaviors(self):
        return sorted({name for name, _, is_getter in self.methods if not is_getter})

    @property
    def accessors(self):
        return sorted({name for name, _, is_getter in self.methods if is_getter})


def read_signature(lines, index):
    text = lines[index].strip()
    while text.count("(") > text.count(")") and index + 1 < len(lines):
        index += 1
        text += " " + lines[index].strip()
    return text


def parse_header(path):
    lines = path.read_text(encoding="utf-8").splitlines()
    classes, current, access, brief = [], None, "private", ""
    for index, line in enumerate(lines):
        match = CLASS_RE.match(line)
        if match:
            current = ClassInfo(match.group(1), match.group(2), path.name, brief)
            access = "private"
            continue
        if line.startswith("///"):
            brief = line[3:].strip()
            continue
        if current is None:
            continue
        if line == "};":
            classes.append(current)
            current = None
            continue
        access_match = ACCESS_RE.match(line)
        if access_match:
            access = access_match.group(1)
            continue
        if not re.match(r"^    \S", line) or line.strip().startswith(("//", "}", "using", "template", "friend")):
            continue
        parse_member(current, access, read_signature(lines, index))
    return classes


def parse_member(info, access, text):
    if "(" not in text:
        field = FIELD_RE.match(text)
        if field:
            info.fields.append((field.group(1).strip(), field.group(2)))
            info.association_text.append(field.group(1))
        return
    method = METHOD_RE.match(text)
    if not method or access != "public":
        return
    name = method.group(2)
    params = text[text.index("(") + 1: text.rindex(")")] if ")" in text else ""
    if name == info.name or name.startswith(("~", "operator")):
        info.association_text.append(params)
        return
    info.methods.append((name, text.split("{")[0].strip(), bool(ACCESSOR_RE.search(text))))
    info.association_text.append(params)


def collect():
    domain, exceptions = [], []
    for path in sorted(HEADERS.glob("*.h")):
        (exceptions if path.name == "Exceptions.h" else domain).extend(parse_header(path))
    return domain, exceptions


def associations(info, names):
    haystack = " ".join(info.association_text)
    return sorted(n for n in names if n != info.name and re.search(rf"\b{n}\b", haystack))


def stats_table(rows):
    lines = ["| Показатель | Требование | Получено | Выполнено |", "| --- | --- | --- | --- |"]
    for title, need, got in rows:
        lines.append(f"| {title} | не менее {need} | {got} | {'да' if got >= need else 'НЕТ'} |")
    return "\n".join(lines)


def class_table(domain, names):
    lines = ["| Класс | Поля | Методы | Поведения | Ассоциации (связанные классы) |", "| --- | --- | --- | --- | --- |"]
    for info in sorted(domain, key=lambda c: c.name):
        links = ", ".join(associations(info, names)) or "—"
        lines.append(f"| {info.name} | {len(info.fields)} | {len({m[0] for m in info.methods})} | "
                     f"{len(info.behaviors)} | {links} |")
    return "\n".join(lines)


def class_details(domain, names):
    parts = []
    for info in sorted(domain, key=lambda c: c.name):
        base = f" (наследует {info.base})" if info.base else ""
        parts.append(f"#### {info.name}{base}\n")
        parts.append(f"{info.brief} Файл: `include/autoshop/{info.header}`.\n")
        parts.append("- Поля: " + ", ".join(f"`{n}` ({t})" for t, n in info.fields) + ".")
        parts.append("- Поведения: " + (", ".join(f"`{b}`" for b in info.behaviors) or "—") + ".")
        parts.append("- Методы доступа: " + (", ".join(f"`{a}`" for a in info.accessors) or "—") + ".")
        parts.append("- Ассоциации: " + (", ".join(associations(info, names)) or "—") + ".\n")
    return "\n".join(parts)


def exception_list(exceptions):
    return "\n".join(f"- `{e.name}` — {e.brief}" + (f" (наследует `{e.base}`)" if e.base else "")
                     for e in exceptions)


HEAD = """# Лабораторная работа №2. Автомастерская

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

"""


def main():
    domain, exceptions = collect()
    names = [c.name for c in domain]
    fields = sum(len(c.fields) for c in domain)
    behaviors = sum(len(c.behaviors) for c in domain)
    links = sum(len(associations(c, names)) for c in domain)
    rows = [("Классы предметной области", MIN_CLASSES, len(domain)), ("Поля", MIN_FIELDS, fields),
            ("Уникальные поведения", MIN_BEHAVIORS, behaviors), ("Ассоциации", MIN_ASSOCIATIONS, links),
            ("Собственные классы исключений", MIN_EXCEPTIONS, len(exceptions))]
    text = HEAD + "## Итоговая статистика\n\n" + stats_table(rows) + "\n\n"
    text += ("«Поведения» — открытые методы с содержательной логикой (геттеры не считаются). «Ассоциация» — случай, когда "
             "другой класс предметной области используется как поле или как параметр метода/конструктора.\n\n")
    text += "## Классы\n\nФормат: `Класс | поля | методы | поведения | связанные классы`.\n\n"
    text += class_table(domain, names) + "\n\n## Исключения\n\n" + exception_list(exceptions) + "\n\n"
    text += "## Подробное описание классов\n\n" + class_details(domain, names)
    (ROOT / "README.md").write_text(text, encoding="utf-8")
    print(f"classes={len(domain)} fields={fields} behaviors={behaviors} associations={links} exceptions={len(exceptions)}")
    failed = [t for t, need, got in rows if got < need]
    if failed:
        print("Не выполнены требования:", ", ".join(failed))
        sys.exit(1)


if __name__ == "__main__":
    main()
