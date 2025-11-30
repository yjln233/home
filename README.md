# home
yjln的所有作品

## 进程枚举示例
使用 `process_list.c` 可以枚举当前系统的进程名和 PID。

编译：

```sh
gcc -Wall -Wextra -Werror -o process_list process_list.c
```

运行：

```sh
./process_list
```

## OCR + 翻译演示
`ocr_translate.c` 提供了一个简易的 OCR/翻译交互程序，支持模拟框选、OCR 文本输入、API Key 校验、翻译（示例为大写转换）、置顶文本预览、锁定防误触以及日志写入（控制台/文件）。

编译：

```sh
gcc -Wall -Wextra -Werror -o ocr_translate ocr_translate.c
```

运行：

```sh
./ocr_translate
```
