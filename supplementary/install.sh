echo "Installing Voro++2D"
cd Voro++2D
make
sudo mv src/libvoro++_2d.a /usr/local/lib/
sudo mkdir /usr/local/include/Voro++2D
sudo cp src/*.hh /usr/local/include/Voro++2D/
cd ..

echo "Installing Boost.SIMD"
git clone https://github.com/NumScale/boost.simd.git
cd boost.simd
mkdir build && cd build
cmake .. -DUSE_SELF_BOOST=
make update.boost.dispatch
sudo make install
cd ../..

echo "Installing iGraph"
get http://igraph.org/nightly/get/c/igraph-0.7.1.tar.gz
gunzip igraph-0.7.1.tar.gz
tar -xf igraph-0.7.1.tar
cd igraph-0.7.1
./configure
make
sudo make install
cd ..

echo "Installing GSL"
wget ftp://ftp.gnu.org/gnu/gsl/gsl-2.1.tar.gz
gunzip gsl-2.1.tar.gz
tar -xf gsl-2.1.tar
cd gsl-2.1
./configure
make
sudo make install
cd ..

echo "Installing STXXL"
wget http://sourceforge.net/projects/stxxl/files/stxxl/1.4.1/stxxl-1.4.1.tar.gz
gunzip stxxl-1.4.1.tar.gz
tar -xf stxxl-1.4.1.tar
cd stxxl-1.4.1
mkdir build && cd build
cmake ..
make
sudo make install
cd ../..

echo "Installing OpenCV"
wget https://github.com/Itseez/opencv/archive/3.1.0.zip
unzip opencv-3.1.0.zip
cd opencv-3.1.0
mkdir build && cd build
cmake ..
make
sudo make install
cd ../..

echo "Done"
