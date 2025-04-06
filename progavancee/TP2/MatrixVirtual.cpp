#include <vector>
#include <iostream>
#include <chrono>
#include <memory>
#include <numeric>

template <typename T>
class InstanceCounter {
    protected:
        inline static int created_count_ = 0;
        inline static int deleted_count_ = 0;

        InstanceCounter() { ++created_count_; }
        virtual ~InstanceCounter() { ++deleted_count_; }

    public:
        static int created_count() { return created_count_; }
        static int deleted_count() { return deleted_count_; }
    };

class MatrixExpr {
    protected:
        unsigned rows_;
        unsigned cols_;

    public:
        MatrixExpr(unsigned rows, unsigned cols) : rows_(rows), cols_(cols) {}
        virtual ~MatrixExpr() = default;

        unsigned rows() const { return rows_; }
        unsigned cols() const { return cols_; }

        virtual double operator()(unsigned row, unsigned col) const = 0;

        virtual double sum() const {
            double total = 0;
            for (unsigned i = 0; i < rows_; ++i) {
                for (unsigned j = 0; j < cols_; ++j) {
                    total += (*this)(i, j);
                }
            }
            return total;
        }
};

class Matrix : public MatrixExpr, public InstanceCounter<Matrix> {
    private:
        std::vector<double> data_;

    public:
        Matrix(unsigned rows, unsigned cols, double init_val = 0.0)
            : MatrixExpr(rows, cols), data_(rows * cols, init_val) {}

        Matrix(const MatrixExpr& expr)
            : MatrixExpr(expr.rows(), expr.cols()), data_(rows_ * cols_) {
            for (unsigned i = 0; i < rows_; ++i) {
                for (unsigned j = 0; j < cols_; ++j) {
                    data_[i * cols_ + j] = expr(i, j);
                }
            }
        }

        ~Matrix() override = default;

        double& operator()(unsigned row, unsigned col) {
            return data_[row * cols_ + col];
        }

        double operator()(unsigned row, unsigned col) const override {
            return data_[row * cols_ + col];
        }

        double sum() const override {
            return std::accumulate(data_.begin(), data_.end(), 0.0);
        }
};

class MatrixAdd : public MatrixExpr, public InstanceCounter<MatrixAdd> {
    private:
        const MatrixExpr& left_;
        const MatrixExpr& right_;

    public:
        MatrixAdd(const MatrixExpr& left, const MatrixExpr& right)
            : MatrixExpr(left.rows(), left.cols()), left_(left), right_(right) {}

        ~MatrixAdd() override = default;

        double operator()(unsigned row, unsigned col) const override {
            return left_(row, col) + right_(row, col);
        }
};

MatrixAdd operator+(const MatrixExpr& left, const MatrixExpr& right) {
    return MatrixAdd(left, right);
}

int main()
{
    const unsigned size{ 10000 };
    Matrix m1{ size, size, 1.e-5 };
 
    auto start = std::chrono::steady_clock::now();
    Matrix res1{ m1 + m1 };
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration< double > elapsed_seconds = end - start;
    std::cout << "res1.sum() = " << res1.sum() << " in " << elapsed_seconds.count() << "s\n";
 
    start = std::chrono::steady_clock::now();
    Matrix res2{ m1 + m1 + m1 + m1 };
    end = std::chrono::steady_clock::now();
    elapsed_seconds = end - start;
    std::cout << "res2.sum() = " << res2.sum() << " in " << elapsed_seconds.count() << "s\n";
 
    start = std::chrono::steady_clock::now();
    Matrix res3{ m1 + m1 + m1 + m1 + m1 + m1 };
    end = std::chrono::steady_clock::now();
    elapsed_seconds = end - start;
    std::cout << "res3.sum() = " << res3.sum() << " in " << elapsed_seconds.count() << "s\n";
 
    std::cout << "Matrix instances created: " << Matrix::created_count() << "\n";
    std::cout << "Matrix instances deleted: " << Matrix::deleted_count() << "\n";
    std::cout << "MatrixAdd instances created: " << MatrixAdd::created_count() << "\n";
    std::cout << "MatrixAdd instances deleted: " << MatrixAdd::deleted_count() << "\n";
}

/*

res1.sum() = 1 in 2.09155s
res2.sum() = 2 in 3.65523s
res3.sum() = 3 in 4.79535s
Matrix instances created: 4
Matrix instances deleted: 0
MatrixAdd instances created: 9
MatrixAdd instances deleted: 9

*/