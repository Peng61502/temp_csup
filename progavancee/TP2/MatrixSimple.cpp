#include <memory>
#include <vector>
#include <iostream>
#include <chrono>
#include <stdexcept>
#include <numeric>

template <typename T>
class InstanceCounter {
    public:
        static int created_count_;
        static int deleted_count_;

        InstanceCounter() { ++created_count_; }
        virtual ~InstanceCounter() { ++deleted_count_; }

        static int created_count() { return created_count_; }
        static int deleted_count() { return deleted_count_; }
};

template <typename T>
int InstanceCounter<T>::created_count_ = 0;
template <typename T>
int InstanceCounter<T>::deleted_count_ = 0;

class Matrix : public InstanceCounter<Matrix> {
    private:
        unsigned rows_;
        unsigned cols_;
        std::unique_ptr<std::vector<double>> data_;

    public:
        Matrix(unsigned rows, unsigned cols, double init_val = 0.0)
            : rows_(rows), cols_(cols),
            data_(std::make_unique<std::vector<double>>(rows * cols, init_val)) {}

        unsigned rows() const { return rows_; }
        unsigned cols() const { return cols_; }

        double& operator()(unsigned row, unsigned col) {
            if (row >= rows_ || col >= cols_) {
                throw std::out_of_range("Index out of range");
            }
            return (*data_)[row * cols_ + col];
        }

        const double& operator()(unsigned row, unsigned col) const {
            if (row >= rows_ || col >= cols_) {
                throw std::out_of_range("Index out of range");
            }
            return (*data_)[row * cols_ + col];
        }

        double sum() const {
            return std::accumulate(data_->begin(), data_->end(), 0.0);
        }
};

Matrix operator+(const Matrix& a, const Matrix& b) {
    if (a.rows() != b.rows() || a.cols() != b.cols()) {
        throw std::invalid_argument("Matrix dimensions mismatch");
    }

    Matrix result(a.rows(), a.cols());
    for (unsigned i = 0; i < a.rows(); ++i) {
        for (unsigned j = 0; j < a.cols(); ++j) {
            result(i, j) = a(i, j) + b(i, j);
        }
    }
    return result;
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
}

/*

res1.sum() = 2000 in 1.92322s
res2.sum() = 4000 in 6.15684s
res3.sum() = 6000 in 9.64529s
Matrix instances created: 10
Matrix instances deleted: 6

*/