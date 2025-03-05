#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <memory>

#include <Eigen/Core>
#include <Eigen/SparseCore>
#include <Eigen/Dense>
#include <Eigen/Eigenvalues>

#include <Spectra/SymEigsSolver.h>
#include <Spectra/MatOp/SparseSymMatProd.h>


class MatrixParser {
    
public:
    enum MtxType {
        BigAndDense = 1,
        BigAndSparse = 2,
        Small = 3
    };

    MatrixParser(std::string filename) : filename_(filename) {
        std::ifstream infile(filename_);

        std::string line;
        while (std::getline(infile, line)) {
            std::istringstream st(line);
            int f, t;
            if (!(st >> f >> t)) {
                break;
            }
            mp_[f] = f;
            mp_[t] = t;
            // if (mp_.count(f) == 0) {
            //     mp_[f] = nodes_count_++;
            // }
            // if (mp_.count(t) == 0) {
            //     mp_[t] = nodes_count_++;
            // }
            nodes_count_ = std::max({nodes_count_, f, t});
            edges_.push_back(std::make_pair(f, t));
        }
        nodes_count_ += 1;
        std::cout << nodes_count_ << ' ' << edges_.size() << '\n';
        infile.close();
    }

    std::unique_ptr<Eigen::MatrixXd> LaplacianDenseMatrix() {
        if (nodes_count_ > 500) {
            return nullptr;
        }
        auto L = std::make_unique<Eigen::MatrixXd>(Eigen::MatrixXd::Zero(nodes_count_, nodes_count_));
        for (auto [u, v]: edges_) {
            (*L)(mp_[u], mp_[v]) = -1;
            (*L)(mp_[v], mp_[u]) = -1;
            (*L)(mp_[u], mp_[u])++;
            (*L)(mp_[v], mp_[v])++;
        }
        return L;
    }

    std::unique_ptr<Eigen::SparseMatrix<double>> LaplacianSparseMatrix() {
        auto L = std::make_unique<Eigen::SparseMatrix<double>>(nodes_count_, nodes_count_);
        std::vector<int> cnt(nodes_count_, 0);
        for (auto [u, v]: edges_) {
            (*L).insert(mp_[v], mp_[u]) = -1;
            (*L).insert(mp_[u], mp_[v]) = -1;
            cnt[mp_[v]]++;
            cnt[mp_[u]]++;
        }
        for (int i = 0; i < nodes_count_; i++) {
            (*L).insert(i, i) = cnt[i];
        }
        return L;
    }
    
    
private:
    std::string filename_;
    std::unordered_map<int, int> mp_;
    int nodes_count_ = 0;
    std::vector<bool> used_;
    std::vector<std::pair<int, int>> edges_;
    int cur_;
    MtxType type_;
};


std::vector<std::vector<double>> ComputeCoordinatesForSmallMtx(std::string filename) {
    MatrixParser mtx(filename);

    auto L = mtx.LaplacianDenseMatrix();

    std::cout << *L << '\n';

    Spectra::DenseSymMatProd<double> op(*L);
    
    Spectra::SymEigsSolver<Spectra::DenseSymMatProd<double>> eigs(op, 2, 6);
 
    eigs.init();
    int nconv = eigs.compute(Spectra::SortRule::SmallestMagn);
 
    Eigen::VectorXd evalues;
    std::vector<std::vector<double>> found(2);
    if(eigs.info() == Spectra::CompInfo::Successful) {
        evalues = eigs.eigenvalues();
        std::cout << evalues << std::endl;
        auto evectors = eigs.eigenvectors();
 
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < L->rows(); j++) {
                found[i].push_back(evectors(j, i));
            }
        }
    }
    return found;
}

std::vector<std::vector<double>> ComputeCoordinatesSparseMtx(std::string filename) {
    MatrixParser mtx(filename);

    auto L = mtx.LaplacianSparseMatrix();

    Spectra::SparseSymMatProd<double> op(*L);
 
    // Construct eigen solver object, requesting the largest three eigenvalues
    Spectra::SymEigsSolver<Spectra::SparseSymMatProd<double>> eigs(op, 3, 10);
 
    eigs.init();
    int nconv = eigs.compute(Spectra::SortRule::SmallestAlge);
 
    std::vector<std::vector<double>> found(2);
    if(eigs.info() == Spectra::CompInfo::Successful) {
        auto evalues = eigs.eigenvalues();
        std::cout << evalues << std::endl;
        auto evectors = eigs.eigenvectors();
        // std::cout << "eigenvectors are\n";
        // std::cout << evectors << '\n';
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < L->rows(); j++) {
                found[i].push_back(evectors(j, i));
            }
        }
    }
    return found;
}

 
int main()
{
    auto res = ComputeCoordinatesSparseMtx("graph.txt");   
    std::cout << res[0].size() << '\n';
    std::ofstream outfile("my_embedding.txt");
    for (int i = 0; i < res[0].size(); i++) {
        outfile << res[0][i] << ' ' << res[1][i] << '\n';
    }
    outfile.close();
    return 0;
}