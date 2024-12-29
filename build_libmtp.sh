cd libmtp || git clone https://github.com/libmtp/libmtp.git
cd libmtp || exit

mkdir target

./autogen.sh
y > ./configure --prefix target
make
