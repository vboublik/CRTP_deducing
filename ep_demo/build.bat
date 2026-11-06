@echo off
chcp 65001 >nul
rem ===== Демонстрація проблеми виразності через окрему компіляцію =====
rem Запускати з цієї папки у вікні, де працює g++.

echo.
echo === 1. VIRTUAL: старий код компілюємо ОДИН раз ===
g++ -std=c++20 -c 1_virtual\old\reports.cpp -o 1_virtual\old\reports.o
echo --- новий тип XmlReport + готовий reports.o:
g++ -std=c++20 1_virtual\new\main.cpp 1_virtual\old\reports.o -o virtual_new_type.exe && virtual_new_type.exe

echo.
echo === 2. VARIANT: старий код компілюємо ОДИН раз ===
g++ -std=c++20 -c 2_variant\old\reports.cpp -o 2_variant\old\reports.o
echo --- (а) нова операція Validate + готовий reports.o:
g++ -std=c++20 2_variant\new\new_operation.cpp 2_variant\old\reports.o -o variant_new_op.exe && variant_new_op.exe
echo.
echo --- (б) новий тип XmlReport + готовий reports.o (ОЧІКУВАНА ПОМИЛКА):
g++ -std=c++20 2_variant\new\new_type.cpp 2_variant\old\reports.o -o variant_new_type.exe
echo.
echo --- (б) те саме, але зі ПЕРЕКОМПІЛЬОВАНИМ старим кодом:
g++ -std=c++20 -DWITH_XML -c 2_variant\old\reports.cpp -o 2_variant\old\reports_xml.o
g++ -std=c++20 2_variant\new\new_type.cpp 2_variant\old\reports_xml.o -o variant_new_type.exe && variant_new_type.exe
