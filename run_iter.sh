
file=$1
num=$2

cmake . -DCMAKE_BUILD_TYPE=Debug
make

for i in $(seq 0 $num); do
    echo $i 
    out/model_checker $file $i
done
