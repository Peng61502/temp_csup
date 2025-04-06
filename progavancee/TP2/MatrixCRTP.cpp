#include <vector>
#include <iostream>
#include <chrono>
#include <numeric>

class MatrixAddCounter {
    protected:
        inline static int created_count_ = 0;
        inline static int deleted_count_ = 0;

        MatrixAddCounter() { ++created_count_; }
        ~MatrixAddCounter() { ++deleted_count_; }

    public:
        static int created_count() { return created_count_; }
        static int deleted_count() { return deleted_count_; }
};

template <typename T>
class InstanceCounter {
    protected:
        inline static int created_count_ = 0;
        inline static int deleted_count_ = 0;

        InstanceCounter() { ++created_count_; }
        ~InstanceCounter() { ++deleted_count_; }

    public:
        static int created_count() { return created_count_; }
        static int deleted_count() { return deleted_count_; }
};

template <typename Derived>
class MatrixExpr {
    protected:
        unsigned rows_;
        unsigned cols_;

    public:
        MatrixExpr(unsigned rows, unsigned cols) : rows_(rows), cols_(cols) {}

        unsigned rows() const { return rows_; }
        unsigned cols() const { return cols_; }

        double operator()(unsigned row, unsigned col) const {
            return static_cast<const Derived&>(*this)(row, col);
        }

        double sum() const {
            double total = 0;
            for (unsigned i = 0; i < rows_; ++i) {
                for (unsigned j = 0; j < cols_; ++j) {
                    total += (*this)(i, j);
                }
            }
            return total;
        }
};

class Matrix : public MatrixExpr<Matrix>, public InstanceCounter<Matrix> {
    private:
        std::vector<double> data_;

    public:
        Matrix(unsigned rows, unsigned cols, double init_val = 0.0)
            : MatrixExpr<Matrix>(rows, cols), data_(rows * cols, init_val) {}

        template <typename Expr>
        Matrix(const MatrixExpr<Expr>& expr)
            : MatrixExpr<Matrix>(expr.rows(), expr.cols()), data_(expr.rows() * expr.cols()) {
            for (unsigned i = 0; i < expr.rows(); ++i) {
                for (unsigned j = 0; j < expr.cols(); ++j) {
                    data_[i * expr.cols() + j] = expr(i, j);
                }
            }
        }

        double& operator()(unsigned row, unsigned col) {
            return data_[row * cols_ + col];
        }

        double operator()(unsigned row, unsigned col) const {
            return data_[row * cols_ + col];
        }

        double sum() const {
            return std::accumulate(data_.begin(), data_.end(), 0.0);
        }
};

template <typename M1, typename M2>
class MatrixAdd : public MatrixExpr<MatrixAdd<M1, M2>>, public MatrixAddCounter {
    private:
        const MatrixExpr<M1>& left_;
        const MatrixExpr<M2>& right_;

    public:
        MatrixAdd(const MatrixExpr<M1>& left, const MatrixExpr<M2>& right)
            : MatrixExpr<MatrixAdd<M1, M2>>(left.rows(), left.cols()), left_(left), right_(right) {}

        double operator()(unsigned row, unsigned col) const {
            return left_(row, col) + right_(row, col);
        }
};

template <typename M1, typename M2>
MatrixAdd<M1, M2> operator+(const MatrixExpr<M1>& left, const MatrixExpr<M2>& right) {
    return MatrixAdd<M1, M2>(left, right);
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
    std::cout << "MatrixAdd instances created: " << MatrixAddCounter::created_count() << "\n";
    std::cout << "MatrixAdd instances deleted: " << MatrixAddCounter::deleted_count() << "\n";
}

/*

res1.sum() = 1 in 1.38172s
res2.sum() = 2 in 1.72569s
res3.sum() = 3 in 1.64656s
Matrix instances created: 4
Matrix instances deleted: 0
MatrixAdd instances created: 9
MatrixAdd instances deleted: 9

*/