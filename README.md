# Programming_3_semester

Файлы с программой: 
  client.c
  server.c
  common.h
  main.c
  
Makefile для сборки проекта

команда
make - сборка

Запуск
оттельно запускается сервер и отдельно клиент
сделано при помощи FIFO(повышенная сложность)
пример запуска:
./server & sleep 1 && ./main 123.txt f 1234 f

./server & sleep 1 && ./main filename symv ...
