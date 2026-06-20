# profanity-tron Windows package / Windows 鍙戝竷鍖?
涓枃锛?
鍙屽嚮 `start.exe` 鎵撳紑 Windows 鍥惧舰鍚姩鍣ㄣ€傚懡浠よ鐢熸垚鍣ㄦ枃浠跺悕涓?`shiyi.exe`锛屽惎鍔ㄥ櫒浼氳嚜鍔ㄨ皟鐢ㄥ畠銆?
English:

Run `start.exe` to open the Windows launcher. The native generator is `shiyi.exe` and is launched automatically.

鏂囦欢 / Files:

- `start.exe`: 鍥惧舰鍚姩鍣?/ graphical launcher
- `shiyi.exe`: OpenCL 鐢熸垚鍣?/ OpenCL generator used by the launcher
- `runtime\targets.txt`: 榛樿鐩爣鍒楄〃 / default target list
- `profanity.txt`: 榛樿鐩爣鍒楄〃鍓湰 / copy of the default target list
- `kernels\*.cl`: OpenCL 鍐呮牳婧愮爜 / OpenCL kernels loaded by the generator

鎻愮ず / Notes:

- 鎸囧畾浣嶆暟妯″紡闄愬埗涓?1-16 涓崄鍏繘鍒朵綅锛岀暀绌哄垯浣跨敤闅忔満妯″紡銆?- Fixed-digit mode is limited to 1-16 hex digits. Leave it blank for random mode.
- 褰撳墠鍖呭凡绉婚櫎鏃х殑闈欐€?curl/缃戠粶閾炬帴渚濊禆锛岄伩鍏嶆柊鏋勫缓鐨?`shiyi.exe` 琚?Smart App Control 璇嫤鎴€?- Obsolete static curl/network link dependencies were removed to avoid Smart App Control false positives on the newly built `shiyi.exe`.
