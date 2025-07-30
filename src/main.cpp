#include "pressuredistsolver.hpp"
#include "csv_reader.hpp"
#include <iostream>
#include <vector>
#include <filesystem>
#include <iomanip>

// 特定の時間の圧力値を取得する関数
std::vector<double> extractPressureValues(const CSVReader::CSVData& data, 
                                        const std::vector<double>& time_values) {
    std::vector<double> pressure_values;
    
    for (double t : time_values) {
        // 最も近い時間のデータを取得
        size_t idx = data.findClosestTimeIndex("simulation_time", t);
        const auto& pressure_col = data.getColumn("pressure_ave");
        pressure_values.push_back(pressure_col[idx]);
    }
    
    return pressure_values;
}

// 境界条件を時間ステップごとに設定する関数
void setBoundaryAtTime(SquareThinFilmFDM& solver, double t, 
                      const std::vector<double>& time_values,
                      const std::vector<double>& bottom_pressures,
                      const std::vector<double>& right_pressures,
                      const std::vector<double>& top_pressures,
                      const std::vector<double>& left_pressures) {
    // 最も近い時間インデックスを見つける
    size_t idx = 0;
    double min_diff = std::abs(time_values[0] - t);
    
    for (size_t i = 1; i < time_values.size(); ++i) {
        double diff = std::abs(time_values[i] - t);
        if (diff < min_diff) {
            min_diff = diff;
            idx = i;
        }
    }
    
    // その時間での各辺の圧力を設定
    solver.setEdgeBoundary(
        bottom_pressures[idx],
        right_pressures[idx],
        top_pressures[idx],
        left_pressures[idx]
    );
}

// 時系列での合力を計算する関数
std::vector<double> calculateForceTimeSeries(SquareThinFilmFDM& solver,
                                           const std::vector<double>& time_values,
                                           const std::vector<double>& bottom_pressures,
                                           const std::vector<double>& right_pressures,
                                           const std::vector<double>& top_pressures,
                                           const std::vector<double>& left_pressures) {
    std::vector<double> forces;
    
    for (size_t i = 0; i < time_values.size(); ++i) {
        // 境界条件を設定
        setBoundaryAtTime(solver, time_values[i], time_values, 
                         bottom_pressures, right_pressures, 
                         top_pressures, left_pressures);
        
        // 圧力分布を解く
        if (!solver.solveDirect()) {
            std::cerr << "Failed to solve at time step " << i << std::endl;
            forces.push_back(0.0);
            continue;
        }
        
        // 合力を計算して記録
        double force = solver.calculateTotalForce();
        forces.push_back(force);
        
        // 進捗表示
        if ((i + 1) % 10 == 0 || i == time_values.size() - 1) {
            std::cout << "計算進捗: " << (i + 1) << "/" << time_values.size() << " 完了" << std::endl;
        }
    }
    
    return forces;
}

int main() {
    try {
        CSVReader reader;
        
        // CSVファイルを読み込み
        std::cout << "CSVファイルを読み込み中..." << std::endl;
        auto bottom_pressure = reader.readCSV("bottompressure.csv");
        auto left_pressure = reader.readCSV("leftpressure.csv");
        auto right_pressure = reader.readCSV("rightpressure.csv");
        auto top_pressure = reader.readCSV("toppressure.csv");
        
        // ソルバーを初期化
        SquareThinFilmFDM solver(100, 0.1, 0.13, nullptr, 0.01, 1.0);
        
        // 共通の時間値を取得（bottompressureファイルから）
        auto time_values = bottom_pressure.getUniqueValues("simulation_time");
        
        // 必要に応じて時間の範囲を制限（計算時間短縮のため）
        if (time_values.size() > 200) {
            time_values.resize(200);
        }
        
        std::cout << "時間ステップ数: " << time_values.size() << std::endl;
        
        // 各辺の圧力値を取得
        auto bottom_pressures = extractPressureValues(bottom_pressure, time_values);
        auto right_pressures = extractPressureValues(right_pressure, time_values);
        auto top_pressures = extractPressureValues(top_pressure, time_values);
        auto left_pressures = extractPressureValues(left_pressure, time_values);
        
        // 出力ディレクトリの作成
        std::filesystem::create_directories("results");
        
        // 合力の時系列計算
        std::cout << "calculating forces..." << std::endl;
        auto forces = calculateForceTimeSeries(
            solver,
            time_values,
            bottom_pressures,
            right_pressures,
            top_pressures,
            left_pressures
        );
        
        // 結果をCSVファイルに保存
        CSVReader::CSVData result_data;
        result_data.headers = {"time", "force", "bottom_pressure", "right_pressure", 
                             "top_pressure", "left_pressure"};
        result_data.num_rows = time_values.size();
        
        // データを列に格納
        result_data.columns["time"] = time_values;
        result_data.columns["force"] = forces;
        result_data.columns["bottom_pressure"] = bottom_pressures;
        result_data.columns["right_pressure"] = right_pressures;
        result_data.columns["top_pressure"] = top_pressures;
        result_data.columns["left_pressure"] = left_pressures;
        
        reader.writeCSV("results/pressure_force_results.csv", result_data);
        
        std::cout << "すべての処理が完了しました。結果は results ディレクトリに保存されています。" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "エラー: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}