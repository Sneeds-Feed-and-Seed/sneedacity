#/bin/bash
sudo apt install pip
pip3 install conan
export PATH="/home/$USER/.local/bin:$PATH"
git clone https://github.com/Sneeds-Feed-and-Seed/sneedacity
mkdir sneed_build
cd sneed_build
cmake -G "Unix Makefiles" -Dsneedacity_use_ffmpeg=loaded ../sneedacity
make -j$(nproc)
sudo make install
