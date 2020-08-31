
file=$1

cmake . -DCMAKE_BUILD_TYPE=Release
make
 
out/model_checker $file
