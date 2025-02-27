#include <Eigen/Core>
#include <Eigen/SparseCore>
#include <Spectra/GenEigsSolver.h>
#include <Spectra/MatOp/SparseGenMatProd.h>
// <Spectra/MatOp/DenseSymMatProd.h> is implicitly included
#include <iostream>


int main()
{
    // A band matrix with 1 on the main diagonal, 2 on the below-main subdiagonal,
    // and 3 on the above-main subdiagonal
    std::vector<Eigen::Triplet<double>> triplets;

    // Laplacian matrix representation
    triplets.push_back({0, 0,  2.0});
    triplets.push_back({0, 1, -1.0});
    triplets.push_back({0, 2, -1.0});

    triplets.push_back({1, 0, -1.0});
    triplets.push_back({1, 1,  3.0});
    triplets.push_back({1, 2, -1.0});
    triplets.push_back({1, 3, -1.0});

    triplets.push_back({2, 0, -1.0});
    triplets.push_back({2, 1, -1.0});
    triplets.push_back({2, 2,  3.0});
    triplets.push_back({2, 4, -1.0});

    triplets.push_back({3, 1, -1.0});
    triplets.push_back({3, 3,  3.0});
    triplets.push_back({3, 4, -1.0});
    triplets.push_back({3, 5, -1.0});

    triplets.push_back({4, 2, -1.0});
    triplets.push_back({4, 3, -1.0});
    triplets.push_back({4, 4,  3.0});
    triplets.push_back({4, 5, -1.0});

    triplets.push_back({5, 3, -1.0});
    triplets.push_back({5, 4, -1.0});
    triplets.push_back({5, 5,  3.0});
    triplets.push_back({5, 6, -1.0});

    triplets.push_back({6, 5, -1.0});
    triplets.push_back({6, 6,  2.0});
    triplets.push_back({6, 7, -1.0});

    triplets.push_back({7, 6, -1.0});
    triplets.push_back({7, 7,  3.0});
    triplets.push_back({7, 8, -1.0});
    triplets.push_back({7, 9, -1.0});

    triplets.push_back({8, 7, -1.0});
    triplets.push_back({8, 8,  2.0});
    triplets.push_back({8, 9, -1.0});

    triplets.push_back({9, 7, -1.0});
    triplets.push_back({9, 8, -1.0});
    triplets.push_back({9, 9,  2.0});
    const int n = 10;
    Eigen::SparseMatrix<double> M(n, n);
    M.setFromTriplets(triplets.begin(), triplets.end());
    // Construct matrix operation object using the wrapper class SparseGenMatProd
    Spectra::SparseGenMatProd<double> op(M);

    // Construct eigen solver object, requesting the largest three eigenvalues
    Spectra::GenEigsSolver<Spectra::SparseGenMatProd<double>> eigs(op, 3, 8);

    // Initialize and compute
    eigs.init();
    int nconv = eigs.compute(Spectra::SortRule::SmallestMagn);

    // // Retrieve results
    if(eigs.info() == Spectra::CompInfo::Successful) {
        auto evalues = eigs.eigenvalues();
        std::cout << "Eigenvalues found:\n" << evalues << std::endl;
        std::cout << "Correspondent Eigenvectors are\n";
        std::cout << eigs.eigenvectors() << std::endl;
    }
    return 0;
}