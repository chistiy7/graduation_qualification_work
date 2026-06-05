# Планнировщик произодства валов
[![en](https://img.shields.io/badge/lang-en-green.svg)](https://github.com/kolyaka32/Shaft-production-planner/blob/Main/README.md)  [![ru](https://img.shields.io/badge/lang-ru-green.svg)](https://github.com/kolyaka32/Shaft-production-planner/blob/Main/README-ru.md)

Данный репозиторий содержит систему расчёта и планирования производственной цепочки и цеха по производству валов.

## Использование
Изначально требуется ввести входные параметры заготовок и выходных изделий.

![Изображение стадии ввода данных](/screenshots/product-ru.png?raw=true)

Основываясь на данных параметрах программа создаёт производственную цепочку, расчитывая все необходимые параметры и стоимость производства.

![Изображение стадии процесса производства](/screenshots/process-ru.png?raw=true)

Основываясь на полученном количестве оборудования следует корректно разместить его на сетке, создав производственный цех.
Также существует автоматический режим размещения оборудования по сетке с приемлемым качеством расположения.

![Изображение планировщика цеха](/screenshots/factory-ru.png?raw=true)

После всех расчётов программа предоставляет выгрузку в файл результатов всех расчитанных значений для дальнейшего анализа.
Кроме того есть возможность сохранения/загрузки на любом этапе работы программы.


## Поддерживаемые языки
* Английский (English)
* Русский


## Запусе программы
Для простого запуска программы скачайте архив из [релизов](https://github.com/kolyaka32/Shaft-production-planner/releases), распокуйте и запустити 'Planner.exe'!


## Устанока и собственноручная сборка (windows)
1. Установить компилятор C++ (например [MSYS2](https://www.msys2.org/#installation));
2. Установить [Make](https://sourceforge.net/projects/gnuwin32/files/make/3.81/make-3.81.exe/download);
3. Установить [CMake](https://sourceforge.net/projects/cmake.mirror/);
4. Скачать и установить внешнюю библиотеку [SFML](https://github.com/SFML/SFML);
Для собственной сборки: [скачать](https://github.com/SFML/SFML/releases/) исходный код, распоковать в любую папку и запустить командную строку **от имени администратора**:
```bash
cd ./SFML-3.0.1
mkdir build
cd build/
cmake ../
make
make install
```
5. (*Дополнительно) Редактор кода [Visual Studio Code](https://code.visualstudio.com/download) (для более простой сборки) (Рекомендуется использовать с плагинами на c++ и cmake).
6. Компиляция самого проекта:
```bash
cd ./Shaft-production-planner
mkdir build
cmake --build ./build --config Release --target all --
```


## Установка и сборка (macOS)

1. Установите инструменты (Homebrew):
```bash
brew install cmake sfml
```
Требуется **SFML 3** (пакет `sfml` в Homebrew).
2. Соберите проект из корня репозитория:
```bash
chmod +x scripts/build.sh scripts/run.sh
./scripts/build.sh
```
3. Запуск (обязательно из **корня** проекта — рядом должна быть папка `resources/`):
```bash
./scripts/run.sh
```
Или вручную:
```bash
cd ./Shaft-production-planner-Main   # корень с resources/
./build/Planner
```

**Важно:** графическое окно открывается в обычном Terminal.app / iTerm. Запуск из изолированной среды без GUI может завершаться с ошибкой `mutex lock failed`.

На Apple Silicon CMake подхватывает SFML из `/opt/homebrew` автоматически.


## Устанока и собственноручная сборка (unux системы)
1. Проверить наличие компилятора[g++], [make] и [cmake];
2. Установть внешнюю библиотеку [SFML](https://github.com/SFML/SFML)
(Для Debian подобных систем):
```bash
sudo apt-get update
sudo apt-get install libsfml-dev
```
Или собрать сомостоятельно: [скачать](https://github.com/SFML/SFML/releases/) исходный код, распоковать в любую папку и запустить командную строку **от имени администратора**:
```bash
cd ./SFML-3.0.1
mkdir build
cd build/
cmake ../
make
make install
```
3. (*Дополнительно) Редактор кода [Visual Studio Code](https://code.visualstudio.com/download) (для более простой сборки) (Рекомендуется использовать с плагинами на c++ и cmake).
4. Компиляция самого проекта:
```bash
cd ./Shaft-production-planner
mkdir build
cmake --build ./build --config Release --target all --
```


## Водготовленные исходники
Исходные файлы с полностью собранными библиотеками и проектом можно найти в [релизах](https://github.com/kolyaka32/Shaft-production-planner/releases) (Development-kit).


## Использованные сторонние библиотеки:
* https://github.com/SFML/SFML/releases/
