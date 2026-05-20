#ifndef HPP_HELP
#define HPP_HELP

#include <string>

const std::string g_strHelp = R"(
Usage: ./profanity [OPTIONS]

  帮助:
    --help              显示帮助信息

  运行模式:
    --matching          匹配输入，可以是文件或单个地址

  匹配配置:
    --prefix-count      最少前缀匹配位数，默认 0
    --suffix-count      最少后缀匹配位数，默认 6
    --quit-count        当生成结果数量达到条件时退出，默认 0

  设备控制:
    --skip              按索引跳过指定 GPU 设备

示例:

  ./profanity --matching profanity.txt
  ./profanity --matching profanity.txt --skip 1
  ./profanity --matching profanity.txt --suffix-count 6
  ./profanity --matching profanity.txt --prefix-count 2 --suffix-count 6 --quit-count 1
  ./profanity --matching TUqEg3dzVEJNQSVW2HY98z5X8SBdhmao8D --prefix-count 2 --suffix-count 4 --quit-count 1

说明:

  Profanity 是一个 Tron 靓号地址生成工具: https://tron.network/
  本项目基于 ethereum profanity 修改而来: https://github.com/johguse/profanity
  请尽量只运行你能审计来源的源码构建版本
  Author: telegram -> @jackslowfak

安全提示:

  在使用生成出的地址前，请独立验证密钥材料。
  涉及真实资产时，请优先使用多签并做好独立校验。
)";

#endif /* HPP_HELP */
