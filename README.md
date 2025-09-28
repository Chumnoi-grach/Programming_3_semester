# Programming_3_semester

Файлы с программой: 
  client.c
  server.c
  common.h
  main.c
Makefile для сборки проекта

make build - сборка
make clean - удалить исполнительные файлы
make rebuild - ребилд проекта

Запуск
оттельно запускается сервер и отдельно клиент
сделано при помощи FIFO(повышенная сложность)
пример запуска:
./server & ./main 123.txt f 1234 f

./server & ./main filename symv ...
