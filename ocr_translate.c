#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_TEXT 1024
#define API_KEY "DEMO_KEY"

typedef struct {
    int x;
    int y;
    int width;
    int height;
    int locked;
    char text[MAX_TEXT];
} Overlay;

static int console_logging_enabled = 1;
static int file_logging_enabled = 1;
static const char *program_log_path = "program.log";
static const char *translation_log_path = "translations.log";

static void timestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

static void log_program(const char *level, const char *message) {
    char time_buf[32];
    timestamp(time_buf, sizeof(time_buf));

    if (console_logging_enabled) {
        printf("[%s] %s: %s\n", time_buf, level, message);
    }

    if (file_logging_enabled) {
        FILE *f = fopen(program_log_path, "a");
        if (f) {
            fprintf(f, "[%s] %s: %s\n", time_buf, level, message);
            fclose(f);
        }
    }
}

static void log_translation(const char *source, const char *translated) {
    char time_buf[32];
    timestamp(time_buf, sizeof(time_buf));
    FILE *f = fopen(translation_log_path, "a");
    if (f) {
        fprintf(f, "[%s] SRC: %s\n[%s] TRN: %s\n---\n", time_buf, source, time_buf, translated);
        fclose(f);
    }
}

static void print_overlay(const Overlay *overlay) {
    int width = overlay->width > 0 ? overlay->width : 40;
    printf("\n=== 透明置顶文本 (位置 %d,%d 尺寸 %dx%d) ===\n", overlay->x, overlay->y, overlay->width, overlay->height);

    const char *text = overlay->text;
    size_t len = strlen(text);
    int col = 0;

    for (size_t i = 0; i < len; ++i) {
        putchar(text[i]);
        col++;
        if (col >= width && text[i] == ' ') {
            putchar('\n');
            col = 0;
        }
    }
    putchar('\n');
    printf("锁定: %s\n", overlay->locked ? "是" : "否");
    printf("=======================================\n\n");
}

static int verify_api_key(const char *input_key) {
    if (strcmp(input_key, API_KEY) == 0) {
        log_program("INFO", "API Key 验证成功");
        return 1;
    }
    log_program("ERROR", "API Key 验证失败");
    return 0;
}

static void translate_text(Overlay *overlay) {
    char key[64];
    printf("输入翻译 API Key (示例: %s): ", API_KEY);
    if (!fgets(key, sizeof(key), stdin)) {
        return;
    }
    key[strcspn(key, "\n")] = '\0';
    if (!verify_api_key(key)) {
        return;
    }

    char source[MAX_TEXT];
    if (strlen(overlay->text) == 0) {
        log_program("WARN", "没有待翻译的文本");
        return;
    }
    strncpy(source, overlay->text, sizeof(source));
    source[sizeof(source) - 1] = '\0';

    char translated[MAX_TEXT];
    size_t len = strlen(source);
    for (size_t i = 0; i < len; ++i) {
        translated[i] = (char)toupper((unsigned char)source[i]);
    }
    translated[len] = '\0';
    strncpy(overlay->text, translated, sizeof(overlay->text));
    overlay->text[sizeof(overlay->text) - 1] = '\0';

    log_program("INFO", "完成翻译并刷新置顶文本");
    log_translation(source, translated);
}

static void update_box(Overlay *overlay) {
    if (overlay->locked) {
        log_program("WARN", "当前文本已锁定，无法调整范围");
        return;
    }

    printf("输入框选区域左上角坐标 X Y: ");
    if (scanf("%d %d", &overlay->x, &overlay->y) != 2) {
        log_program("ERROR", "坐标输入无效");
        while (getchar() != '\n');
        return;
    }
    printf("输入宽度和高度: ");
    if (scanf("%d %d", &overlay->width, &overlay->height) != 2) {
        log_program("ERROR", "尺寸输入无效");
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');
    log_program("INFO", "更新框选区域");
}

static void move_overlay(Overlay *overlay) {
    if (overlay->locked) {
        log_program("WARN", "当前文本已锁定，无法移动");
        return;
    }
    int dx, dy;
    printf("输入移动偏移 dx dy: ");
    if (scanf("%d %d", &dx, &dy) != 2) {
        log_program("ERROR", "偏移输入无效");
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');
    overlay->x += dx;
    overlay->y += dy;
    log_program("INFO", "已移动置顶框");
}

static void mock_ocr(Overlay *overlay) {
    if (overlay->locked) {
        log_program("WARN", "当前文本已锁定，无法更新 OCR 内容");
        return;
    }

    printf("模拟 OCR: 输入要识别的文本 (行尾回车结束):\n");
    if (!fgets(overlay->text, sizeof(overlay->text), stdin)) {
        log_program("ERROR", "读取 OCR 文本失败");
        return;
    }
    overlay->text[strcspn(overlay->text, "\n")] = '\0';
    log_program("INFO", "已更新 OCR 文本并刷新置顶框");
}

static void toggle_logging(void) {
    int choice;
    printf("1. 切换控制台日志 (当前 %s)\n", console_logging_enabled ? "开启" : "关闭");
    printf("2. 切换文件日志 (当前 %s)\n", file_logging_enabled ? "开启" : "关闭");
    printf("选择: ");
    if (scanf("%d", &choice) != 1) {
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');
    if (choice == 1) {
        console_logging_enabled = !console_logging_enabled;
        log_program("INFO", console_logging_enabled ? "控制台日志已开启" : "控制台日志已关闭");
    } else if (choice == 2) {
        file_logging_enabled = !file_logging_enabled;
        log_program("INFO", file_logging_enabled ? "文件日志已开启" : "文件日志已关闭");
    }
}

static void lock_overlay(Overlay *overlay) {
    overlay->locked = !overlay->locked;
    log_program("INFO", overlay->locked ? "文本已锁定，防止误触" : "文本已解锁，可编辑");
}

int main(void) {
    Overlay overlay = { .x = 100, .y = 100, .width = 400, .height = 200, .locked = 0 };
    overlay.text[0] = '\0';
    log_program("INFO", "OCR+翻译演示程序启动");

    int running = 1;
    while (running) {
        printf("\n==== OCR/翻译 GUI 演示 ====\n");
        printf("\n1. 自定义框选范围 (模拟 GUI 框选)\n");
        printf("2. 模拟 OCR 识别文本\n");
        printf("3. 翻译并刷新置顶文本\n");
        printf("4. 显示置顶文本预览\n");
        printf("5. 移动置顶文本位置\n");
        printf("6. 锁定/解锁置顶文本\n");
        printf("7. 日志设置\n");
        printf("0. 退出程序\n");
        printf("选择: ");

        int choice = -1;
        if (scanf("%d", &choice) != 1) {
            log_program("ERROR", "菜单输入无效");
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n');

        switch (choice) {
            case 1:
                update_box(&overlay);
                break;
            case 2:
                mock_ocr(&overlay);
                break;
            case 3:
                translate_text(&overlay);
                break;
            case 4:
                print_overlay(&overlay);
                break;
            case 5:
                move_overlay(&overlay);
                break;
            case 6:
                lock_overlay(&overlay);
                break;
            case 7:
                toggle_logging();
                break;
            case 0:
                running = 0;
                log_program("INFO", "退出程序");
                break;
            default:
                log_program("WARN", "未知的菜单选项");
                break;
        }
    }
    return 0;
}

