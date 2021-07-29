g++ -Iinclude -I. -std=c++11 -c src/*.cc
ar rcs libwebm.a *.o
g++ -c src/webm_splitter.cpp -Iinclude
g++ webm_splitter.o libwebm.a -o webm_splitter
rm ./*.o libwebm.a

