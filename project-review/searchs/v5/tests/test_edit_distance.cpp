/**
 * 最小编辑距离单元测试
 * 测试编辑距离算法的正确性
 */

#include <iostream>
#include <string>
#include <cassert>
#include "ProcessFile.h"

using namespace std;

void test_edit_distance() {
    TextProcessor processor("", "", "", "", "", "");

    // 测试用例1: 相同字符串
    assert(processor.editDistance("abc", "abc") == 0);
    cout << "✓ Test 1 passed: same string distance 0" << endl;

    // 测试用例2: 一个字符不同
    assert(processor.editDistance("abc", "adc") == 1);
    cout << "✓ Test 2 passed: one char different distance 1" << endl;

    // 测试用例3: kitten -> sitting 经典例子
    assert(processor.editDistance("kitten", "sitting") == 3);
    cout << "✓ Test 3 passed: kitten->sitting distance 3" << endl;

    // 测试用例4: 空字符串
    assert(processor.editDistance("", "abc") == 3);
    assert(processor.editDistance("abc", "") == 3);
    cout << "✓ Test 4 passed: empty string distance correct" << endl;

    // 测试用例5: 完全不同
    assert(processor.editDistance("abc", "def") == 3);
    cout << "✓ Test 5 passed: completely different distance 3" << endl;

    // 测试用例6: 中文测试
    assert(processor.editDistance("搜索引擎", "搜索") == 2);
    cout << "✓ Test 6 passed: Chinese distance correct" << endl;

    // 测试用例7: 中英混合
    assert(processor.editDistance("搜索engine", "搜索Engine") == 1);
    cout << "✓ Test 7 passed: mixed Chinese-English distance correct" << endl;

    cout << endl << "All edit distance tests passed! 🎉" << endl;
}

int main() {
    test_edit_distance();
    return 0;
}
