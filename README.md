# gv0
Verification research for general RTL/system-level designs (take 0.0)

# GV tool installation
- **For GV tool installation, type:**
    - (**For general users**) In current version, GV needs to compile our program with “yosys-config” (under /usr/local/bin) and dynamically link the library (libyosys.so, under /usr/local/lib/yosys) during the execution process, which means we need to use "sudo GV_INSTALL_NEW.sh” to install “libyosys.so” and “yosys-config” under the default path. 
    - (**For cthulhu users**) Currently, we have installed these two files so you just need to type “GV_INSTALL_NEW.sh” without “sudo” and just ignore the error messages during the building process.
  
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
