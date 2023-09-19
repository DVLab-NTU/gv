# How to develop on GV 
## Add a new GV command type 
The current version of the GV tool’s mode manager splits the commands into two modes which are setup mode (setup) and verification mode (vrf) according to their command types.

Thus, when you create a new command type, please follow the steps below to modify our command manager (src/cmd/gvCmdMgr.h) to decide which mode the new command type should belong.

1. Add the new command type in the src/mod/gvModMgr.h’s enum GVCmdType.

2. Add the new command type in the list of two vectors which are `_vrf` and `_setup` under the constructor of the mode manager.

For example, if you want to add a new command type named NEWTYPE, according to the first step, you need to define it in enum GVCmdType.
```c++
enum GVCmdType{
            GV_CMD_TYPE_REVEALED = 0,
            GV_CMD_TYPE_COMMON   = 1,
            GV_CMD_TYPE_VERIFY   = 2,
            …
            GV_CMD_TYPE_NEWTYPE 
       };
```

Next, if you want this command can only execute in verification mode, you need to add it to the "unordered_set<GVCmdType> _vrfMode".

```c++
unordered_set<GVCmdType> _vrfMode  = {GV_CMD_TYPE_VERIFY, … ,  GV_CMD_TYPE_NEWTYPE};
```

## Add a new GV command 
1. Select your directory under "src" 
- e.g. If a user would like to define a **"read design"** command, and since the command is related to "network", the user defines the command in **"src/ntk"** directory 

2. Define your command name in the **"cmd.h"** file under your selected directory 
- e.g. To define **"read design"** command under **"src/ntk"** directory, first open **"src/ntk/gvNtkCmd.h"** and add **"GV_COMMAND(<cmd_class_name>, <cmd_type>)"** 
- e.g. GV_COMMAND(GVReadDesignCmd, GV_CMD_TYPE_NETWORK) 

3. **(OPTIONAL)** If needs to new a directory, one should add the new command into the initialization function in **"src/main/main.cpp"** for recognition 

4. Define exec(), usage(), help() overloads under **"cmd.c"** file 
- e.g. define the "read design" command in **src/ntk/gvNtkCmd.cpp** 

5. If one adds a new header file under the directory (e.g. src/ntk), add the header file name into the **Makefile** in the same dirctory (e.g. src/ntk)