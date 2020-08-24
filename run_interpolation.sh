
file=$1

cmake . -DCMAKE_BUILD_TYPE=Debug
make
 
out/model_checker $file
