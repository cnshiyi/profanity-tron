#ifndef HPP_HELP
#define HPP_HELP

#include <string>

const std::string g_strHelp = R"(
Usage: ./profanity [OPTIONS]

  帮助:
    --help                 显示帮助信息

  运行模式:
    --matching             匹配输入，可以是文件或单个 Tron 地址

  匹配配置:
    --prefix-count         最少前缀匹配位数，默认 0
    --suffix-count         最少后缀匹配位数，默认 6
    --quit-count           命中指定数量后退出，默认 0
    --benchmark-seconds    固定秒数基准测试，不等待命中退出
    --output               将验证通过的命中结果追加写入文件

  私钥范围控制:
    --range-start          起始私钥，64 位十六进制；留空则使用安全随机模式
    --range-end            结束私钥，64 位十六进制；必须和 --range-start 同时填写
    --range-direction      搜索方向，up 或 down；留空默认为 up

    注意：当前范围模式控制私钥前 64 位里连续变化的一段，变化窗口必须结束在第 16 个 hex 位。
    默认不填这些参数时，仍然使用系统安全随机种子。

  设备控制:
    --skip                 按索引跳过指定 GPU 设备
    --cpu-assist           Optional: add low-rate OpenCL CPU assist generation when available; default off
    --cpu-assist-inverse-multiple
                           CPU assist batch size, default 128; lower means lighter CPU participation
    --no-cache             禁用 OpenCL 程序二进制缓存

示例:

  ./profanity --matching profanity.txt
  ./profanity --matching profanity.txt --skip 1
  ./profanity --matching profanity.txt --suffix-count 6
  ./profanity --matching profanity.txt --prefix-count 2 --suffix-count 6 --quit-count 1
  ./profanity --matching TUqEg3dzVEJNQSVW2HY98z5X8SBdhmao8D --prefix-count 1 --suffix-count 1 --benchmark-seconds 10
  ./profanity --matching TUqEg3dzVEJNQSVW2HY98z5X8SBdhmao8D --prefix-count 0 --suffix-count 1 --quit-count 3 --range-start 58958afa84300000e8ce89f9a5b808e8ee611ad00deaf3745a5305e0a3e1d688 --range-end 58958afa844fffffe8ce89f9a5b808e8ee611ad00deaf3745a5305e0a3e1d688 --range-direction up --output hits.txt

说明:

  Profanity Tron 是一个本地 GPU / OpenCL Tron 靓号地址生成工具。
  使用生成结果前，请务必独立验证地址和私钥材料是否一致。
  涉及真实资金时，优先使用多签和独立校验流程。
)";

#endif /* HPP_HELP */
