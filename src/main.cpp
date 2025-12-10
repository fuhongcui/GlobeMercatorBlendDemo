/**
 * Globe-Mercator 过渡测试 (参考 maplibre-gl-js)
 * 
 * 演示 maplibre 风格的 Globe 和 Mercator 平滑过渡
 * 
 * 关键实现点：
 * 1. 双矩阵系统：Globe 和 Mercator 投影矩阵在裁剪空间混合
 * 2. 动态 Wrap 选择：为每个 tile 选择最接近 center 的 wrap 值
 * 3. Z 值延迟混合：前 80% 使用平面 Z，后 20% 使用 Globe 裁剪 Z
 * 4. 背面裁剪平面：Globe 模式下隐藏背面几何体
 */

#include "Application.h"
int main()
{
    Application app;
    app.run();
    return 0;
}
