#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <math.h>
#include <Windows.h>
#include "hpdf.h"

#pragma execution_character_set("utf-8")  

//每个字符的宽度设置
#define page_num 328  //输出的页面数量 和有效的Unicode编码数量有关 该字体集不超过65533个 页码不超过328 
#define char_width 20  ////每个字符的宽度设置
#define line_num 20 //每页输出的行数
#define line_char_num 10 //每行的字符写入的数量
unsigned int unicode = 0x0020;//Unicode编码值，从0x0020开始(空格） 输出的方框表示该Unicode码在该字体中没有对应的utf8编码

void error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void* user_data)
{
    printf("ERROR: error_no=%04X, detail_no=%u\n", (unsigned int)error_no, (unsigned int)detail_no);
}

void unicode_to_utf8(unsigned int unicode);

char* utf8=nullptr;

int main() {
    SetConsoleOutputCP(CP_UTF8);
    // 创建一个新的PDF文档
    HPDF_Doc pdf = HPDF_New(error_handler, nullptr);
    if (!pdf) {
        std::cerr << "Error: Unable to create PDF document." << std::endl;
        return -1;
    }

    // 设置utf8编码
    HPDF_UseUTFEncodings(pdf);
    HPDF_SetCurrentEncoder(pdf, "UTF-8");

    const char* fontName = nullptr;
    fontName = HPDF_LoadTTFontFromFile(pdf, "Arial_Unicode_MS.ttf", HPDF_TRUE);

    if (fontName == nullptr) {
        HPDF_Free(pdf);
        std::cerr << "Error: Unable to load .ttf file." << std::endl;//字体不支持 或是没设置utf编码等
        return -1;
    }
    //std::cout << fontName << std::endl;

    HPDF_Font font = HPDF_GetFont(pdf, fontName, "UTF-8");

    utf8 = (char*)malloc(5 * sizeof(char)); // UTF-8 最多需要 4 个字节，再加上字符串结尾 '\0'


    //写入328页 arial unicode ms字符实际有效编码不超过65533个 200个一页，328页足够
    for (int i = 0; i < page_num; i++) {
        // 添加一页
        HPDF_Page page = HPDF_AddPage(pdf);

        // 设置页面尺寸
        HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);

        //设置字体大小
        HPDF_REAL font_size = 20.0;
        HPDF_Page_SetFontAndSize(page, font, font_size);

        HPDF_REAL line_height = font_size + 5;//增加一些行间距

        //每一页刚开始的写入位置
        HPDF_REAL x0 = (HPDF_Page_GetWidth(page)) / 2 - char_width * 5;
        HPDF_REAL y0 = (HPDF_Page_GetHeight(page)) / 2 + line_height * 10;

        HPDF_REAL y = y0;

        //每页写20行
        for (int i = 0; i < line_num; i++) {

            HPDF_REAL x = x0;

            //每行写10个字符
            for (int j = 0; j < line_char_num; j++) {

                //先将unicode值转为对应的utf-8
                unicode_to_utf8(unicode);

                HPDF_Page_BeginText(page);
                x += char_width;
                HPDF_Page_MoveTextPos(page, x, y);
                HPDF_Page_ShowText(page, utf8);
                HPDF_Page_EndText(page);

                unicode++;
            }

            y = y - line_height;//写完10个字符，下移一行
        }
    }

    // 保存PDF文件
    const char* filename = "results.pdf";
    if (HPDF_SaveToFile(pdf, filename) != HPDF_OK) {
        std::cerr << "Error: Unable to save PDF file." << std::endl;
        HPDF_Free(pdf);
        return -1;
    }
    // 清理资源
    HPDF_Free(pdf);
    free(utf8);

    std::cout << "PDF file created successfully: " << filename << std::endl;
    return 0;
}



//Unicode码转为utf-8码
void unicode_to_utf8(unsigned int unicode) {
    memset(utf8, 0, 5);

    if (utf8 == nullptr) {
        perror("Memory allocation failed");
        exit(1);
    }

    if (unicode <= 0x7F) { // ASCII 字符（单字节）
        utf8[0] = unicode & 0x7F;
    }
    else if (unicode <= 0x7FF) { // 2 字节编码
        utf8[0] = 0xC0 | (unicode >> 6);
        utf8[1] = 0x80 | (unicode & 0x3F);
    }
    else if (unicode <= 0xFFFF) { // 3 字节编码
        utf8[0] = 0xE0 | (unicode >> 12);
        utf8[1] = 0x80 | ((unicode >> 6) & 0x3F);
        utf8[2] = 0x80 | (unicode & 0x3F);
    }
    else if (unicode <= 0x10FFFF) { // 4 字节编码
        utf8[0] = 0xF0 | (unicode >> 18);
        utf8[1] = 0x80 | ((unicode >> 12) & 0x3F);
        utf8[2] = 0x80 | ((unicode >> 6) & 0x3F);
        utf8[3] = 0x80 | (unicode & 0x3F);
    }
    else {
        // 处理无效的 Unicode 编码
        fprintf(stderr, "Invalid Unicode code: %u\n", unicode);
        free(utf8);
    }

    return;
}
