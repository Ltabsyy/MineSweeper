# MSR EVF 说明

该文件夹内容仅供测试evf扫雷录像格式。

使用`EVF Reader`读取evf，检查文件内容。

使用`MSR Operations Transcoder`将MSR扫雷操作记录文件转码到evf。

使用[evf v4](https://github.com/eee555/ms-toollib/blob/main/evf%E6%A0%87%E5%87%86.md)格式，无校验码，转换时自设用户标识，不具备安全性。

可在[元扫雷](https://github.com/eee555/Metasweeper)、在线解析网站播放，未来或可被[开源扫雷网](https://openms.top/#/)、[中国扫雷网](https://saolei.wang/)等网站的开放录像板块收录。

已通过播放测试：[元扫雷3.3.4](https://github.com/eee555/Metasweeper/releases/tag/3.3.4)，[strange-dust.github.io](https://strange-dust.github.io/minesweeper-replay-analyzer/)

不建议丢弃源MSR扫雷操作记录文件，未来更新不对已输出的evf负责，不会将evf逆转回txt。