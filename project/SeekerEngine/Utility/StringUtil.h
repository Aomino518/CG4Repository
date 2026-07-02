#pragma once
#include <string>
#include <vector>

std::wstring ConvertString(const std::string& str);

std::string ConvertString(const std::wstring& str);

/// <summary>
/// 文字列を分離する関数
/// </summary>
/// <param name="str">文字列</param>
/// <param name="del">区切る対象文字</param>
/// <returns>分離した文字列</returns>
std::vector<std::string> Split(std::string str, char del);