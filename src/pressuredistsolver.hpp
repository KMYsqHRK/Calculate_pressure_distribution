#include <Eigen/Sparse>
#include <vector>
#include <functional>

class SquareThinFilmFDM {
public:
    using Matrix = Eigen::MatrixXd;
    using SparseMatrix = Eigen::SparseMatrix<double>;
    using Vector = Eigen::VectorXd;
    using HeightFunction = std::function<double(double, double)>;

private:
    int n;                    // 格子点数
    double width;            // 長方形の幅[m]
    double height;           // 長方形の高さ[m]
    double viscosity;        // 粘度 [Pa・s]
    double velocity;         // すべり速度 [m/s]
    
    double dx;               // x方向格子間隔
    double dy;               // y方向格子間隔
    
    Vector x;                // x座標
    Vector y;                // y座標
    Matrix X;                // x座標メッシュ
    Matrix Y;                // y座標メッシュ
    Matrix P;                // 圧力場
    Matrix h;                // 膜厚

public:
    /**
     * コンストラクタ
     * @param n 片側の格子点数（n×nの格子を生成）
     * @param side_width 長方形の幅[m]
     * @param side_height 長方形の高さ[m]
     * @param h_func 膜厚を計算する関数 h(x, y)、nullptrの場合は一定膜厚
     * @param viscosity 粘度 [Pa・s]
     * @param velocity すべり速度 [m/s]（x方向正）
     */
    SquareThinFilmFDM(int n, double side_width, double side_height, 
                     HeightFunction h_func = nullptr, 
                     double viscosity = 0.01, double velocity = 1.0);

    /**
     * 各辺に異なる一定圧力を設定
     * @param p_bottom 下辺の圧力 [Pa]
     * @param p_right 右辺の圧力 [Pa]
     * @param p_top 上辺の圧力 [Pa]
     * @param p_left 左辺の圧力 [Pa]
     */
    void setEdgeBoundary(double p_bottom, double p_right, double p_top, double p_left);

    /**
     * 直接法で定常状態のレイノルズ方程式を解く
     * @return 解が成功したかどうか
     */
    bool solveDirect();

    /**
     * 領域全体にわたる合力を計算する
     * @return 合力 [N]
     */
    double calculateTotalForce() const;

    /**
     * 圧力分布を取得
     * @return 圧力分布の行列
     */
    const Matrix& getPressureField() const { return P; }

    /**
     * 膜厚分布を取得
     * @return 膜厚分布の行列
     */
    const Matrix& getHeightField() const { return h; }

private:
    void initializeHeight(HeightFunction h_func);
};