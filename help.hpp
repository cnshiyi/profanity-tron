#ifndef HPP_HELP
#define HPP_HELP

#include <string>

const std::string g_strHelp = R"(
Usage: ./profanity [OPTIONS]

  Help:
    --help                 Show help information

  Modes:
    --matching             Matching input, file path or single Tron address

  Matching:
    --prefix-count         Minimum prefix match length, default 0
    --suffix-count         Minimum suffix match length, default 6
    --quit-count           Exit after this many accepted hits, default 0
    --benchmark-seconds    Run a fixed-duration benchmark instead of waiting for hits
    --output               Append validated hits to a result file

  Private-key range:
    --range-start          Start private key, up to 64 hex chars
    --range-end            End private key, up to 64 hex chars
    --range-direction      up or down, default up

    Range mode controls one continuous 1-16 hex-char window inside a single
    16-char private-key block. The window must end at the right edge of that
    block, for example the last 16 hex chars. Leave range arguments empty for
    secure random mode.

  Device:
    --skip                 Skip GPU device by index
    --inverse-multiple     GPU batch multiplier; leave empty for automatic tuning
                           Manual values are checked against a 60% GPU memory budget
    --cpu-assist           Optional low-rate OpenCL CPU assist, default off
    --cpu-assist-inverse-multiple
                           CPU assist batch size, default 128
    --no-cache             Disable OpenCL program binary cache

Examples:

  ./profanity --matching profanity.txt
  ./profanity --matching profanity.txt --suffix-count 6
  ./profanity --matching TUqEg3dzVEJNQSVW2HY98z5X8SBdhmao8D --prefix-count 1 --suffix-count 1 --benchmark-seconds 10
  ./profanity --matching TUqEg3dzVEJNQSVW2HY98z5X8SBdhmao8D --prefix-count 0 --suffix-count 1 --quit-count 3 --range-start 58958afa84300000e8ce89f9a5b808e8ee611ad00deaf3745a5305e0a3e1d688 --range-end 58958afa844fffffe8ce89f9a5b808e8ee611ad00deaf3745a5305e0a3e1d688 --range-direction up --output hits.txt

Notes:

  Always independently verify that a generated address matches the printed private key.
)";

#endif /* HPP_HELP */
