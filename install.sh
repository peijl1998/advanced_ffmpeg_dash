#./configure --prefix=$HOME/ffmpeg-install \
#           --enable-demuxer=dash \
#           --enable-libxml2 \
#           --enable-gpl  \
#           --enable-libx264 \
#           --enable-libvpx \
#           --enable-libvorbis \
#           --enable-debug=3 \
#           --disable-optimizations \
#           --disable-asm  \
#           --disable-stripping \

make -j$(nproc)
make install
