# configure with open mp support (using gcc)
./configure --with-boost-include=/home/peter/boost_1_55_0 --with-boost-lib=/home/peter/boost_1_55_0/stage/lib --enable-openmp CXXFLAGS="-m64 -O3 -g -Wall -std=c++11" LIBS="-lntl -lgmp -lm -lboost_timer -lboost_chrono -lboost_system"