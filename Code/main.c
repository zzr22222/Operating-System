//导入接口函数与工具函数
#include "funcSet.h"
int main()
{
    getDisk();
    createDisk();
    int ret = Shell();
    freeDisk();
    return ret;
}


