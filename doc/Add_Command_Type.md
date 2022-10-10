The current version of the GV tool’s mode manager split the commands into two modes which are setup mode and verification mode according to their command types.

Thus, when you create a new command type, please follow the steps below to modify our Mode Manager to decide which mode the new command type should belong.

1. Add the new command type in the src/mod/gvModMgr.h’s enum GVCmdType.

2. Add the new command type in the list of two vectors which are `_vrf` and `_setup` under the constructor of the mode manager.

For example, if you want to add a new command type named NEWTYPE, according to the first step, you need to define it in enum GVCmdType.
```c++
enum GVCmdType{
            GV_CMD_TYPE_COMMON,
            GV_CMD_TYPE_VERIFY,
            …
            GV_CMD_TYPE_NEWTYPE 
       }
```

Next, if you want this command can only execute in verification mode, you need to add it to the _vrf vector under the constructor of the mode manager.

```c++
GVModMgr::GVModMgr(){
    …
    …
    _vrfMode  = {GV_CMD_TYPE_VERIFY, … ,  GV_CMD_TYPE_NEWTYPE}
}
```