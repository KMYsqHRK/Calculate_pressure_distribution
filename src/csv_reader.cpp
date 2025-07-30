#include "csv_reader.hpp"
#include <algorithm>
#include <cctype>

CSVReader::CSVData CSVReader::readCSV(const std::string& filename) const {
    CSVData data;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    
    std::string line;
    bool first_line = true;
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        std::vector<std::string> tokens = splitLine(line);
        
        if (first_line) {
            // ヘッダー行の処理
            data.headers = tokens;
            for (const auto& header : data.headers) {
                data.columns[header] = std::vector<double>();
            }
            first_line = false;
        } else {
            // データ行の処理
            if (tokens.size() != data.headers.size()) {
                std::cerr << "Warning: Row has different number of columns than header" << std::endl;
                continue;
            }
            
            for (size_t i = 0; i < tokens.size(); ++i) {
                if (isNumber(tokens[i])) {
                    data.columns[data.headers[i]].push_back(parseNumber(tokens[i]));
                } else {
                    // 数値でない場合は0として扱う（または例外を投げる）
                    data.columns[data.headers[i]].push_back(0.0);
                }
            }
            data.num_rows++;
        }
    }
    
    file.close();
    return data;
}

void CSVReader::writeCSV(const std::string& filename, const CSVData& data) const {
    std::ofstream file(filename);
    
    if (!file.is_open()) {
        throw std::runtime_error("Cannot create file: " + filename);
    }
    
    // ヘッダーを書き込み
    for (size_t i = 0; i < data.headers.size(); ++i) {
        file << data.headers[i];
        if (i < data.headers.size() - 1) {
            file << delimiter;
        }
    }
    file << std::endl;
    
    // データを書き込み
    for (size_t row = 0; row < data.num_rows; ++row) {
        for (size_t col = 0; col < data.headers.size(); ++col) {
            const auto& column = data.columns.at(data.headers[col]);
            if (row < column.size()) {
                file << column[row];
            }
            if (col < data.headers.size() - 1) {
                file << delimiter;
            }
        }
        file << std::endl;
    }
    
    file.close();
}

std::vector<std::string> CSVReader::splitLine(const std::string& line) const {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        // 前後の空白を削除
        token.erase(token.begin(), std::find_if(token.begin(), token.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
        token.erase(std::find_if(token.rbegin(), token.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), token.end());
        
        tokens.push_back(token);
    }
    
    return tokens;
}

bool CSVReader::isNumber(const std::string& str) const {
    if (str.empty()) return false;
    
    size_t start = 0;
    if (str[0] == '-' || str[0] == '+') {
        start = 1;
    }
    
    bool has_decimal = false;
    bool has_digit = false;
    
    for (size_t i = start; i < str.length(); ++i) {
        if (std::isdigit(str[i])) {
            has_digit = true;
        } else if (str[i] == '.' && !has_decimal) {
            has_decimal = true;
        } else if (str[i] == 'e' || str[i] == 'E') {
            // 科学的記法をサポート
            if (i == str.length() - 1) return false;
            i++;
            if (str[i] == '+' || str[i] == '-') i++;
            if (i >= str.length()) return false;
            for (; i < str.length(); ++i) {
                if (!std::isdigit(str[i])) return false;
            }
            break;
        } else {
            return false;
        }
    }
    
    return has_digit;
}

double CSVReader::parseNumber(const std::string& str) const {
    try {
        return std::stod(str);
    } catch (const std::exception&) {
        return 0.0;
    }
}