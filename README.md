# gv0
Verification research for general RTL/system-level designs (take 0.0)

# GV tool installation
```json=
git clone git@github.com:ric2k1/gv0.git
cd gv0/
# General Users
sudo ./INSTALL.sh 
# On cthulhu
./INSTALL.sh
```

- **For using GV tool interface, type after installation:**
```json=
cd GV/
./gv
```

- **Please notice that, if the shell script has no permission, type:**
```json=
chmod +x <shell_script_filename>
```

# GV tool's third-party tools
- **parser** --> [yosys](https://github.com/YosysHQ/yosys), [berkeley-abc](https://github.com/berkeley-abc/abc), [V3](https://github.com/chengyinwu/V3)
- **file format converter** --> yosys, V3
- **formal verification engine** --> berkeley-abc 
- **simulator** --> yosys
