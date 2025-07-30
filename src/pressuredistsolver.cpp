#include "pressuredistsolver.hpp"
#include <iostream>
#include <cmath>

SquareThinFilmFDM::SquareThinFilmFDM(int n, double side_width, double side_height,
                                   HeightFunction h_func, double viscosity, double velocity)
    : n(n), width(side_width), height(side_height),
      viscosity(viscosity), velocity(velocity) {
    
    // 格子間隔
    dx = side_width / (n - 1);
    dy = side_height / (n - 1);
    
    // メッシュの初期化
    initializeMesh();
    
    // 圧力場の初期化
    P = Matrix::Zero(n, n);
    
    // 膜厚の初期化
    initializeHeight(h_func);
}

void SquareThinFilmFDM::initializeMesh() {
    // 座標ベクトルの生成
    x = Vector::LinSpaced(n, 0.0, width);
    y = Vector::LinSpaced(n, 0.0, height);
    
    // メッシュグリッドの生成
    X = Matrix(n, n);
    Y = Matrix(n, n);
    
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            X(i, j) = x(j);
            Y(i, j) = y(i);
        }
    }
}

void SquareThinFilmFDM::initializeHeight(HeightFunction h_func) {
    h = Matrix(n, n);
    
    if (h_func == nullptr) {
        // デフォルトは一様膜厚 (1mm)
        h.setConstant(0.001);
    } else {
        // 指定された関数で膜厚を計算
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                h(i, j) = h_func(x(j), y(i));
            }
        }
    }
}

void SquareThinFilmFDM::setEdgeBoundary(double p_bottom, double p_right, 
                                       double p_top, double p_left) {
    // 各辺に異なる圧力を設定
    P.row(0).setConstant(p_bottom);      // 下辺
    P.row(n-1).setConstant(p_top);       // 上辺
    P.col(0).setConstant(p_left);        // 左辺
    P.col(n-1).setConstant(p_right);     // 右辺
    
    // 角の処理（平均値を使用）
    P(0, 0) = (p_bottom + p_left) / 2.0;     // 左下
    P(0, n-1) = (p_bottom + p_right) / 2.0;  // 右下
    P(n-1, 0) = (p_top + p_left) / 2.0;      // 左上
    P(n-1, n-1) = (p_top + p_right) / 2.0;   // 右上
}

bool SquareThinFilmFDM::solveDirect() {
    // 係数の計算
    Matrix h3_12mu = h.array().pow(3) / (12.0 * viscosity);
    
    // 膜厚勾配の計算（中心差分）
    Matrix dhdx = Matrix::Zero(n, n);
    for (int i = 0; i < n; ++i) {
        for (int j = 1; j < n-1; ++j) {
            dhdx(i, j) = (h(i, j+1) - h(i, j-1)) / (2.0 * dx);
        }
    }
    
    // 内部点のみを扱う
    int inner_n = n - 2;
    int n_unknowns = inner_n * inner_n;
    
    // スパース行列とベクトルの準備
    std::vector<Eigen::Triplet<double>> triplets;
    Vector b = Vector::Zero(n_unknowns);
    
    // 方程式の構築
    int idx = 0;
    for (int i = 1; i < n - 1; ++i) {
        for (int j = 1; j < n - 1; ++j) {
            // 節点の平均膜厚係数
            double h3_e = 0.5 * (h3_12mu(i, j) + h3_12mu(i, j + 1));
            double h3_w = 0.5 * (h3_12mu(i, j) + h3_12mu(i, j - 1));
            double h3_n = 0.5 * (h3_12mu(i, j) + h3_12mu(i + 1, j));
            double h3_s = 0.5 * (h3_12mu(i, j) + h3_12mu(i - 1, j));
            
            // 中心差分の係数
            double coef_e = h3_e / (dx * dx);
            double coef_w = h3_w / (dx * dx);
            double coef_n = h3_n / (dy * dy);
            double coef_s = h3_s / (dy * dy);
            
            // メインの対角成分
            double main_coef = -(coef_e + coef_w + coef_n + coef_s);
            triplets.emplace_back(idx, idx, main_coef);
            
            // 隣接点への係数
            if (j < n - 2) {  // 東
                triplets.emplace_back(idx, idx + 1, coef_e);
            }
            if (j > 1) {  // 西
                triplets.emplace_back(idx, idx - 1, coef_w);
            }
            if (i < n - 2) {  // 北
                triplets.emplace_back(idx, idx + inner_n, coef_n);
            }
            if (i > 1) {  // 南
                triplets.emplace_back(idx, idx - inner_n, coef_s);
            }
            
            // 右辺ベクトル
            // すべり速度による項
            b(idx) = -6.0 * velocity * viscosity * dhdx(i, j);
            
            // 境界条件の寄与
            if (j == 1) {  // 左端に隣接
                b(idx) -= coef_w * P(i, 0);
            }
            if (j == n - 2) {  // 右端に隣接
                b(idx) -= coef_e * P(i, n-1);
            }
            if (i == 1) {  // 下端に隣接
                b(idx) -= coef_s * P(0, j);
            }
            if (i == n - 2) {  // 上端に隣接
                b(idx) -= coef_n * P(n-1, j);
            }
            
            idx++;
        }
    }
    
    // スパース行列の構築
    SparseMatrix A(n_unknowns, n_unknowns);
    A.setFromTriplets(triplets.begin(), triplets.end());
    
    // 線形方程式を解く
    Eigen::SparseLU<SparseMatrix> solver;
    solver.compute(A);
    
    if (solver.info() != Eigen::Success) {
        std::cerr << "行列の分解に失敗しました" << std::endl;
        return false;
    }
    
    Vector p_inner = solver.solve(b);
    
    if (solver.info() != Eigen::Success) {
        std::cerr << "線形方程式の求解に失敗しました" << std::endl;
        return false;
    }
    
    // 結果を圧力場に反映
    idx = 0;
    for (int i = 1; i < n - 1; ++i) {
        for (int j = 1; j < n - 1; ++j) {
            P(i, j) = p_inner(idx);
            idx++;
        }
    }
    
    return true;
}

double SquareThinFilmFDM::calculateTotalForce() const {
    // 基本の面積要素
    double area_element = dx * dy;
    double total_force = 0.0;
    
    // すべての格子点での圧力と面積の積を合計
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            // 点の位置による面積の重み付け
            // 境界上の点は内部点の半分の面積を代表し、コーナー点は1/4の面積を代表する
            double area_coef = 1.0;
            
            // 境界上の点の重み付け
            if (i == 0 || i == n - 1) {
                area_coef *= 0.5;
            }
            if (j == 0 || j == n - 1) {
                area_coef *= 0.5;
            }
            
            // 修正された面積要素
            double modified_area = area_element * area_coef;
            
            // 力の加算
            total_force += P(i, j) * modified_area;
        }
    }
    
    return total_force;
}