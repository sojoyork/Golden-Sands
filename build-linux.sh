# uncomment this line if you REALLY need to update
# sudo apt update
sudo apt install libncurses5-dev libncursesw5-dev
g++ src/main -o src/golden-sands -lncurses
echo "compiling done!!!! It is in the src folder!!"
cd src
./golden-sands
