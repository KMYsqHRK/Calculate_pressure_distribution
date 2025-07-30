#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

class CSVReader {
public:
    struct CSVData {
        std::vector<std::string> headers;
        std::map<std::string, std::vector<double>> columns;
        size_t num_rows;
        
        CSVData() : num_rows(0) {}
        
        // 特定の列のデータを取得
        const std::vector<double>& getColumn(const std::string& column_name) const {
            auto it = columns.find(column_name);
            if (it == columns.end()) {
                throw std::runtime_error("Column '" + column_name + "' not found");
            }
            return it->second;
        }
        
        // 列の存在確認
        bool hasColumn(const std::string& column_name) const {
            return columns.find(column_name) != columns.end();
        }
        
        // ユニークな値を取得
        std::vector<double> getUniqueValues(const std::string& column_name) const {
            const auto& col = getColumn(column_name);
            std::vector<double> unique_vals;
            
            for (double val : col) {
                if (std::find(unique_vals.begin(), unique_vals.end(), val) == unique_vals.end()) {
                    unique_vals.push_back(val);
                }
            }
            
            return unique_vals;
        }
        
        // 最も近い時間のインデックスを取得
        size_t findClosestTimeIndex(const std::string& time_column, double target_time) const {
            const auto& time_col = getColumn(time_column);
            size_t best_idx = 0;
            double min_diff = std::abs(time_col[0] - target_time);
            
            for (size_t i = 1; i < time_col.size(); ++i) {
                double diff = std::abs(time_col[i] - target_time);
                if (diff < min_diff) {
                    min_diff = diff;
                    best_idx = i;
                }
            }
            
            return best_idx;
        }
    };

private:
    char delimiter;

public:
    explicit CSVReader(char delim = ',') : delimiter(delim) {}

    /**
     * CSVファイルを読み込む
     * @param filename ファイル名
     * @return CSVデータ
     */
    CSVData readCSV(const std::string& filename) const;

    /**
     * CSVデータを書き込む
     * @param filename ファイル名
     * @param data 書き込むデータ
     */
    void writeCSV(const std::string& filename, const CSVData& data) const;

private:
    std::vector<std::string> splitLine(const std::string& line) const;
    bool isNumber(const std::string& str) const;
    double parseNumber(const std::string& str) const;
};