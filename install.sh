./configure --prefix=$HOME/ffmpeg-install --enable-demuxer=dash --enable-libxml2 --enable-libvpx --enable-libvorbis
make -j$(nproc) install
