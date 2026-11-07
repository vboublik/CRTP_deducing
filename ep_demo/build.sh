#!/usr/bin/env bash
# ===== Демонстрація проблеми виразності через окрему компіляцію =====
# Запуск з цієї папки:  bash build.sh
cd "$(dirname "$0")"

echo
echo "=== 1. VIRTUAL: старий код компілюємо ОДИН раз ==="
g++ -std=c++20 -c 1_virtual/old/reports.cpp -o 1_virtual/old/reports.o
echo "--- новий тип XmlReport + готовий reports.o:"
g++ -std=c++20 1_virtual/new/main.cpp 1_virtual/old/reports.o -o virtual_new_type && ./virtual_new_type

echo
echo "=== 2. VARIANT: старий код компілюємо ОДИН раз ==="
g++ -std=c++20 -c 2_variant/old/reports.cpp -o 2_variant/old/reports.o
echo "--- (а) нова операція Validate + готовий reports.o:"
g++ -std=c++20 2_variant/new/new_operation.cpp 2_variant/old/reports.o -o variant_new_op && ./variant_new_op
echo
echo "--- (б) новий тип XmlReport + готовий reports.o (ОЧІКУВАНА ПОМИЛКА):"
g++ -std=c++20 2_variant/new/new_type.cpp 2_variant/old/reports.o -o variant_new_type
echo
echo "--- (б) те саме, але зі ПЕРЕКОМПІЛЬОВАНИМ старим кодом:"
g++ -std=c++20 -DWITH_XML -c 2_variant/old/reports.cpp -o 2_variant/old/reports_xml.o
g++ -std=c++20 2_variant/new/new_type.cpp 2_variant/old/reports_xml.o -o variant_new_type && ./variant_new_type
