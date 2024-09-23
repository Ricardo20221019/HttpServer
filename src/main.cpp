#include "myserver.h"
#include "customConfig.h"

int main(int argc, char *argv[])
{
    // 配置服务端
    std::shared_ptr<CustomConfig> customConfig = std::make_shared<CustomConfig>();
    bool is_config = customConfig.get()->initConfig();
    LOG(outHead("info") + "服务器初始化配置数据完毕！", true, MAIN_REACTOR);
    if (!is_config)
    {
        initServerFile(customConfig);
        is_config = customConfig.get()->initConfig();
        if (!is_config)
        {
            LOG(outHead("error") + "服务初始化失败，请尝试删除 ini 文件夹并重启服务！", false, MAIN_REACTOR);
        }
    }
    // 根据端口号和线程数构造http_server
    MyServer http_server(customConfig.get()->serverInfo.port, 8, customConfig);
    http_server.start();
    return 0;
}