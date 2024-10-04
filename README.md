# GV
- **GV** serves as the bridge between multiple engines, meaning that developers who require several engines can implement their algorithms using only **"GV"**

# GV Installation
```bash=
git clone https://github.com/DVLab-NTU/gv.git
cd ./gv
```

## Install the dependencies
- for MacOS :
```bash=
brew tap Homebrew/bundle && brew bundle

export PATH="$(brew --prefix)/bin:$PATH"
```
- for linux (Ubuntu) :
```bash=
sudo apt-get -y install gperf build-essential bison flex libreadline-dev gawk tcl-dev libffi-dev git cmake parallel

sudo apt-get -y install graphviz xdot pkg-config python3 libboost-system-dev libboost-python-dev libboost-filesystem-dev zlib1g-dev libgmp-dev
```

## Compile
```bash=
make
```

- **For using GV tool interface, type after installation:**
```bash=
./gv
```

